// CUDA fp32 tier (M6c phase 4): the kernels of core/f32_kernels.h compiled by nvcc in the same
// "double = float" world as the Metal back end and the CPU fp32 tier (see hd.h, AVAS_F32_CUDA).  The
// namespace is renamed (avas -> avas_f32) so that these float definitions never meet the fp64 ones of
// the CUDA back end; the launch functions below take untyped pointers to the host mirrors of the
// structures (core/f32_host.cpp), whose layout is checked against sizes().  --fmad=false and IEEE
// division / square root make every operation the correctly rounded float operation of Metal and
// the CPU, so the three give the same bytes.
#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#define AVAS_F32_CUDA 1
#define avas avas_f32
#define double float
#include "core/hd.h"
#include "core/mesh_kernels.h"
#include "core/physics.h"
#include "core/kernels.h"
#include "core/sc_kernels.h"
#include "core/f32_kernels.h"
#undef double
#undef avas

namespace avas_f32 {
namespace cuk {

__global__ void k_step(kern::LatV L, f32::StepU U, f32::PartF F, const float* off, const float* rft, int s0, int s1, int s2,
                       int s3, int lo, int hi) {
    const int i = lo + static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    if (i >= hi) return;
    const int sizes[4] = {s0, s1, s2, s3};
    f32::step_particle(L, U, F, off, rft, sizes, i);
}

__global__ void k_bound(f32::BoundU U, f32::PartF F, const f32::GeomF* geoms, const float* segStart, const float* segEnd,
                        const float* bndZ, const int* bndR1, const int* bndR2, int lo, int hi) {
    const int i = lo + static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    if (i >= hi) return;
    f32::bound_particle(U, F, geoms, segStart, segEnd, bndZ, bndR1, bndR2, i);
}

// One reduction group (4096 particles) per block of 256, as k_group of kernels.metal: per column the
// largest |value| fixes e with |v| 2^e < 2^50, the values are summed as 64-bit integers (exact, so
// independent of the order); the maxima are exact in float.
__global__ void k_group(const float* C, const float* Mx, long long* sums, int* exps, float* mxo, int n, int stride, int ncol, int nmx,
                        int nmx0, int g0) {
    __shared__ float tf[256];
    __shared__ long long tl[256];
    const int g = g0 + static_cast<int>(blockIdx.x);
    const int t = static_cast<int>(threadIdx.x);
    const int lo = g * 4096, hi = min(n, lo + 4096);
    for (int col = 0; col < ncol; ++col) {
        float m = 0.0f;
        for (int p = lo + t; p < hi; p += 256) m = fmaxf(m, fabsf(C[static_cast<size_t>(col) * stride + p]));
        tf[t] = m;
        __syncthreads();
        for (int s2 = 128; s2 > 0; s2 >>= 1) {
            if (t < s2) tf[t] = fmaxf(tf[t], tf[t + s2]);
            __syncthreads();
        }
        const float gm = tf[0];
        __syncthreads();
        int ex = 0;
        if (gm > 0.0f) frexpf(gm, &ex);
        const int e = gm > 0.0f ? 50 - ex : 0;
        long long acc = 0;
        for (int p = lo + t; p < hi; p += 256) acc += static_cast<long long>(rintf(ldexpf(C[static_cast<size_t>(col) * stride + p], e)));
        tl[t] = acc;
        __syncthreads();
        for (int s2 = 128; s2 > 0; s2 >>= 1) {
            if (t < s2) tl[t] = tl[t] + tl[t + s2];
            __syncthreads();
        }
        if (t == 0) {
            sums[ncol * g + col] = tl[0];
            exps[ncol * g + col] = e;
        }
        __syncthreads();
    }
    for (int col = 0; col < nmx; ++col) {
        float m = col < nmx0 ? 0.0f : -INFINITY;
        for (int p = lo + t; p < hi; p += 256) m = fmaxf(m, Mx[static_cast<size_t>(col) * stride + p]);
        tf[t] = m;
        __syncthreads();
        for (int s2 = 128; s2 > 0; s2 >>= 1) {
            if (t < s2) tf[t] = fmaxf(tf[t], tf[t + s2]);
            __syncthreads();
        }
        if (t == 0) mxo[nmx * g + col] = tf[0];
        __syncthreads();
    }
}

__global__ void k_zrow(f32::ZArgs A, f32::PartF F, int lo, int hi) {
    const int i = lo + static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    if (i >= hi) return;
    f32::zrow_particle(A, F, i);
}

__global__ void k_deposit(kern::MeshV M, const float* x, const float* y, const float* z, const float* q, const int* lost,
                          unsigned* lo, unsigned* hi, int p0, int p1) {
    const int p = p0 + static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    if (p >= p1) return;
    f32::deposit_particle(M, x, y, z, q, lost, lo, hi, p);
}

__global__ void k_interp(kern::MeshV M, const float* x, const float* y, const float* z, const int* lost, const float* ex,
                         const float* ey, const float* ez, float* sc, int p0, int p1) {
    const int p = p0 + static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
    if (p >= p1) return;
    f32::interp_particle(M, x, y, z, lost, ex, ey, ez, sc, p);
}

__global__ void k_picnic(sck::SpGrid g, const float* rho, const float* G, float* phi) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n3()) return;
    int i, j, k;
    g.ijk(id, i, j, k);
    phi[id] = sck::picnic_node<float>(g, rho, G, i, j, k);
}

__global__ void k_sym(sck::SpGrid g, const float* rho, const float* G, float thr, float* phi) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n_targets()) return;
    int i, j, k;
    g.target(id, i, j, k);
    sck::sym_target<float>(g, rho, G, thr, i, j, k, phi);
}

__global__ void k_corr(sck::SpGrid g, const float* G, const int* corr, const float* dval, long long nc, float* phi) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n3()) return;
    int a, b, c;
    g.ijk(id, a, b, c);
    sck::corr_node<float>(g, G, corr, dval, nc, a, b, c, phi);
}

__global__ void k_grad(sck::SpGrid g, const float* phi, float hx, float hy, float hz, float* ex, float* ey, float* ez) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= g.n3()) return;
    int i, j, k;
    g.ijk(id, i, j, k);
    sck::grad_spicnic_node(g, phi, hx, hy, hz, i, j, k, ex, ey, ez);
}

inline unsigned blocks(long long n, int t) { return static_cast<unsigned>((n + t - 1) / t); }

template <class T>
T from(const void* p) {
    T v;
    std::memcpy(&v, p, sizeof(T));
    return v;
}

}  // namespace cuk
}  // namespace avas_f32

namespace avas {
namespace f32cuda {

using namespace avas_f32;

void sizes(unsigned* o) {
    o[0] = sizeof(kern::MeshV);
    o[1] = sizeof(sck::SpGrid);
    o[2] = sizeof(kern::LatV);
    o[3] = sizeof(kern::ElemV);
    o[4] = sizeof(MapView<float, float>);
    o[5] = sizeof(f32::PartF);
    o[6] = sizeof(f32::StepU);
    o[7] = sizeof(kern::SubV);
    o[8] = sizeof(f32::GeomF);
    o[9] = sizeof(f32::BoundU);
    o[10] = sizeof(f32::ZArgs);
}

// byte offsets of the candidate counters, lists and entries in PartF (every GPU gets its own in device memory)
void partf_lists(size_t* o) {
    o[0] = offsetof(f32::PartF, crossCount);
    o[1] = offsetof(f32::PartF, crossIdx);
    o[2] = offsetof(f32::PartF, spIdx);
    o[3] = offsetof(f32::PartF, bdIdx);
    o[4] = offsetof(f32::PartF, cSnap);
    o[5] = offsetof(f32::PartF, cNp);
    o[6] = offsetof(f32::PartF, sSnap);
    o[7] = offsetof(f32::PartF, sElem);
}

void step(const void* L, const void* U, const void* F, const float* off, const float* rft, const int* sz, int lo, int hi) {
    if (hi <= lo) return;
    cuk::k_step<<<cuk::blocks(hi - lo, 128), 128>>>(cuk::from<kern::LatV>(L), cuk::from<f32::StepU>(U), cuk::from<f32::PartF>(F),
                                                    off, rft, sz[0], sz[1], sz[2], sz[3], lo, hi);
}

void bound(const void* U, const void* F, const void* geoms, const float* segStart, const float* segEnd, const float* bndZ,
           const int* bndR1, const int* bndR2, int lo, int hi) {
    if (hi <= lo) return;
    cuk::k_bound<<<cuk::blocks(hi - lo, 128), 128>>>(cuk::from<f32::BoundU>(U), cuk::from<f32::PartF>(F),
                                                     static_cast<const f32::GeomF*>(geoms), segStart, segEnd, bndZ, bndR1, bndR2,
                                                     lo, hi);
}

void group(const float* C, const float* Mx, long long* sums, int* exps, float* mx, int n, int stride, int ncol, int nmx, int nmx0,
           int g0, int g1) {
    if (g1 <= g0) return;
    cuk::k_group<<<static_cast<unsigned>(g1 - g0), 256>>>(C, Mx, sums, exps, mx, n, stride, ncol, nmx, nmx0, g0);
}

void zrow(const void* A, const void* F, int lo, int hi) {
    if (hi <= lo) return;
    cuk::k_zrow<<<cuk::blocks(hi - lo, 128), 128>>>(cuk::from<f32::ZArgs>(A), cuk::from<f32::PartF>(F), lo, hi);
}

void deposit(const void* M, const float* x, const float* y, const float* z, const float* q, const int* lost, unsigned* lo,
             unsigned* hi, int p0, int p1) {
    if (p1 <= p0) return;
    cuk::k_deposit<<<cuk::blocks(p1 - p0, 256), 256>>>(cuk::from<kern::MeshV>(M), x, y, z, q, lost, lo, hi, p0, p1);
}

void interp(const void* M, const float* x, const float* y, const float* z, const int* lost, const float* ex, const float* ey,
            const float* ez, float* sc, int p0, int p1) {
    if (p1 <= p0) return;
    cuk::k_interp<<<cuk::blocks(p1 - p0, 256), 256>>>(cuk::from<kern::MeshV>(M), x, y, z, lost, ex, ey, ez, sc, p0, p1);
}

void picnic(const void* g, const float* rho, const float* G, float* phi) {
    const sck::SpGrid S = cuk::from<sck::SpGrid>(g);
    cuk::k_picnic<<<cuk::blocks(S.n3(), 128), 128>>>(S, rho, G, phi);
}

void sym(const void* g, const float* rho, const float* G, float thr, float* phi) {
    const sck::SpGrid S = cuk::from<sck::SpGrid>(g);
    if (S.n_targets() > 0) cuk::k_sym<<<cuk::blocks(S.n_targets(), 128), 128>>>(S, rho, G, thr, phi);
}

void corr(const void* g, const float* G, const int* corr, const float* dval, long long nc, float* phi) {
    const sck::SpGrid S = cuk::from<sck::SpGrid>(g);
    cuk::k_corr<<<cuk::blocks(S.n3(), 128), 128>>>(S, G, corr, dval, nc, phi);
}

void grad(const void* g, const float* phi, const float* h, float* ex, float* ey, float* ez) {
    const sck::SpGrid S = cuk::from<sck::SpGrid>(g);
    cuk::k_grad<<<cuk::blocks(S.n3(), 256), 256>>>(S, phi, h[0], h[1], h[2], ex, ey, ez);
}

}  // namespace f32cuda
}  // namespace avas
