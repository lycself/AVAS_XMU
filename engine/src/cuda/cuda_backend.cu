// CUDA back end of the tracker (device.h).
//
// The particle work is the shared code of kernels.h (compiled here for the GPU with --fmad=false);
// this file only holds memory management, kernel launches, the fixed-order reductions of reduce.h
// and the space-charge mesh solvers.  Several GPUs of one process each hold whole reduction groups
// of particles (4096); their partial sums are combined on the host in group order, so the result
// does not depend on the number of GPUs.
#include <cuda_runtime.h>
#include <cub/device/device_select.cuh>
#include <cub/iterator/counting_input_iterator.cuh>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "core/device.h"
#include "core/errors.h"
#include "core/format.h"
#include "core/kernels.h"
#include "core/options.h"
#include "core/reduce.h"
#include "core/sc_kernels.h"
#include "core/secondary.h"
#include "cuda/cuda_mpi.h"

namespace avas {

bool cuda_compiled() { return true; }

namespace cu {

void check(cudaError_t e, const char* what) {
    if (e != cudaSuccess) throw EngineError(std::string("CUDA error: ") + cudaGetErrorString(e) + " (" + what + ")");
}
#define CK(x) ::avas::cu::check((x), #x)

template <class T>
struct DBuf {
    T* p = nullptr;
    size_t n = 0;
    DBuf() = default;
    DBuf(const DBuf&) = delete;
    DBuf& operator=(const DBuf&) = delete;
    ~DBuf() { release(); }
    void release() {
        if (p) cudaFree(p);
        p = nullptr;
        n = 0;
    }
    void alloc(size_t m) {
        if (m == n && p) return;
        release();
        if (m == 0) return;
        CK(cudaMalloc(reinterpret_cast<void**>(&p), m * sizeof(T)));
        n = m;
    }
    void zero() {
        if (p) CK(cudaMemset(p, 0, n * sizeof(T)));
    }
    void up_alloc(const T* h, long long cnt) {
        alloc(static_cast<size_t>(cnt > 0 ? cnt : 1));
        up(h, static_cast<size_t>(cnt));
    }
    void up(const T* h, size_t cnt, size_t off = 0) {
        if (cnt) CK(cudaMemcpy(p + off, h, cnt * sizeof(T), cudaMemcpyHostToDevice));
    }
    void down(T* h, size_t cnt, size_t off = 0) const {
        if (cnt) CK(cudaMemcpy(h, p + off, cnt * sizeof(T), cudaMemcpyDeviceToHost));
    }
    template <class V>
    void set(const std::vector<V>& v) {
        alloc(v.size());
        up(reinterpret_cast<const T*>(v.data()), v.size());
    }
};

// ---------------------------------------------------------------------------------------------
// Fixed-order reductions (reduce.h): level 1 in a block of 256 threads, level 2 over 16 chunks.
// A functor F provides K sums and M extrema per particle (extrema are maxima; a minimum is passed
// negated) and the neutral value of every extremum.

constexpr int kThreads = kRedChunk;

__device__ inline double max_skip_nan(double a, double b) { return b > a ? b : a; }

template <int K, int M>
__device__ inline void block_reduce(double* sh, const double* s, const double* m, const double* neutral, double* out) {
    const int t = threadIdx.x;
    for (int k = 0; k < K; ++k) {
        sh[t] = s[k];
        __syncthreads();
        for (int w = kThreads / 2; w >= 1; w >>= 1) {
            if (t < w) sh[t] = sh[t] + sh[t + w];
            __syncthreads();
        }
        if (t == 0) out[k] = sh[0];
        __syncthreads();
    }
    for (int k = 0; k < M; ++k) {
        const double v = m[k];
        sh[t] = v != v ? neutral[k] : v;
        __syncthreads();
        for (int w = kThreads / 2; w >= 1; w >>= 1) {
            if (t < w) sh[t] = max_skip_nan(sh[t], sh[t + w]);
            __syncthreads();
        }
        if (t == 0) out[K + k] = sh[0];
        __syncthreads();
    }
}

template <int K, int M, class F>
__global__ void k_reduce_chunks(F f, int n, double* chunkOut) {
    __shared__ double sh[kThreads];
    double s[K > 0 ? K : 1], m[M > 0 ? M : 1], neutral[M > 0 ? M : 1];
    f.neutral(neutral);
    for (int k = 0; k < K; ++k) s[k] = 0.0;
    for (int k = 0; k < M; ++k) m[k] = neutral[k];
    const int i = blockIdx.x * kThreads + threadIdx.x;
    if (i < n) f(i, s, m);
    block_reduce<K, M>(sh, s, m, neutral, chunkOut + static_cast<long long>(blockIdx.x) * (K + M));
}

// Measurement only (AVAS_ENGINE_GPU_ATOMIC=1): the usual non-deterministic scheme, block sums added to
// one global accumulator with atomics in the order the blocks finish (no fixed order, no level 2).
__device__ inline void atomic_max_double(double* a, double v) {
    unsigned long long* p = reinterpret_cast<unsigned long long*>(a);
    unsigned long long old = *p;
    while (__longlong_as_double(static_cast<long long>(old)) < v) {
        const unsigned long long prev = atomicCAS(p, old, static_cast<unsigned long long>(__double_as_longlong(v)));
        if (prev == old) break;
        old = prev;
    }
}

template <int K, int M, class F>
__global__ void k_reduce_atomic(F f, int n, double* acc) {
    __shared__ double sh[kThreads];
    __shared__ double res[K + M > 0 ? K + M : 1];
    double s[K > 0 ? K : 1], m[M > 0 ? M : 1], neutral[M > 0 ? M : 1];
    f.neutral(neutral);
    for (int k = 0; k < K; ++k) s[k] = 0.0;
    for (int k = 0; k < M; ++k) m[k] = neutral[k];
    const int i = blockIdx.x * kThreads + threadIdx.x;
    if (i < n) f(i, s, m);
    block_reduce<K, M>(sh, s, m, neutral, res);
    if (threadIdx.x == 0) {
        for (int k = 0; k < K; ++k) atomicAdd(acc + k, res[k]);
        for (int k = 0; k < M; ++k) atomic_max_double(acc + K + k, res[K + k]);
    }
}

// level 2: one block of 16 threads per group of 16 chunks
template <int K, int M, class F>
__global__ void k_reduce_groups(F f, const double* chunkOut, int nChunks, double* groupOut) {
    __shared__ double sh[kRedGroupChunks];
    double neutral[M > 0 ? M : 1];
    f.neutral(neutral);
    const int t = threadIdx.x;
    const int ch = blockIdx.x * kRedGroupChunks + t;
    for (int k = 0; k < K + M; ++k) {
        const bool sum = k < K;
        sh[t] = ch < nChunks ? chunkOut[static_cast<long long>(ch) * (K + M) + k] : (sum ? 0.0 : neutral[k - K]);
        __syncthreads();
        for (int w = kRedGroupChunks / 2; w >= 1; w >>= 1) {
            if (t < w) sh[t] = sum ? sh[t] + sh[t + w] : max_skip_nan(sh[t], sh[t + w]);
            __syncthreads();
        }
        if (t == 0) groupOut[static_cast<long long>(blockIdx.x) * (K + M) + k] = sh[0];
        __syncthreads();
    }
}

// ---- functors ----

// Step sums: the 19 statistics of kern::Contrib, then the superposeout crossings and the flagged
// crossing records (counts, exact in double).
constexpr int kStepK = kern::kStepSums + 2;

struct StepF {
    kern::LatV L;
    kern::PartV P;
    kern::StepArgs a;
    kern::RfSrc rf;
    unsigned long long* counters;  // unused
    __device__ void neutral(double* nt) const {
        for (int j = 0; j < 6; ++j) nt[j] = 0.0;
        nt[6] = -1e10;  // -zmin
        nt[7] = -1e10;  // zmax
    }
    __device__ void operator()(int i, double* s, double* m) const {
        kern::Contrib c;
        kern::trans_particle<PortableMath>(L, P, i, a, rf, c);
        kern::contrib_sums(c, s);
        if (c.hasA) {
            for (int j = 0; j < 6; ++j) m[j] = c.mx[j];
            m[6] = -c.zmin;
            m[7] = c.zmax;
        }
        s[kern::kStepSums] = c.sp ? 1.0 : 0.0;
        s[kern::kStepSums + 1] = P.rec[8 * static_cast<long long>(i) + 7] == 1.0 ? 1.0 : 0.0;
    }
};

struct Ubp1F {
    kern::PartV P;
    double s6[6];
    __device__ void neutral(double*) const {}
    __device__ void operator()(int i, double* s, double*) const { kern::ubp_pass1(P, i, s6, s); }
};

struct Ubp2F {
    kern::PartV P;
    double cen[6];
    double igz;
    __device__ void neutral(double* nt) const {
        nt[0] = -1e10;
        nt[1] = -1e10;
    }
    __device__ void operator()(int i, double* s, double* m) const {
        kern::ubp_pass2(P, i, cen, igz, s);
        m[0] = -P.z[i];
        m[1] = P.z[i];
    }
};

struct Hesc1F {
    kern::PartV P;
    double tRef;
    __device__ void neutral(double*) const {}
    __device__ void operator()(int i, double* s, double*) const { kern::hesc_pass1(P, i, tRef, s); }
};

struct Hesc2F {
    kern::PartV P;
    double cen[6];
    double gc;
    __device__ void neutral(double*) const {}
    __device__ void operator()(int i, double* s, double*) const { kern::hesc_pass2(P, i, cen, gc, s); }
};

struct TrefF {  // fixed-order average and minimum of the times of the flagged rows
    kern::PartV P;
    __device__ void neutral(double* nt) const { nt[0] = -1e10; }
    __device__ void operator()(int i, double* s, double* m) const {
        const double* r = P.rec + 8 * static_cast<long long>(i);
        const bool on = r[7] == 1.0;
        s[0] = on ? r[6] : 0.0;
        s[1] = on ? 1.0 : 0.0;
        if (on) m[0] = -r[6];
    }
};

struct SpTminF {
    kern::PartV P;
    __device__ void neutral(double* nt) const { nt[0] = -1e10; }
    __device__ void operator()(int i, double*, double* m) const {
        const double* r = P.spRec + 8 * static_cast<long long>(i);
        if (r[7] == 1.0) m[0] = -r[6];
    }
};

// ---- per-particle kernels ----

__global__ void k_rows(kern::RowOp op, kern::PartV P, int n) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    double* r = P.rec + 8 * static_cast<long long>(i);
    if (r[7] != 1.0) return;
    kern::het_row<PortableMath>(op, r, P.qOverM[i], P.scx[i], P.scy[i], P.scz[i]);
}

__global__ void k_kick(int kind, double wx, double wy, double len, kern::PartV P, int n) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    double* r = P.rec + 8 * static_cast<long long>(i);
    if (r[7] != 1.0) return;
    kern::kick_row(r, kind, wx, wy, len * P.qOverM[i], false);
}

__global__ void k_ztot(kern::PartV P, int n, double zPlane, double tRef) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    P.nextPlane[i] = 0;
    double* r = P.rec + 8 * static_cast<long long>(i);
    if (r[7] == 1.0) ztot_row(r, zPlane, tRef, true, P.x[i], P.ux[i], P.y[i], P.uy[i], P.z[i], P.uz[i]);
    for (int j = 0; j < 8; ++j) r[j] = 0.0;
}

__global__ void k_deposit(kern::MeshV M, kern::PartV P, int n, unsigned long long* grid) {
    const int p = blockIdx.x * blockDim.x + threadIdx.x;
    if (p >= n || P.lost[p] == 1) return;
    int i, j, k;
    double w[8];
    if (!kern::mesh_cell(M, P, p, i, j, k, w)) return;
    const double q = P.charge[p];
    for (int m = 0; m < 8; ++m) {
        const long long v = kern::fix_charge(w[m] * q, M.fixScaleExp);
        atomicAdd(grid + kern::mesh_corner(M, i, j, k, m), static_cast<unsigned long long>(v));
    }
}

// measurement only: double atomics in arbitrary order
__global__ void k_deposit_atomic(kern::MeshV M, kern::PartV P, int n, double* grid) {
    const int p = blockIdx.x * blockDim.x + threadIdx.x;
    if (p >= n || P.lost[p] == 1) return;
    int i, j, k;
    double w[8];
    if (!kern::mesh_cell(M, P, p, i, j, k, w)) return;
    const double q = P.charge[p];
    for (int m = 0; m < 8; ++m) atomicAdd(grid + kern::mesh_corner(M, i, j, k, m), w[m] * q);
}

__global__ void k_interp(kern::MeshV M, kern::PartV P, int n, const double* ex, const double* ey, const double* ez) {
    const int p = blockIdx.x * blockDim.x + threadIdx.x;
    if (p >= n) return;
    kern::mesh_interp(M, P, p, ex, ey, ez, 8987551787.368176);
}

__global__ void k_boundary(kern::LatV L, kern::PartV P, int n, double t, double dt, BoundaryHit* hits,
                           unsigned long long* count, int cap) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    int g = 0;
    double Pp[6], tHit = t;
    const int code = kern::boundary_particle<PortableMath>(L, P, i, t, dt, false, g, Pp, tHit);
    if (code == 0) return;
    if (code == 1) P.lost[i] = 1;
    const unsigned long long slot = atomicAdd(count, 1ull);
    if (slot >= static_cast<unsigned long long>(cap)) return;
    BoundaryHit& h = hits[slot];
    h.i = i;
    h.code = code;
    h.geom = g;
    for (int j = 0; j < 6; ++j) h.P[j] = Pp[j];
    h.tHit = tHit;
}

__global__ void k_lp_fold(kern::PartV P, int n, double zs, double full, double half) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) kern::lp_fold_particle(P, i, zs, full, half);
}

__global__ void k_long_loss(kern::PartV P, int n, const double* restMass, double zs, double ws, double dW, double dzMax) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) kern::long_loss_particle(P, i, restMass, zs, ws, dW, dzMax, false);
}

// ---- space-charge mesh solvers (sc_kernels.h), one node / target per thread ----

using sck::SpGrid;

template <class T>
__global__ void k_sym(SpGrid g, const T* rho, const T* G, T thr, double* phi) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n_targets()) return;
    int i, j, k;
    g.target(id, i, j, k);
    sck::sym_target<T>(g, rho, G, thr, i, j, k, phi);
}

template <class T>
__global__ void k_corr(SpGrid g, const T* G, const int* corr, const T* dval, long long nc, double* phi) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n3()) return;
    int a, bb, c;
    g.ijk(id, a, bb, c);
    sck::corr_node<T>(g, G, corr, dval, nc, a, bb, c, phi);
}

// Green table (sc_kernels.h): F at the cell corners, then the cell averages into the 8 mirror entries.
__global__ void k_green_corner(SpGrid g, double dx, double dy, double dz, double* F) {
    const long long ay = g.my + 1, az = g.mz + 1;
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= (g.mx + 1) * ay * az) return;
    F[id] = sck::green_corner(static_cast<int>(id / (ay * az)), static_cast<int>((id / az) % ay), static_cast<int>(id % az),
                              dx, dy, dz);
}

__global__ void k_green_cell(SpGrid g, const double* F, double vol, double* G) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= static_cast<long long>(g.mx) * g.my * g.mz) return;
    sck::green_cell(g, F, vol, static_cast<int>(id / (static_cast<long long>(g.my) * g.mz)),
                    static_cast<int>((id / g.mz) % g.my), static_cast<int>(id % g.mz), G);
}

// SPICNIC correction candidates: q = 8 target + mirror, kept when the difference is not dropped.
__global__ void k_corr_val(SpGrid g, const double* rho, double thr, long long nq, double* dv, int* flag) {
    const long long q = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (q >= nq) return;
    int I, J, K;
    const double d = sck::corr_value(g, rho, thr, q / 8, static_cast<int>(q % 8), I, J, K);
    dv[q] = d;
    flag[q] = d != 0.0 ? 1 : 0;
}

__global__ void k_corr_expand(SpGrid g, const double* rho, double thr, const int* idx, const double* dv, long long nc,
                              int* corr, double* dval) {
    const long long e = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (e >= nc) return;
    const long long q = idx[e];
    int I, J, K;
    sck::corr_value(g, rho, thr, q / 8, static_cast<int>(q % 8), I, J, K);
    corr[3 * e] = I;
    corr[3 * e + 1] = J;
    corr[3 * e + 2] = K;
    dval[e] = dv[q];
}

// PICNIC: nodes [lo, hi) of the mesh, one per thread.
template <class T>
__global__ void k_picnic(SpGrid g, const T* rho, const T* G, long long lo, long long hi, double* phi) {
    const long long id = lo + static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= hi) return;
    int i, j, k;
    g.ijk(id, i, j, k);
    phi[id] = sck::picnic_node<T>(g, rho, G, i, j, k);
}

__global__ void k_grad_spicnic(SpGrid g, const double* phi, double hx, double hy, double hz, double* ex, double* ey,
                               double* ez) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n3()) return;
    int i, j, k;
    g.ijk(id, i, j, k);
    sck::grad_spicnic_node(g, phi, hx, hy, hz, i, j, k, ex, ey, ez);
}

__global__ void k_scale(const double* rho, double s, double* a, long long n3) {
    const long long e = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (e < n3) a[e] = rho[e] * s;
}

__global__ void k_dst(SpGrid g, int axis, const double* in, double* out, const double* sinTab) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n3()) return;
    int i, j, k;
    g.ijk(id, i, j, k);
    out[id] = sck::dst_node(g, axis, in, sinTab, i, j, k);
}

__global__ void k_dft_z(SpGrid g, const double* re, const double* im, double* ore, double* oim, const double* cosTab,
                        const double* sinTab, double sign) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n3()) return;
    int i, j, k;
    g.ijk(id, i, j, k);
    sck::dft_z_node(g, re, im, cosTab, sinTab, sign, i, j, k, ore[id], oim[id]);
}

__global__ void k_green_mul(SpGrid g, const double* lx, const double* ly, const double* lz, double* re, double* im) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n3()) return;
    int i, j, k;
    g.ijk(id, i, j, k);
    const double f = sck::green_factor(lx, ly, lz, i, j, k);
    re[id] = re[id] * f;
    im[id] = im[id] * f;
}

__global__ void k_norm(const double* in, double s, double* out, long long n3) {
    const long long e = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (e < n3) out[e] = in[e] * s;
}

__global__ void k_grad_fft(SpGrid g, const double* phi, double hx, double hy, double hz, double* ex, double* ey,
                           double* ez) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n3()) return;
    int i, j, k;
    g.ijk(id, i, j, k);
    sck::grad_fft_node(g, phi, hx, hy, hz, i, j, k, ex, ey, ez);
}

__global__ void k_probe(int* out) {
    if (threadIdx.x == 0 && blockIdx.x == 0) *out = 61;
}

// ---- secondary particles: one thread tracks one particle through the whole lattice ----

struct GpuSeSink {
    double* planes;  // per particle (local index) and slot: 8 values
    int nSlots;
    BoundaryHit* hits;
    unsigned long long* count;
    int cap;
    __device__ void plane(int slot, const double* S, bool, int i) {
        double* o = planes + (static_cast<long long>(i) * nSlots + slot) * 8;
        for (int j = 0; j < 8; ++j) o[j] = S[j];
    }
    __device__ void sync_step(int) {}
    __device__ void sync_iter(int) {}
    __device__ void hit(int gi, int code, const double* P, double tHit, int i) {
        const unsigned long long k = atomicAdd(count, 1ull);
        if (k >= static_cast<unsigned long long>(cap)) return;
        BoundaryHit& h = hits[k];
        h.i = i;
        h.code = code;
        h.geom = gi;
        for (int j = 0; j < 6; ++j) h.P[j] = P[j];
        h.tHit = tHit;
    }
};

__global__ void k_secondary(kern::LatV L, kern::PartV P, kern::SeV V, int n, GpuSeSink sink) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    kern::se_track<PortableMath>(L, P, V, i, false, sink, nullptr);
}

inline int blocks(long long n, int t = 256) { return static_cast<int>((n + t - 1) / t); }

// ---------------------------------------------------------------------------------------------

struct Shard {
    int dev = 0;
    int lo = 0, hi = 0;  // particles [lo, hi)
    int n() const { return hi - lo; }
    // particles
    DBuf<double> x, ux, y, uy, z, uz, ex, ey, ez, bx, by, bz, scx, scy, scz, rel[6];
    DBuf<double> weight, qOverM, charge, spOutZ0, spOutZ1, old, rec, spRec, restMass, t, dt;
    DBuf<int> elemIndex, lost, nextPlane, boundaryMode, geomIndex, spOutCount;
    // lattice
    DBuf<kern::ElemV> elems, eff;
    DBuf<kern::SubV> subs;
    DBuf<MapView<double>> maps;
    std::vector<std::unique_ptr<DBuf<double>>> mapData;
    DBuf<double> zStart, zEnd, cavTime, cavPhase, segStart, segEnd, bndZ;
    DBuf<kern::SpOutV> spOut;
    DBuf<kern::RfSlotV> slots;
    DBuf<kern::GeomV> geoms;
    DBuf<int> bndR1, bndR2;
    kern::LatV L{};
    // per step
    DBuf<double> rf, events, chunks, groups;
    DBuf<unsigned long long> counters;
    DBuf<BoundaryHit> hits;
    // space charge
    DBuf<unsigned long long> grid;
    DBuf<double> gridD;  // measurement only (atomic)
    DBuf<double> xs, ys, zs, rho, G, phi, gex, gey, gez, work[4], tabs[6];
    DBuf<int> corr, cflag, cidx, csel;
    DBuf<double> dval, gF, cdv;
    DBuf<unsigned char> ctmp;
    kern::PartV P{};
};

class CudaBackend : public DeviceBackend {
public:
    CudaBackend(std::vector<int> devs, std::string desc)
        : devs_(std::move(devs)), desc_(std::move(desc)), atomic_(options().gpuAtomic) {}

    ~CudaBackend() override {
        if (prof_)
            std::fprintf(stdout, "[gpu profile] step calls %lld: launch+kernel %.3f s, download+host %.3f s; other device calls %.3f s\n",
                         profN_, profK_, profD_, profO_);
    }
    std::string describe() const override { return desc_; }
    bool fp32() const override { return false; }

    void begin_run(Beam& b, const LatHost& L, bool spRec) override {
        n_ = b.n;
        spRecOn_ = spRec;
        const int ng = red_groups(n_);
        // whole groups to the MPI ranks, then to the GPUs of this rank
        const MpiWorld& w = mpi_world();
        rankGOff_.assign(static_cast<size_t>(w.size), 0);
        rankGCnt_.assign(static_cast<size_t>(w.size), 0);
        rankPOff_.assign(static_cast<size_t>(w.size), 0);
        rankPCnt_.assign(static_cast<size_t>(w.size), 0);
        for (int r = 0; r < w.size; ++r) {
            const int g0 = static_cast<int>(static_cast<long long>(ng) * r / w.size);
            const int g1 = static_cast<int>(static_cast<long long>(ng) * (r + 1) / w.size);
            rankGOff_[static_cast<size_t>(r)] = g0;
            rankGCnt_[static_cast<size_t>(r)] = g1 - g0;
            rankPOff_[static_cast<size_t>(r)] = std::min(n_, g0 * kRedGroup);
            rankPCnt_[static_cast<size_t>(r)] = std::min(n_, g1 * kRedGroup) - std::min(n_, g0 * kRedGroup);
        }
        const int rg0 = static_cast<int>(rankGOff_[static_cast<size_t>(w.rank)]);
        const int rgn = static_cast<int>(rankGCnt_[static_cast<size_t>(w.rank)]);
        const int G = static_cast<int>(devs_.size());
        sh_.clear();
        for (int k = 0; k < G; ++k) {
            auto s = std::make_unique<Shard>();
            s->dev = devs_[static_cast<size_t>(k)];
            const int g0 = rg0 + static_cast<int>(static_cast<long long>(rgn) * k / G);
            const int g1 = rg0 + static_cast<int>(static_cast<long long>(rgn) * (k + 1) / G);
            s->lo = std::min(n_, g0 * kRedGroup);
            s->hi = std::min(n_, g1 * kRedGroup);
            sh_.push_back(std::move(s));
        }
        for (auto& sp : sh_) {
            Shard& s = *sp;
            CK(cudaSetDevice(s.dev));
            const size_t m = static_cast<size_t>(std::max(s.n(), 1));
            for (DBuf<double>* d : {&s.x, &s.ux, &s.y, &s.uy, &s.z, &s.uz, &s.ex, &s.ey, &s.ez, &s.bx, &s.by, &s.bz, &s.scx,
                                    &s.scy, &s.scz, &s.weight, &s.qOverM, &s.charge, &s.spOutZ0, &s.spOutZ1, &s.restMass, &s.t,
                                    &s.dt})
                d->alloc(m);
            for (auto& r : s.rel) r.alloc(m);
            s.old.alloc(6 * m);
            s.rec.alloc(8 * m);
            s.spRec.alloc(spRec ? 8 * m : 8);
            s.spRec.zero();
            for (DBuf<int>* d : {&s.elemIndex, &s.lost, &s.nextPlane, &s.boundaryMode, &s.geomIndex, &s.spOutCount}) d->alloc(m);
            s.counters.alloc(2);
            const int nch = (s.n() + kThreads - 1) / kThreads;
            s.chunks.alloc(static_cast<size_t>(std::max(nch, 1)) * 32);
            s.groups.alloc(static_cast<size_t>(std::max((nch + kRedGroupChunks - 1) / kRedGroupChunks, 1)) * 32);
            s.P = kern::PartV{};
            s.P.x = s.x.p; s.P.ux = s.ux.p; s.P.y = s.y.p; s.P.uy = s.uy.p; s.P.z = s.z.p; s.P.uz = s.uz.p;
            s.P.ex = s.ex.p; s.P.ey = s.ey.p; s.P.ez = s.ez.p; s.P.bx = s.bx.p; s.P.by = s.by.p; s.P.bz = s.bz.p;
            s.P.scx = s.scx.p; s.P.scy = s.scy.p; s.P.scz = s.scz.p;
            for (int j = 0; j < 6; ++j) s.P.rel[j] = s.rel[j].p;
            s.P.elemIndex = s.elemIndex.p; s.P.lost = s.lost.p; s.P.nextPlane = s.nextPlane.p;
            s.P.boundaryMode = s.boundaryMode.p; s.P.geomIndex = s.geomIndex.p; s.P.spOutCount = s.spOutCount.p;
            s.P.weight = s.weight.p; s.P.qOverM = s.qOverM.p; s.P.charge = s.charge.p;
            s.P.spOutZ0 = s.spOutZ0.p; s.P.spOutZ1 = s.spOutZ1.p;
            s.P.old = s.old.p; s.P.rec = s.rec.p; s.P.spRec = s.spRec.p; s.P.t = s.t.p; s.P.dt = s.dt.p;
            s.P.sync = -1;  // the synchronous particle stays on the host
            upload_lattice(s, L);
        }
        upload(b, nullptr, nullptr, nullptr, kDevAll & ~(kDevOld | kDevRec | kDevSpRec));
        for (auto& sp : sh_) {
            Shard& s = *sp;
            CK(cudaSetDevice(s.dev));
            s.old.zero();
            s.rec.zero();
            s.spRec.zero();
            const size_t cnt = static_cast<size_t>(s.n());
            s.weight.up(b.weight.data() + s.lo, cnt);
            s.qOverM.up(b.qOverM.data() + s.lo, cnt);
            s.charge.up(b.charge.data() + s.lo, cnt);
            s.restMass.up(b.restMass.data() + s.lo, cnt);
            s.t.up(b.t.data() + s.lo, cnt);
            s.dt.up(b.dt.data() + s.lo, cnt);
        }
    }

    void set_elems(const LatHost& L) override {
        for (auto& sp : sh_) {
            CK(cudaSetDevice(sp->dev));
            sp->elems.up(L.elems.data(), L.elems.size());
        }
    }

    void download(Beam& b, double* old, double* rec, double* spRec, unsigned mask) override {
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            const size_t c = static_cast<size_t>(s.n());
            const size_t o = static_cast<size_t>(s.lo);
            if (mask & kDevCoord) {
                s.x.down(b.x.data() + o, c); s.ux.down(b.ux.data() + o, c); s.y.down(b.y.data() + o, c);
                s.uy.down(b.uy.data() + o, c); s.z.down(b.z.data() + o, c); s.uz.down(b.uz.data() + o, c);
            }
            if (mask & kDevField) {
                s.ex.down(b.ex.data() + o, c); s.ey.down(b.ey.data() + o, c); s.ez.down(b.ez.data() + o, c);
                s.bx.down(b.bx.data() + o, c); s.by.down(b.by.data() + o, c); s.bz.down(b.bz.data() + o, c);
            }
            if (mask & kDevSc) {
                s.scx.down(b.scx.data() + o, c); s.scy.down(b.scy.data() + o, c); s.scz.down(b.scz.data() + o, c);
            }
            if (mask & kDevRel)
                for (int j = 0; j < 6; ++j) s.rel[j].down(b.rel[j].data() + o, c);
            if (mask & kDevLost) s.lost.down(b.lost.data() + o, c);
            if (mask & kDevIdx) {
                s.elemIndex.down(b.elemIndex.data() + o, c); s.nextPlane.down(b.nextPlane.data() + o, c);
                s.boundaryMode.down(b.boundaryMode.data() + o, c); s.geomIndex.down(b.geomIndex.data() + o, c);
                s.spOutCount.down(b.spOutCount.data() + o, c);
            }
            if (mask & kDevSpz) {
                s.spOutZ0.down(b.spOutZ0.data() + o, c);
                s.spOutZ1.down(b.spOutZ1.data() + o, c);
            }
            if ((mask & kDevOld) && old) s.old.down(old + 6 * o, 6 * c);
            if ((mask & kDevRec) && rec) s.rec.down(rec + 8 * o, 8 * c);
            if ((mask & kDevSpRec) && spRec && spRecOn_) s.spRec.down(spRec + 8 * o, 8 * c);
        }
        if (mpi_world().size > 1) {
            if (mask & kDevCoord)
                for (auto* v : {&b.x, &b.ux, &b.y, &b.uy, &b.z, &b.uz}) mpi_gather_particles(v->data(), 1);
            if (mask & kDevField)
                for (auto* v : {&b.ex, &b.ey, &b.ez, &b.bx, &b.by, &b.bz}) mpi_gather_particles(v->data(), 1);
            if (mask & kDevSc)
                for (auto* v : {&b.scx, &b.scy, &b.scz}) mpi_gather_particles(v->data(), 1);
            if (mask & kDevRel)
                for (auto& v : b.rel) mpi_gather_particles(v.data(), 1);
            if (mask & kDevLost) mpi_gather_particles(b.lost.data(), 1);
            if (mask & kDevIdx)
                for (auto* v : {&b.elemIndex, &b.nextPlane, &b.boundaryMode, &b.geomIndex, &b.spOutCount})
                    mpi_gather_particles(v->data(), 1);
            if (mask & kDevSpz)
                for (auto* v : {&b.spOutZ0, &b.spOutZ1}) mpi_gather_particles(v->data(), 1);
            if ((mask & kDevOld) && old) mpi_gather_particles(old, 6);
            if ((mask & kDevRec) && rec) mpi_gather_particles(rec, 8);
            if ((mask & kDevSpRec) && spRec && spRecOn_) mpi_gather_particles(spRec, 8);
        }
    }

    void upload(const Beam& b, const double* old, const double* rec, const double* spRec, unsigned mask) override {
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            const size_t c = static_cast<size_t>(s.n());
            const size_t o = static_cast<size_t>(s.lo);
            if (mask & kDevCoord) {
                s.x.up(b.x.data() + o, c); s.ux.up(b.ux.data() + o, c); s.y.up(b.y.data() + o, c);
                s.uy.up(b.uy.data() + o, c); s.z.up(b.z.data() + o, c); s.uz.up(b.uz.data() + o, c);
            }
            if (mask & kDevField) {
                s.ex.up(b.ex.data() + o, c); s.ey.up(b.ey.data() + o, c); s.ez.up(b.ez.data() + o, c);
                s.bx.up(b.bx.data() + o, c); s.by.up(b.by.data() + o, c); s.bz.up(b.bz.data() + o, c);
            }
            if (mask & kDevSc) {
                s.scx.up(b.scx.data() + o, c); s.scy.up(b.scy.data() + o, c); s.scz.up(b.scz.data() + o, c);
            }
            if (mask & kDevRel)
                for (int j = 0; j < 6; ++j) s.rel[j].up(b.rel[j].data() + o, c);
            if (mask & kDevLost) s.lost.up(b.lost.data() + o, c);
            if (mask & kDevIdx) {
                s.elemIndex.up(b.elemIndex.data() + o, c); s.nextPlane.up(b.nextPlane.data() + o, c);
                s.boundaryMode.up(b.boundaryMode.data() + o, c); s.geomIndex.up(b.geomIndex.data() + o, c);
                s.spOutCount.up(b.spOutCount.data() + o, c);
            }
            if (mask & kDevSpz) {
                s.spOutZ0.up(b.spOutZ0.data() + o, c);
                s.spOutZ1.up(b.spOutZ1.data() + o, c);
            }
            if ((mask & kDevOld) && old) s.old.up(old + 6 * o, 6 * c);
            if ((mask & kDevRec) && rec) s.rec.up(rec + 8 * o, 8 * c);
            if ((mask & kDevSpRec) && spRec && spRecOn_) s.spRec.up(spRec + 8 * o, 8 * c);
        }
    }

    void zero_rec(bool sp) override {
        for (auto& s : sh_) {
            CK(cudaSetDevice(s->dev));
            if (sp) s->spRec.zero();
            else s->rec.zero();
        }
    }

    void step(const LatScalars& ls, const kern::StepArgs& a0, const std::vector<double>& rf,
              const std::vector<double>& events, std::vector<StepGroup>& out, long long& spCount,
              long long& recCount) override {
        const auto tp0 = std::chrono::steady_clock::now();
        const int ng = red_groups(n_);
        StepGroup empty{};
        empty.zmin = 1e10;
        empty.zmax = -1e10;
        out.assign(static_cast<size_t>(ng), empty);
        std::vector<double> gbuf;
        spCount = recCount = 0;
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            set_scalars(s, ls);
            s.rf.set(rf.empty() ? std::vector<double>{0.0} : rf);
            s.events.set(events.empty() ? std::vector<double>{0.0} : events);
            s.counters.zero();
            kern::StepArgs a = a0;
            a.events = s.events.p;
            StepF f{s.L, s.P, a, kern::RfSrc{s.rf.p, a.tEnd}, s.counters.p};
            if (atomic_) launch_atomic<kStepK, 8>(s, f);
            else launch_reduce<kStepK, 8>(s, f);
        }
        if (prof_) sync_all();
        const auto tp1 = std::chrono::steady_clock::now();
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            int sg = groups_of(s);
            constexpr int W = kStepK + 8;
            if (atomic_) sg = 1;  // one total in group slot of the shard
            gbuf.resize(static_cast<size_t>(sg) * W);
            s.groups.down(gbuf.data(), gbuf.size());
            const int g0 = s.lo / kRedGroup;
            for (int g = 0; g < sg; ++g) {
                StepGroup& o = out[static_cast<size_t>(g0 + g)];
                const double* v = gbuf.data() + static_cast<size_t>(g) * W;
                for (int k = 0; k < kern::kStepSums; ++k) o.s[k] = v[k];
                o.sp = v[kern::kStepSums];
                o.rec = v[kern::kStepSums + 1];
                for (int j = 0; j < 6; ++j) o.mx[j] = v[kStepK + j];
                o.zmin = -v[kStepK + 6];
                o.zmax = v[kStepK + 7];
            }
        }
        if (mpi_world().size > 1) mpi_allgatherv(out.data(), rankGOff_, rankGCnt_);
        for (const StepGroup& o : out) {
            spCount += static_cast<long long>(o.sp);
            recCount += static_cast<long long>(o.rec);
        }
        if (prof_) {
            const auto tp2 = std::chrono::steady_clock::now();
            profK_ += std::chrono::duration<double>(tp1 - tp0).count();
            profD_ += std::chrono::duration<double>(tp2 - tp1).count();
            ++profN_;
        }
    }

    void ubp1(const double* s6, double* sums) override {
        Ubp1F f{};
        for (int j = 0; j < 6; ++j) f.s6[j] = s6[j];
        reduce_all<kern::kUbpSums, 0>([&](Shard& s) { f.P = s.P; return f; }, sums, nullptr);
    }

    void ubp2(const double* cen, double igz, double* q3, double& zmin, double& zmax) override {
        Ubp2F f{};
        for (int j = 0; j < 6; ++j) f.cen[j] = cen[j];
        f.igz = igz;
        double m[2];
        reduce_all<3, 2>([&](Shard& s) { f.P = s.P; return f; }, q3, m);
        zmin = -m[0];
        zmax = m[1];
    }

    void rows(const kern::RowOp& op) override {
        for_shards([&](Shard& s) { k_rows<<<blocks(s.n()), 256>>>(op, s.P, s.n()); });
    }

    void rows_tref(bool average, double& tRef) override {
        double s2[2], m[1];
        reduce_all<2, 1>([&](Shard& s) { return TrefF{s.P}; }, s2, m);
        tRef = average ? s2[0] / s2[1] : (m[0] == -1e10 ? 1e10 : -m[0]);
    }

    void rows_kick(int kind, double wx, double wy, double len) override {
        for_shards([&](Shard& s) { k_kick<<<blocks(s.n()), 256>>>(kind, wx, wy, len, s.P, s.n()); });
    }

    void ztot(double zPlane, double tRef, const double* sync) override {
        (void)sync;
        for_shards([&](Shard& s) { k_ztot<<<blocks(s.n()), 256>>>(s.P, s.n(), zPlane, tRef); });
    }

    void rec_tmin(bool sp, double& tmin) override {
        double m[1];
        if (sp) reduce_all<0, 1>([&](Shard& s) { return SpTminF{s.P}; }, nullptr, m);
        else {
            double s2[2];
            reduce_all<2, 1>([&](Shard& s) { return TrefF{s.P}; }, s2, m);
        }
        tmin = m[0] == -1e10 ? 1e10 : -m[0];
    }

    void hesc1(double tRef, double* sums) override {
        reduce_all<kern::kHescSums, 0>([&](Shard& s) { return Hesc1F{s.P, tRef}; }, sums, nullptr);
    }

    void hesc2(const double* cen, double gc, double* q3) override {
        Hesc2F f{};
        for (int j = 0; j < 6; ++j) f.cen[j] = cen[j];
        f.gc = gc;
        reduce_all<3, 0>([&](Shard& s) { f.P = s.P; return f; }, q3, nullptr);
    }

    void sc_deposit(const kern::MeshV& M0, std::vector<long long>& acc) override {
        const long long n3 = static_cast<long long>(M0.nx) * M0.ny * M0.nz;
        acc.assign(static_cast<size_t>(n3), 0);
        std::vector<long long> part(static_cast<size_t>(n3));
        for (auto& sp : sh_) {
            Shard& s = *sp;
            CK(cudaSetDevice(s.dev));
            s.xs.up_alloc(M0.xs, M0.nx);
            s.ys.up_alloc(M0.ys, M0.ny);
            s.zs.up_alloc(M0.zs, M0.nz);
            s.grid.alloc(static_cast<size_t>(n3));
            s.grid.zero();
            if (s.n() == 0) continue;
            kern::MeshV M = M0;
            M.xs = s.xs.p;
            M.ys = s.ys.p;
            M.zs = s.zs.p;
            if (atomic_) {
                s.gridD.alloc(static_cast<size_t>(n3));
                s.gridD.zero();
                k_deposit_atomic<<<blocks(s.n()), 256>>>(M, s.P, s.n(), s.gridD.p);
            } else k_deposit<<<blocks(s.n()), 256>>>(M, s.P, s.n(), s.grid.p);
        }
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            CK(cudaGetLastError());
            if (atomic_) {
                std::vector<double> d(static_cast<size_t>(n3));
                s.gridD.down(d.data(), d.size());
                for (long long e = 0; e < n3; ++e) part[static_cast<size_t>(e)] = kern::fix_charge(d[static_cast<size_t>(e)], M0.fixScaleExp);
            } else {
                CK(cudaMemcpy(part.data(), s.grid.p, static_cast<size_t>(n3) * 8, cudaMemcpyDeviceToHost));
            }
            for (long long e = 0; e < n3; ++e)
                acc[static_cast<size_t>(e)] = static_cast<long long>(static_cast<unsigned long long>(acc[static_cast<size_t>(e)]) +
                                                                     static_cast<unsigned long long>(part[static_cast<size_t>(e)]));
        }
        mpi_allreduce_sum(acc.data(), n3);
    }

    // Green table of SPICNIC / PICNIC on the device of shard s.
    void green_table(Shard& s, const SpGrid& g, const ScSolveIn& in) {
        const long long nF = static_cast<long long>(g.mx + 1) * (g.my + 1) * (g.mz + 1);
        const long long nC = static_cast<long long>(g.mx) * g.my * g.mz;
        const size_t nG = static_cast<size_t>(2 * in.nx + 1) * (2 * in.ny + 1) * (2 * in.nz + 1);
        s.gF.alloc(static_cast<size_t>(nF));
        s.G.alloc(nG);
        k_green_corner<<<blocks(nF), 256>>>(g, in.dx, in.dy, in.dz, s.gF.p);
        k_green_cell<<<blocks(nC), 256>>>(g, s.gF.p, in.vol, s.G.p);
    }

    // SPICNIC correction list on the device (stable compaction keeps the order of the CPU engine).
    long long corr_list(Shard& s, const SpGrid& g, const ScSolveIn& in) {
        const long long nq = 8 * g.n_targets();
        s.cdv.alloc(static_cast<size_t>(nq));
        s.cflag.alloc(static_cast<size_t>(nq));
        s.cidx.alloc(static_cast<size_t>(nq));
        s.csel.alloc(1);
        k_corr_val<<<blocks(nq), 256>>>(g, s.rho.p, in.thr, nq, s.cdv.p, s.cflag.p);
        cub::CountingInputIterator<int> it(0);
        size_t bytes = 0;
        CK(cub::DeviceSelect::Flagged(nullptr, bytes, it, s.cflag.p, s.cidx.p, s.csel.p, static_cast<int>(nq)));
        s.ctmp.alloc(std::max<size_t>(bytes, 1));
        CK(cub::DeviceSelect::Flagged(s.ctmp.p, bytes, it, s.cflag.p, s.cidx.p, s.csel.p, static_cast<int>(nq)));
        int nc = 0;
        s.csel.down(&nc, 1);
        s.corr.alloc(static_cast<size_t>(std::max(3 * nc, 1)));
        s.dval.alloc(static_cast<size_t>(std::max(nc, 1)));
        if (nc > 0) k_corr_expand<<<blocks(nc), 256>>>(g, s.rho.p, in.thr, s.cidx.p, s.cdv.p, nc, s.corr.p, s.dval.p);
        return nc;
    }

    void sc_solve(const ScSolveIn& in) override {
        const long long n3 = static_cast<long long>(in.nx) * in.ny * in.nz;
        n3_ = n3;
        const SpGrid g{in.nx, in.ny, in.nz, in.mx, in.my, in.mz};
        const double hx = 1.0 / (in.dx + in.dx), hy = 1.0 / (in.dy + in.dy), hz = 1.0 / (in.dz + in.dz);
        std::vector<Shard*> act;
        for (auto& sp : sh_)
            if (sp->n() > 0) act.push_back(sp.get());
        for (Shard* sp : act) {
            Shard& s = *sp;
            CK(cudaSetDevice(s.dev));
            s.rho.up_alloc(in.rho, n3);
            s.phi.alloc(static_cast<size_t>(n3));
            s.gex.alloc(static_cast<size_t>(n3));
            s.gey.alloc(static_cast<size_t>(n3));
            s.gez.alloc(static_cast<size_t>(n3));
            if (in.method == 1 || in.method == 2) {
                green_table(s, g, in);
            }
            if (in.method == 1) {
                const long long nc = corr_list(s, g, in);
                const long long nt = g.n_targets();
                k_sym<double><<<blocks(nt, 128), 128>>>(g, s.rho.p, s.G.p, in.thr, s.phi.p);
                if (nc > 0) k_corr<double><<<blocks(n3, 128), 128>>>(g, s.G.p, s.corr.p, s.dval.p, nc, s.phi.p);
            } else if (in.method == 0) {
                fft_solve(s, g, in, n3);
            }
            CK(cudaGetLastError());
        }
        if (in.method == 2 && !act.empty()) {
            // PICNIC: the nodes are split between the GPUs of this process, then the potential is
            // gathered on the host and sent to every GPU (each node is computed the same way anywhere)
            const long long na = static_cast<long long>(act.size());
            auto lo_of = [&](long long a) { return n3 * a / na; };
            for (long long a = 0; a < na; ++a) {
                Shard& s = *act[static_cast<size_t>(a)];
                CK(cudaSetDevice(s.dev));
                const long long lo = lo_of(a), hi = lo_of(a + 1);
                if (hi <= lo) continue;
                k_picnic<double><<<blocks(hi - lo, 128), 128>>>(g, s.rho.p, s.G.p, lo, hi, s.phi.p);
                CK(cudaGetLastError());
            }
            if (na > 1) {
                std::vector<double> phi(static_cast<size_t>(n3));
                for (long long a = 0; a < na; ++a) {
                    Shard& s = *act[static_cast<size_t>(a)];
                    CK(cudaSetDevice(s.dev));
                    const long long lo = lo_of(a), hi = lo_of(a + 1);
                    if (hi > lo)
                        CK(cudaMemcpy(phi.data() + lo, s.phi.p + lo, static_cast<size_t>(hi - lo) * 8, cudaMemcpyDeviceToHost));
                }
                for (Shard* sp : act) {
                    CK(cudaSetDevice(sp->dev));
                    sp->phi.up(phi.data(), static_cast<size_t>(n3));
                }
            }
        }
        for (Shard* sp : act) {
            Shard& s = *sp;
            CK(cudaSetDevice(s.dev));
            if (in.method == 0) k_grad_fft<<<blocks(n3), 256>>>(g, s.phi.p, hx, hy, hz, s.gex.p, s.gey.p, s.gez.p);
            else k_grad_spicnic<<<blocks(n3), 256>>>(g, s.phi.p, hx, hy, hz, s.gex.p, s.gey.p, s.gez.p);
            CK(cudaGetLastError());
        }
    }

    void sc_fields(std::vector<double>& ex, std::vector<double>& ey, std::vector<double>& ez) override {
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            ex.resize(static_cast<size_t>(n3_));
            ey.resize(static_cast<size_t>(n3_));
            ez.resize(static_cast<size_t>(n3_));
            s.gex.down(ex.data(), static_cast<size_t>(n3_));
            s.gey.down(ey.data(), static_cast<size_t>(n3_));
            s.gez.down(ez.data(), static_cast<size_t>(n3_));
            return;
        }
    }

    void sc_interp(const kern::MeshV& M0) override {
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            kern::MeshV M = M0;
            M.xs = s.xs.p;
            M.ys = s.ys.p;
            M.zs = s.zs.p;
            k_interp<<<blocks(s.n()), 256>>>(M, s.P, s.n(), s.gex.p, s.gey.p, s.gez.p);
        }
        sync_all();
    }

    void boundary(const LatScalars& ls, double t, double dt, std::vector<BoundaryHit>& hits) override {
        hits.clear();
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            set_scalars(s, ls);
            const int cap = std::max(1024, s.n() / 16);
            s.hits.alloc(static_cast<size_t>(cap));
            s.counters.zero();
            k_boundary<<<blocks(s.n()), 256>>>(s.L, s.P, s.n(), t, dt, s.hits.p, s.counters.p, cap);
            CK(cudaGetLastError());
            unsigned long long cnt[2];
            s.counters.down(cnt, 2);
            if (cnt[0] > static_cast<unsigned long long>(cap)) {
                // more hits than the buffer: run again with room for all (lost flags already set,
                // so only the remaining particles hit) -- simplest: grow and repeat on a copy
                throw EngineError("boundary.txt: too many collisions in one step for the GPU buffer");
            }
            std::vector<BoundaryHit> h(static_cast<size_t>(cnt[0]));
            s.hits.down(h.data(), h.size());
            for (BoundaryHit& x : h) x.i += s.lo;
            hits.insert(hits.end(), h.begin(), h.end());
        }
        mpi_gather_hits(hits);
    }

    void lp_fold(double zs, double full, double half) override {
        for_shards([&](Shard& s) { k_lp_fold<<<blocks(s.n()), 256>>>(s.P, s.n(), zs, full, half); });
    }

    void long_loss(double zs, double ws, double dW, double dzMax) override {
        for_shards([&](Shard& s) { k_long_loss<<<blocks(s.n()), 256>>>(s.P, s.n(), s.restMass.p, zs, ws, dW, dzMax); });
    }

    void secondary(const LatScalars& ls, const SeHost& se, const kern::SeV& v0, SeOut& out) override {
        const int nSlots = std::max(se.nSlots, 1);
        out.planes.assign(static_cast<size_t>(n_) * nSlots * 8, 0.0);
        out.hits.clear();
        struct Bufs {
            DBuf<kern::SeSegV> segs;
            DBuf<double> ev, bend, planes;
            DBuf<int> evMark, evSlot, el, elMark, elSlot;
            DBuf<kern::SeElemV> elems;
            DBuf<BoundaryHit> hits;
            DBuf<unsigned long long> count;
        };
        std::vector<std::unique_ptr<Bufs>> bufs;
        auto nz = [](const auto& v, auto dflt) { return v.empty() ? std::vector<decltype(dflt)>(1, dflt) : v; };
        const std::vector<double> bendRef(v0.bendRef, v0.bendRef + se.elems.size());
        for (auto& sp : sh_) {
            Shard& s = *sp;
            bufs.push_back(std::make_unique<Bufs>());
            if (s.n() == 0) continue;
            Bufs& B = *bufs.back();
            CK(cudaSetDevice(s.dev));
            set_scalars(s, ls);
            CK(cudaDeviceSetLimit(cudaLimitStackSize, 16384));
            B.segs.set(nz(se.segs, kern::SeSegV{}));
            B.ev.set(nz(se.ev, 0.0));
            B.evMark.set(nz(se.evMark, 0));
            B.evSlot.set(nz(se.evSlot, 0));
            B.el.set(nz(se.el, 0));
            B.elMark.set(nz(se.elMark, 0));
            B.elSlot.set(nz(se.elSlot, 0));
            B.elems.set(nz(se.elems, kern::SeElemV{}));
            B.bend.set(nz(bendRef, 0.0));
            B.planes.alloc(static_cast<size_t>(s.n()) * nSlots * 8);
            B.planes.zero();
            const int cap = std::max(1024, s.n());
            B.hits.alloc(static_cast<size_t>(cap));
            B.count.alloc(1);
            B.count.zero();
            kern::SeV v = v0;
            v.segs = B.segs.p; v.ev = B.ev.p; v.evMark = B.evMark.p; v.evSlot = B.evSlot.p;
            v.el = B.el.p; v.elMark = B.elMark.p; v.elSlot = B.elSlot.p; v.elems = B.elems.p; v.bendRef = B.bend.p;
            GpuSeSink sink{B.planes.p, nSlots, B.hits.p, B.count.p, cap};
            k_secondary<<<blocks(s.n(), 64), 64>>>(s.L, s.P, v, s.n(), sink);
            CK(cudaGetLastError());
        }
        for (size_t q = 0; q < sh_.size(); ++q) {
            Shard& s = *sh_[q];
            if (s.n() == 0) continue;
            Bufs& B = *bufs[q];
            CK(cudaSetDevice(s.dev));
            CK(cudaDeviceSynchronize());
            B.planes.down(out.planes.data() + static_cast<size_t>(s.lo) * nSlots * 8, static_cast<size_t>(s.n()) * nSlots * 8);
            unsigned long long cnt = 0;
            B.count.down(&cnt, 1);
            std::vector<BoundaryHit> h(static_cast<size_t>(std::min<unsigned long long>(cnt, B.hits.n)));
            B.hits.down(h.data(), h.size());
            for (BoundaryHit& x : h) x.i += s.lo;
            out.hits.insert(out.hits.end(), h.begin(), h.end());
        }
        mpi_gather_doubles(out.planes, static_cast<size_t>(nSlots) * 8);
        mpi_gather_hits(out.hits);
    }

    void download_times(Beam& b) override {
        for (auto& sp : sh_) {
            if (sp->n() == 0) continue;
            CK(cudaSetDevice(sp->dev));
            sp->t.down(b.t.data() + sp->lo, static_cast<size_t>(sp->n()));
        }
        mpi_gather_particles(b.t.data(), 1);
    }

private:
    std::vector<int> devs_;
    std::string desc_;
    bool atomic_;  // measurement only: non-deterministic atomic sums
    bool prof_ = std::getenv("AVAS_ENGINE_PROFILE") != nullptr;
    double profK_ = 0, profD_ = 0, profO_ = 0;
    long long profN_ = 0;
    int n_ = 0;
    bool spRecOn_ = false;
    long long n3_ = 0;
    std::vector<std::unique_ptr<Shard>> sh_;
    std::vector<long long> rankGOff_, rankGCnt_, rankPOff_, rankPCnt_;  // groups / particles of every MPI rank

    template <class T>
    void mpi_gather_particles(T* base, int per) {
        if (mpi_world().size <= 1) return;
        std::vector<long long> off(rankPOff_), cnt(rankPCnt_);
        for (auto& v : off) v *= per;
        for (auto& v : cnt) v *= per;
        mpi_allgatherv(base, off, cnt);
    }
    void mpi_gather_doubles(std::vector<double>& v, size_t per) { mpi_gather_particles(v.data(), static_cast<int>(per)); }
    void mpi_gather_hits(std::vector<BoundaryHit>& h) { mpi_allgather_list(h); }

    static int groups_of(const Shard& s) {
        const int nch = (s.n() + kThreads - 1) / kThreads;
        return (nch + kRedGroupChunks - 1) / kRedGroupChunks;
    }

    template <class Fn>
    void for_shards(Fn&& fn) {
        for (auto& sp : sh_) {
            if (sp->n() == 0) continue;
            CK(cudaSetDevice(sp->dev));
            fn(*sp);
            CK(cudaGetLastError());
        }
        sync_all();
    }

    void sync_all() {
        for (auto& sp : sh_) {
            CK(cudaSetDevice(sp->dev));
            CK(cudaDeviceSynchronize());
        }
    }

    template <int K, int M, class F>
    void launch_atomic(Shard& s, const F& f) {
        std::vector<double> init(static_cast<size_t>(K + M), 0.0);
        for (int k = 0; k < M; ++k) init[static_cast<size_t>(K + k)] = -1e300;
        s.groups.up(init.data(), init.size());
        const int nch = (s.n() + kThreads - 1) / kThreads;
        k_reduce_atomic<K, M, F><<<nch, kThreads>>>(f, s.n(), s.groups.p);
        CK(cudaGetLastError());
    }

    template <int K, int M, class F>
    void launch_reduce(Shard& s, const F& f) {
        const int nch = (s.n() + kThreads - 1) / kThreads;
        k_reduce_chunks<K, M, F><<<nch, kThreads>>>(f, s.n(), s.chunks.p);
        CK(cudaGetLastError());
        k_reduce_groups<K, M, F><<<groups_of(s), kRedGroupChunks>>>(f, s.chunks.p, nch, s.groups.p);
        CK(cudaGetLastError());
    }

    // Fixed-order sums over all particles: group results of every shard, summed in group order.
    template <int K, int M, class Make>
    void reduce_all(Make&& make, double* sums, double* ext) {
        std::vector<double> all;
        const int ng = red_groups(n_);
        all.assign(static_cast<size_t>(ng) * (K + M), 0.0);
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            auto f = make(s);
            launch_reduce<K, M>(s, f);
        }
        for (auto& sp : sh_) {
            Shard& s = *sp;
            if (s.n() == 0) continue;
            CK(cudaSetDevice(s.dev));
            s.groups.down(all.data() + static_cast<size_t>(s.lo / kRedGroup) * (K + M), static_cast<size_t>(groups_of(s)) * (K + M));
        }
        if (mpi_world().size > 1) {
            std::vector<long long> off(rankGOff_), cnt(rankGCnt_);
            for (auto& v : off) v *= (K + M);
            for (auto& v : cnt) v *= (K + M);
            mpi_allgatherv(all.data(), off, cnt);
        }
        if (K > 0) {
            for (int k = 0; k < K; ++k) sums[k] = 0.0;
            for (int g = 0; g < ng; ++g)
                for (int k = 0; k < K; ++k) sums[k] = sums[k] + all[static_cast<size_t>(g) * (K + M) + k];
        }
        if (M > 0) {
            for (int k = 0; k < M; ++k) {
                double v = -1e300;
                bool first = true;
                for (int g = 0; g < ng; ++g) {
                    const double x = all[static_cast<size_t>(g) * (K + M) + K + k];
                    if (first || x > v) v = x;
                    first = false;
                }
                ext[k] = ng ? v : -1e10;
            }
        }
    }

    void set_scalars(Shard& s, const LatScalars& ls) {
        s.L.spOutCur = ls.spOutCur;
        s.L.spOutCount = ls.spOutCount;
        s.L.spOutSize = ls.spOutSize;
        s.L.totalLength = ls.totalLength;
        s.L.spExit = ls.spExit;
    }

    void upload_lattice(Shard& s, const LatHost& L) {
        s.elems.set(L.elems);
        if (L.hasEff) s.eff.set(L.eff);
        s.subs.set(L.subs.empty() ? std::vector<kern::SubV>(1) : L.subs);
        std::vector<MapView<double>> mv = L.maps;
        s.mapData.clear();
        for (size_t k = 0; k < mv.size(); ++k) {
            const MapView<double>& m = L.maps[k];
            const size_t cnt = static_cast<size_t>(m.nx + 1) * (m.ny + 1) * (m.nz + 1) * m.ncomp;
            auto d = std::make_unique<DBuf<double>>();
            d->alloc(std::max<size_t>(cnt, 1));
            d->up(m.data, cnt);
            mv[k].data = d->p;
            s.mapData.push_back(std::move(d));
        }
        s.maps.set(mv.empty() ? std::vector<MapView<double>>(1) : mv);
        s.zStart.set(L.zStart);
        s.zEnd.set(L.zEnd);
        s.spOut.set(L.spOut.empty() ? std::vector<kern::SpOutV>(1) : L.spOut);
        s.slots.set(L.slots.empty() ? std::vector<kern::RfSlotV>(1) : L.slots);
        s.cavTime.set(L.cavTime.empty() ? std::vector<double>(1) : L.cavTime);
        s.cavPhase.set(L.cavPhase.empty() ? std::vector<double>(1) : L.cavPhase);
        s.geoms.set(L.geoms.empty() ? std::vector<kern::GeomV>(1) : L.geoms);
        s.segStart.set(L.segStart.empty() ? std::vector<double>(1) : L.segStart);
        s.segEnd.set(L.segEnd.empty() ? std::vector<double>(1) : L.segEnd);
        s.bndZ.set(L.bndZ.empty() ? std::vector<double>(1) : L.bndZ);
        s.bndR1.set(L.bndR1.empty() ? std::vector<int>(1) : L.bndR1);
        s.bndR2.set(L.bndR2.empty() ? std::vector<int>(1) : L.bndR2);
        kern::LatV v{};
        v.elems = s.elems.p;
        v.eff = L.hasEff ? s.eff.p : nullptr;
        v.subs = s.subs.p;
        v.maps = s.maps.p;
        v.elemZStart = s.zStart.p;
        v.elemZEnd = s.zEnd.p;
        v.spOut = s.spOut.p;
        v.cavTime = s.cavTime.p;
        v.cavPhase = s.cavPhase.p;
        v.rfSlots = s.slots.p;
        v.nElem = static_cast<int>(L.elems.size());
        v.errOn = L.errOn ? 1 : 0;
        v.geoms = s.geoms.p;
        v.segStart = s.segStart.p;
        v.segEnd = s.segEnd.p;
        v.nSeg = static_cast<int>(L.segEnd.size());
        v.bndZ = s.bndZ.p;
        v.bndR1 = s.bndR1.p;
        v.bndR2 = s.bndR2.p;
        v.nBnd = static_cast<int>(L.bndZ.size());
        s.L = v;
    }

    void fft_solve(Shard& s, const SpGrid& g, const ScSolveIn& in, long long n3) {
        for (auto& w : s.work) w.alloc(static_cast<size_t>(n3));
        const sck::FftTables T = sck::fft_tables(in.nx, in.ny, in.nz);
        s.tabs[0].set(T.sx);
        s.tabs[1].set(T.sy);
        s.tabs[2].set(T.cz);
        s.tabs[3].set(T.sz);
        std::vector<double> lam(in.lx, in.lx + in.nx);
        lam.insert(lam.end(), in.ly, in.ly + in.ny);
        lam.insert(lam.end(), in.lz, in.lz + in.nz);
        s.tabs[4].set(lam);
        const double* lx = s.tabs[4].p;
        const double* ly = lx + in.nx;
        const double* lz = ly + in.ny;
        double* A = s.work[0].p;
        double* B = s.work[1].p;
        double* C = s.work[2].p;
        double* D = s.work[3].p;
        k_scale<<<blocks(n3), 256>>>(s.rho.p, in.scale, A, n3);
        k_dst<<<blocks(n3), 256>>>(g, 0, A, B, s.tabs[0].p);
        k_dst<<<blocks(n3), 256>>>(g, 1, B, A, s.tabs[1].p);
        k_dft_z<<<blocks(n3), 256>>>(g, A, nullptr, C, D, s.tabs[2].p, s.tabs[3].p, -1.0);
        k_green_mul<<<blocks(n3), 256>>>(g, lx, ly, lz, C, D);
        k_dft_z<<<blocks(n3), 256>>>(g, C, D, A, B, s.tabs[2].p, s.tabs[3].p, 1.0);
        k_dst<<<blocks(n3), 256>>>(g, 0, A, C, s.tabs[0].p);
        k_dst<<<blocks(n3), 256>>>(g, 1, C, D, s.tabs[1].p);
        k_norm<<<blocks(n3), 256>>>(D, in.norm, s.phi.p, n3);
    }
};

}  // namespace cu

// ---- device selection ----

namespace {

bool full_rate_fp64(const cudaDeviceProp& p) {
    // HPC parts with fp64 at half the fp32 rate: P100 (6.0), V100 (7.0), A100 / A30 (8.0), H100 (9.0)
    return p.minor == 0 && (p.major == 6 || p.major == 7 || p.major == 8 || p.major == 9);
}

std::vector<int> parse_gpus(const std::string& s, int count, std::string& why) {
    std::vector<int> v;
    if (s.empty()) {
        v.push_back(0);
        return v;
    }
    if (s == "all") {
        for (int i = 0; i < count; ++i) v.push_back(i);
        return v;
    }
    std::stringstream ss(s);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        if (tok.find_first_not_of(' ') == std::string::npos) continue;
        const int d = std::atoi(tok.c_str());
        if (d < 0 || d >= count) {
            why = "gpus: device " + tok + " does not exist (" + std::to_string(count) + " visible)";
            return {};
        }
        if (std::find(v.begin(), v.end(), d) == v.end()) v.push_back(d);
    }
    if (v.empty()) why = "gpus: empty device list";
    return v;
}

}  // namespace

ProcessRank process_rank() {
#if defined(AVAS_WITH_MPI)
    if (options().device != Device::Cpu && !compat_dll()) {
        const cu::MpiWorld& w = cu::mpi_world();
        return ProcessRank{w.rank, w.size};
    }
#endif
    return ProcessRank{};
}

bool cuda_plan(long long nParticles, bool spaceCharge, DevicePlan& plan, std::string& why) {
    int count = 0;
    const cudaError_t e = cudaGetDeviceCount(&count);
    int drv = 0;
    cudaDriverGetVersion(&drv);
    if (drv == 0 && e != cudaSuccess) {
        why = std::string("no NVIDIA driver found (") + cudaGetErrorString(e) + ")";
        return false;
    }
    if (e == cudaErrorInsufficientDriver) {
        why = "the NVIDIA driver is too old (it supports CUDA " + std::to_string(drv / 1000) + "." +
              std::to_string((drv % 1000) / 10) + "; this engine needs a driver for CUDA 12, version 525.60 or later on "
              "Linux, 527.41 or later on Windows)";
        return false;
    }
    if (e != cudaSuccess || count == 0) {
        why = std::string("no usable NVIDIA GPU (") + (e != cudaSuccess ? cudaGetErrorString(e) : "no CUDA device") + ")";
        return false;
    }
    std::vector<int> devs = parse_gpus(options().gpus, count, why);
    if (devs.empty()) return false;
    const cu::MpiWorld& world = cu::mpi_world();
    if (world.size > 1 && options().gpus.empty()) devs.assign(1, world.localRank % count);  // one GPU per rank
    std::ostringstream desc;
    bool allFull = true;
    size_t minFree = static_cast<size_t>(-1);
    for (size_t k = 0; k < devs.size(); ++k) {
        cudaDeviceProp p{};
        if (cudaGetDeviceProperties(&p, devs[k]) != cudaSuccess) {
            why = "cannot read the properties of GPU " + std::to_string(devs[k]);
            return false;
        }
        if (p.major * 10 + p.minor < 60) {
            why = std::string("GPU ") + std::to_string(devs[k]) + " (" + p.name + ", sm_" + std::to_string(p.major) +
                  std::to_string(p.minor) + ") is older than Pascal; sm_60 or newer is needed";
            return false;
        }
        if (cudaSetDevice(devs[k]) != cudaSuccess) {
            why = "cannot use GPU " + std::to_string(devs[k]);
            return false;
        }
        int* d = nullptr;
        int h = 0;
        cudaError_t pe = cudaMalloc(reinterpret_cast<void**>(&d), sizeof(int));
        if (pe == cudaSuccess) {
            cu::k_probe<<<1, 1>>>(d);
            pe = cudaGetLastError();
            if (pe == cudaSuccess) pe = cudaMemcpy(&h, d, sizeof(int), cudaMemcpyDeviceToHost);
            cudaFree(d);
        }
        if (pe != cudaSuccess || h != 61) {
            why = std::string("GPU ") + std::to_string(devs[k]) + " (" + p.name + ") cannot run the engine kernels (" +
                  cudaGetErrorString(pe) + ")";
            return false;
        }
        size_t fr = 0, tot = 0;
        cudaMemGetInfo(&fr, &tot);
        minFree = std::min(minFree, fr);
        allFull = allFull && full_rate_fp64(p);
        desc << (k ? ", " : "") << devs[k] << " " << p.name << " (sm_" << p.major << p.minor << ", "
             << (tot >> 20) << " MiB)";
    }
    const Precision pr = options().precision;
    const bool fp32 = pr == Precision::Fp32 || (pr == Precision::Auto && !allFull);
    std::string reason = pr == Precision::Fp64   ? "requested"
                         : pr == Precision::Fp32 ? "requested"
                         : allFull               ? "auto: full-rate fp64 GPU"
                                                 : "auto: fp64 is slow on this GPU, coordinates relative to the "
                                                   "synchronous particle in fp32";
    if (nParticles > 0) {
        // memory estimate per GPU: particle arrays (fp64, or the float state of the fp32 tier) and the mesh
        const long long perGpu =
            (nParticles + static_cast<long long>(devs.size()) - 1) / static_cast<long long>(devs.size());
        const double bytesPer = fp32 ? 4.0 * 94 : 8.0 * 50 + 4.0 * 6;  // fp32: 82 floats + 8 ints + candidate lists
        const double need =
            bytesPer * static_cast<double>(perGpu) + (spaceCharge ? 64.0 * 1024 * 1024 : 0.0) + 32.0 * 1024 * 1024;
        if (need > 0.92 * static_cast<double>(minFree)) {
            char buf[200];
            std::snprintf(buf, sizeof buf, "not enough GPU memory: %lld particles need about %.2f GB per GPU, %.2f GB free",
                          nParticles, need / 1e9, static_cast<double>(minFree) / 1e9);
            why = buf;
            return false;
        }
    }
    plan.device = "cuda";
    plan.precision = fp32 ? "fp32" : "fp64";
    plan.gpus = devs;
    plan.reason = reason;
    plan.desc = desc.str();
    plan.error.clear();
    return true;
}

std::unique_ptr<DeviceBackend> make_cuda_backend(const DevicePlan& plan, std::string& why) {
    const cu::MpiWorld& world = cu::mpi_world();
    const bool fp32 = plan.precision == "fp32";
    const std::vector<int>& devs = plan.gpus;
    std::ostringstream full;
    if (world.size > 1) full << "MPI rank " << world.rank << " of " << world.size << ", ";
    full << "CUDA, " << devs.size() << " GPU" << (devs.size() > 1 ? "s" : "") << ": " << plan.desc << "; precision "
         << (fp32 ? "fp32" : "fp64") << " (" << plan.reason << ")";
    if (fp32) {
        // the fp32 tier (M6c phase 4): the device back end on the host with the fp32 kernels of
        // core/f32_kernels.h on the GPUs, as Metal; every MPI rank computes all particles
        full << ": t-code push, z-code rows, statistics, boundary.txt, lpboundary / longlimits and the space-charge mesh "
                "work (PICNIC, SPICNIC, FFT) on the GPU in fp32 (secondary particles on the CPU)";
        if (world.size > 1) full << "; every MPI rank computes all particles";
        std::unique_ptr<ScAccel> acc = make_cuda_f32_accel(devs, full.str(), why);
        if (!acc) return nullptr;
        return make_emu_backend(std::move(acc));
    }
    if (options().gpuAtomic) full << "; MEASUREMENT MODE: atomic sums, results not reproducible";
    return std::make_unique<cu::CudaBackend>(devs, full.str());
}

std::vector<CudaDeviceInfo> cuda_devices(std::string& why) {
    std::vector<CudaDeviceInfo> out;
    int count = 0;
    const cudaError_t e = cudaGetDeviceCount(&count);
    if (e != cudaSuccess) {
        why = cudaGetErrorString(e);
        return out;
    }
    for (int i = 0; i < count; ++i) {
        cudaDeviceProp p{};
        if (cudaGetDeviceProperties(&p, i) != cudaSuccess) continue;
        CudaDeviceInfo d;
        d.index = i;
        d.name = p.name;
        d.major = p.major;
        d.minor = p.minor;
        d.fp64Full = full_rate_fp64(p);
        d.memoryGB = static_cast<double>(p.totalGlobalMem) / 1e9;
        out.push_back(d);
    }
    return out;
}

}  // namespace avas
