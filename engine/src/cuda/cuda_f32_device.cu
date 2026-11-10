// CUDA fp32 tier (M6c phases 4 and 5): the fp32 device of core/f32_host.h on NVIDIA GPUs.
//
// The host logic is the one of the Metal back end (F32Accel, core/f32_host.cpp); this file allocates the
// memory, copies between host and device and launches the kernels of cuda_f32_kernels.cu.
// Memory (phase 5): ensure() buffers are device memory (cudaMalloc; with several GPUs one managed
// allocation per buffer, used only as a common address, never touched by the host) with a separate host
// copy exchanged by explicit copies (upload / download); ensure_shared() buffers are pinned host memory
// mapped into the device (the host's candidate and result lists).  No page of device data moves on demand,
// which also makes the tier independent of the Windows (WDDM) restrictions on managed memory.
// Several GPUs of one process split the particles by whole reduction groups (4096); every GPU gets its
// own candidate counters, lists and entries and its own deposit mesh (64-bit integer sums), combined on
// the host, so the result does not depend on the number of GPUs.  The mesh solve, the fp64 Green table
// and the SPICNIC correction list (the CPU engine's numbers) run on the first GPU.
#include <cuda_runtime.h>
#include <cub/device/device_select.cuh>
#include <cub/iterator/counting_input_iterator.cuh>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "core/errors.h"
#include "core/f32_host.h"
#include "core/reduce.h"
#include "core/sc_kernels.h"

namespace avas {

namespace f32cuda {  // cuda_f32_kernels.cu (compiled in the float world)
void sizes(unsigned* o);
void partf_lists(size_t* o);
void step(const void* L, const void* U, const void* F, const float* off, const float* rft, const int* sz, int lo, int hi);
void bound(const void* U, const void* F, const void* geoms, const float* segStart, const float* segEnd, const float* bndZ,
           const int* bndR1, const int* bndR2, int lo, int hi);
void group(const float* C, const float* Mx, long long* sums, int* exps, float* mx, int n, int stride, int ncol, int nmx, int nmx0,
           int g0, int g1);
void zrow(const void* A, const void* F, int lo, int hi);
void fft(const void* g, const void* P, long long n3);
void deposit64(const void* M, const float* x, const float* y, const float* z, const float* q, const int* lost,
               unsigned long long* acc, int p0, int p1);
void interp(const void* M, const float* x, const float* y, const float* z, const int* lost, const float* ex, const float* ey,
            const float* ez, float* sc, int p0, int p1);
void picnic(const void* g, const float* rho, const float* G, float* phi);
void sym(const void* g, const float* rho, const float* G, float thr, float* phi);
void corr(const void* g, const float* G, const int* corr, const float* dval, long long nc, float* phi);
void grad(const void* g, const float* phi, const float* h, float* ex, float* ey, float* ez);
}  // namespace f32cuda

namespace {

void ck(cudaError_t e, const char* what) {
    if (e != cudaSuccess) throw EngineError(std::string("CUDA fp32 (") + what + "): " + cudaGetErrorString(e));
}

inline unsigned nblk(long long n, int t = 256) { return static_cast<unsigned>((n + t - 1) / t); }

// ---- fp64 mesh tables (the kernels of the fp64 back end, sc_kernels.h: the CPU engine's numbers) ----
__global__ void g_corner(sck::SpGrid g, double dx, double dy, double dz, double* F) {
    const long long ay = g.my + 1, az = g.mz + 1;
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= (g.mx + 1) * ay * az) return;
    F[id] = sck::green_corner(static_cast<int>(id / (ay * az)), static_cast<int>((id / az) % ay), static_cast<int>(id % az),
                              dx, dy, dz);
}
__global__ void g_cell(sck::SpGrid g, const double* F, double vol, double* G) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id >= static_cast<long long>(g.mx) * g.my * g.mz) return;
    sck::green_cell(g, F, vol, static_cast<int>(id / (static_cast<long long>(g.my) * g.mz)),
                    static_cast<int>((id / g.mz) % g.my), static_cast<int>(id % g.mz), G);
}
__global__ void g_tofloat(const double* a, float* b, long long n) {
    const long long id = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (id < n) b[id] = static_cast<float>(a[id]);
}
__global__ void g_corr_val(sck::SpGrid g, const double* rho, double thr, long long nq, double* dv, int* flag) {
    const long long q = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (q >= nq) return;
    int I, J, K;
    const double d = sck::corr_value(g, rho, thr, q / 8, static_cast<int>(q % 8), I, J, K);
    dv[q] = d;
    flag[q] = d != 0.0 ? 1 : 0;
}
__global__ void g_corr_expand(sck::SpGrid g, const double* rho, double thr, const int* idx, const double* dv, long long nc,
                              int* corr, float* dval) {
    const long long e = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (e >= nc) return;
    const long long q = idx[e];
    int I, J, K;
    sck::corr_value(g, rho, thr, q / 8, static_cast<int>(q % 8), I, J, K);
    corr[3 * e] = I;
    corr[3 * e + 1] = J;
    corr[3 * e + 2] = K;
    dval[e] = static_cast<float>(dv[q]);
}

template <class T>
T* D(F32Slice s) { return reinterpret_cast<T*>(s.dev()); }

class CudaF32Device : public F32Device {
public:
    CudaF32Device(std::vector<int> devs, std::string desc) : devs_(std::move(devs)), desc_(std::move(desc)) {}
    ~CudaF32Device() override {
        sync();
        for (auto& r : recs_) release(*r);
        for (Gpu& g : gpu_) {
            cudaSetDevice(g.dev);
            for (void* p : {static_cast<void*>(g.cnt), static_cast<void*>(g.idx), static_cast<void*>(g.ent), static_cast<void*>(g.grp),
                            static_cast<void*>(g.acc), static_cast<void*>(g.fld)})
                cudaFree(p);
        }
        first();
        for (void* p : {tmp_[0], tmp_[1], tmp_[2], tmp_[3], tmp_[4]}) cudaFree(p);
    }

    bool init(std::string& why) override {
        (void)why;
        f32cuda::partf_lists(lists_);
        unsigned sz[16] = {0};
        f32cuda::sizes(sz);
        partSize_ = sz[5];
        gpu_.resize(devs_.size());
        for (size_t k = 0; k < devs_.size(); ++k) gpu_[k].dev = devs_[k];
        first();
        return true;
    }
    std::string describe() const override { return desc_; }

    // ---- memory ----
    void ensure(F32Buf& b, size_t bytes) override { alloc(b, bytes, false); }
    void ensure_shared(F32Buf& b, size_t bytes) override { alloc(b, bytes, true); }
    void upload(F32Buf& b, size_t off, size_t bytes) override {
        Rec* r = static_cast<Rec*>(b.impl);
        if (!r || r->shared || bytes == 0) return;
        first();
        ck(cudaMemcpy(static_cast<char*>(r->dev) + off, static_cast<char*>(r->host) + off, bytes, cudaMemcpyHostToDevice), "upload");
    }
    void download(F32Buf& b, size_t off, size_t bytes) override {
        Rec* r = static_cast<Rec*>(b.impl);
        if (!r || r->shared || bytes == 0) return;
        sync();
        first();
        ck(cudaMemcpy(static_cast<char*>(r->host) + off, static_cast<char*>(r->dev) + off, bytes, cudaMemcpyDeviceToHost), "download");
    }
    void tables(F32Buf& b) override {
        Rec* r = static_cast<Rec*>(b.impl);
        if (!r || !r->managed || r->readMostly) return;
        cudaMemAdvise(r->dev, r->bytes, cudaMemAdviseSetReadMostly, devs_[0]);
        cudaGetLastError();
        r->readMostly = true;
    }
    void particles(F32Buf& b, size_t colBytes, int n, size_t elem) override {
        Rec* r = static_cast<Rec*>(b.impl);
        if (!r || !r->managed || colBytes == 0) return;  // several GPUs: the particle columns by range on their GPU
        const size_t cols = b.len / colBytes;
        for (size_t k = 0; k < gpu_.size(); ++k) {
            int lo, hi;
            range(n, static_cast<int>(k), lo, hi);
            if (hi <= lo) continue;
            for (size_t c = 0; c < cols; ++c) {
                char* p = static_cast<char*>(r->dev) + c * colBytes + static_cast<size_t>(lo) * elem;
                const size_t len = static_cast<size_t>(hi - lo) * elem;
                cudaMemAdvise(p, len, cudaMemAdviseSetPreferredLocation, gpu_[k].dev);
                cudaMemPrefetchAsync(p, len, gpu_[k].dev);
            }
        }
        sync();
        cudaGetLastError();  // advice is only a hint
    }

    // ---- fp64 mesh tables on the first GPU ----
    bool green(const void* gp, double dx, double dy, double dz, double vol, F32Slice G) override {
        sck::SpGrid g;
        std::memcpy(&g, gp, sizeof g);
        const long long nF = static_cast<long long>(g.mx + 1) * (g.my + 1) * (g.mz + 1);
        const long long nC = static_cast<long long>(g.mx) * g.my * g.mz;
        const long long nG = static_cast<long long>(2 * g.nx + 1) * (2 * g.ny + 1) * (2 * g.nz + 1);
        first();
        double* F = static_cast<double*>(tmp(0, static_cast<size_t>(nF) * 8));
        double* Gd = static_cast<double*>(tmp(1, static_cast<size_t>(nG) * 8));
        ck(cudaMemsetAsync(Gd, 0, static_cast<size_t>(nG) * 8), "Green table");
        g_corner<<<nblk(nF), 256>>>(g, dx, dy, dz, F);
        g_cell<<<nblk(nC), 256>>>(g, F, vol, Gd);
        g_tofloat<<<nblk(nG), 256>>>(Gd, D<float>(G), nG);
        done("Green table");
        return true;
    }
    bool rho_corr(const void* gp, F32Slice rhoD, F32Slice rhoF, long long n3, bool withCorr, double thr, F32Slice corr,
                  F32Slice dval, long long& nc) override {
        sck::SpGrid g;
        std::memcpy(&g, gp, sizeof g);
        first();
        g_tofloat<<<nblk(n3), 256>>>(D<double>(rhoD), D<float>(rhoF), n3);
        nc = 0;
        if (withCorr) {  // stable compaction: the order of the CPU engine's list
            const long long nq = 8 * g.n_targets();
            double* dv = static_cast<double*>(tmp(2, static_cast<size_t>(std::max<long long>(nq, 1)) * 8));
            int* flag = static_cast<int*>(tmp(3, static_cast<size_t>(std::max<long long>(nq, 1)) * 8 + 16));
            int* idx = flag + std::max<long long>(nq, 1);
            int* cnt = idx + std::max<long long>(nq, 1);
            g_corr_val<<<nblk(nq), 256>>>(g, D<double>(rhoD), thr, nq, dv, flag);
            cub::CountingInputIterator<int> it(0);
            size_t bytes = 0;
            ck(cub::DeviceSelect::Flagged(nullptr, bytes, it, flag, idx, cnt, static_cast<int>(nq)), "correction list");
            void* work = tmp(4, std::max<size_t>(bytes, 1));
            ck(cub::DeviceSelect::Flagged(work, bytes, it, flag, idx, cnt, static_cast<int>(nq)), "correction list");
            int n = 0;
            ck(cudaMemcpy(&n, cnt, 4, cudaMemcpyDeviceToHost), "correction list");
            nc = n;
            if (nc > 0) g_corr_expand<<<nblk(nc), 256>>>(g, D<double>(rhoD), thr, idx, dv, nc, D<int>(corr), D<float>(dval));
        }
        done("charge / correction list");
        return true;
    }

    void sizes(unsigned* o) override { f32cuda::sizes(o); }

    void step(const void* L, const void* U, const void* F, F32Slice off, F32Slice rft, const int* sz, int n) override {
        split(n, F, [&](int lo, int hi, const void* Fk) { f32cuda::step(L, U, Fk, D<float>(off), D<float>(rft), sz, lo, hi); }, 0, 2);
    }
    void bound(const void* U, const void* F, F32Slice geoms, F32Slice segStart, F32Slice segEnd, F32Slice bndZ, F32Slice bndR1,
               F32Slice bndR2, int n) override {
        split(n, F, [&](int lo, int hi, const void* Fk) {
            f32cuda::bound(U, Fk, D<void>(geoms), D<float>(segStart), D<float>(segEnd), D<float>(bndZ), D<int>(bndR1), D<int>(bndR2), lo, hi);
        }, 2, 3);
    }
    void zrow(const void* A, const void* F, int n) override {
        const int G = static_cast<int>(gpu_.size());
        for (int k = 0; k < G; ++k) {
            ck(cudaSetDevice(gpu_[static_cast<size_t>(k)].dev), "set device");
            int lo, hi;
            range(n, k, lo, hi);
            f32cuda::zrow(A, F, lo, hi);
            ck(cudaGetLastError(), "z-code rows");
        }
        sync();
    }
    void group(F32Slice C, F32Slice Mx, F32Slice sums, F32Slice exps, F32Slice mx, int n, int stride, int ncol, int nmx,
               int nmx0) override {
        // into device buffers (layout of the host buffers), then copied into the (shared) host buffers
        const int ng = red_groups(n), G = static_cast<int>(gpu_.size());
        const size_t bs = static_cast<size_t>(ng) * ncol * 8, be = static_cast<size_t>(ng) * ncol * 4,
                     bm = static_cast<size_t>(ng) * std::max(nmx, 1) * 4;
        for (int k = 0; k < G; ++k) {
            Gpu& g = gpu_[static_cast<size_t>(k)];
            ck(cudaSetDevice(g.dev), "set device");
            if (g.grpCap < bs + be + bm) {
                cudaFree(g.grp);
                ck(cudaMalloc(&g.grp, bs + be + bm), "group sums");
                g.grpCap = bs + be + bm;
            }
            char* d = static_cast<char*>(g.grp);
            const int g0 = static_cast<int>(static_cast<long long>(ng) * k / G), g1 = static_cast<int>(static_cast<long long>(ng) * (k + 1) / G);
            f32cuda::group(D<float>(C), D<float>(Mx), reinterpret_cast<long long*>(d), reinterpret_cast<int*>(d + bs),
                           reinterpret_cast<float*>(d + bs + be), n, stride, ncol, nmx, nmx0, g0, g1);
            ck(cudaGetLastError(), "group sums");
        }
        for (int k = 0; k < G; ++k) {
            Gpu& g = gpu_[static_cast<size_t>(k)];
            ck(cudaSetDevice(g.dev), "set device");
            const int g0 = static_cast<int>(static_cast<long long>(ng) * k / G), g1 = static_cast<int>(static_cast<long long>(ng) * (k + 1) / G);
            if (g1 <= g0) continue;
            const char* d = static_cast<const char*>(g.grp);
            const size_t s0 = static_cast<size_t>(g0) * ncol, sn = static_cast<size_t>(g1 - g0) * ncol;
            ck(cudaMemcpy(sums.ptr<long long>() + s0, d + s0 * 8, sn * 8, cudaMemcpyDeviceToHost), "group sums");
            ck(cudaMemcpy(exps.ptr<int>() + s0, d + bs + s0 * 4, sn * 4, cudaMemcpyDeviceToHost), "group sums");
            if (nmx > 0)
                ck(cudaMemcpy(mx.ptr<float>() + static_cast<size_t>(g0) * nmx, d + bs + be + static_cast<size_t>(g0) * nmx * 4,
                              static_cast<size_t>(g1 - g0) * nmx * 4, cudaMemcpyDeviceToHost),
                   "group sums");
        }
    }
    void deposit(const void* M, F32Slice x, F32Slice y, F32Slice z, F32Slice q, F32Slice lost, F32Slice lo, F32Slice hi,
                 int n) override {
        int nn[3];
        std::memcpy(nn, M, sizeof nn);  // MeshV starts with nx, ny, nz
        const size_t n3 = static_cast<size_t>(nn[0]) * nn[1] * nn[2];
        const int G = static_cast<int>(gpu_.size());
        for (int k = 0; k < G; ++k) {  // native 64-bit atomics: the same sums modulo 2^64 as the two words of add64
            Gpu& g = gpu_[static_cast<size_t>(k)];
            ck(cudaSetDevice(g.dev), "set device");
            if (g.n3 < n3) {
                cudaFree(g.acc);
                ck(cudaMalloc(reinterpret_cast<void**>(&g.acc), n3 * 8), "deposit mesh");
                g.n3 = n3;
            }
            ck(cudaMemsetAsync(g.acc, 0, n3 * 8), "deposit mesh");
            int p0, p1;
            range(n, k, p0, p1);
            f32cuda::deposit64(M, D<float>(x), D<float>(y), D<float>(z), D<float>(q), D<int>(lost), g.acc, p0, p1);
            ck(cudaGetLastError(), "deposit");
        }
        std::uint32_t* L = lo.ptr<std::uint32_t>();  // shared (host) buffers, zero on entry
        std::uint32_t* H = hi.ptr<std::uint32_t>();
        accHost_.resize(n3);
        for (int k = 0; k < G; ++k) {
            Gpu& g = gpu_[static_cast<size_t>(k)];
            ck(cudaSetDevice(g.dev), "set device");
            ck(cudaMemcpy(accHost_.data(), g.acc, n3 * 8, cudaMemcpyDeviceToHost), "deposit download");
            for (size_t e = 0; e < n3; ++e) {  // modulo 2^64: order independent
                const std::uint64_t v = ((static_cast<std::uint64_t>(H[e]) << 32) | L[e]) + accHost_[e];
                L[e] = static_cast<std::uint32_t>(v);
                H[e] = static_cast<std::uint32_t>(v >> 32);
            }
        }
    }
    void interp(const void* M, F32Slice x, F32Slice y, F32Slice z, F32Slice lost, F32Slice ex, F32Slice ey, F32Slice ez, F32Slice sc,
                int n) override {
        int nn[3];
        std::memcpy(nn, M, sizeof nn);
        const size_t n3 = static_cast<size_t>(nn[0]) * nn[1] * nn[2];
        const int G = static_cast<int>(gpu_.size());
        for (int k = 0; k < G; ++k) {
            Gpu& g = gpu_[static_cast<size_t>(k)];
            ck(cudaSetDevice(g.dev), "set device");
            int p0, p1;
            range(n, k, p0, p1);
            const float* f[3] = {D<float>(ex), D<float>(ey), D<float>(ez)};
            if (k > 0 && p1 > p0) {  // the field (written by the first GPU) copied once into this GPU's memory
                if (g.fldCap < 3 * n3) {
                    cudaFree(g.fld);
                    ck(cudaMalloc(reinterpret_cast<void**>(&g.fld), 3 * n3 * 4), "field copy");
                    g.fldCap = 3 * n3;
                }
                for (int c = 0; c < 3; ++c) {
                    ck(cudaMemcpyAsync(g.fld + c * n3, f[c], n3 * 4, cudaMemcpyDefault), "field copy");
                    f[c] = g.fld + c * n3;
                }
            }
            f32cuda::interp(M, D<float>(x), D<float>(y), D<float>(z), D<int>(lost), f[0], f[1], f[2], D<float>(sc), p0, p1);
            ck(cudaGetLastError(), "interpolation");
        }
        sync();
    }
    void fft(const void* g, const void* P, long long n3) override {
        first();
        f32cuda::fft(g, P, n3);
        done("FFT");
    }
    void picnic(const void* g, F32Slice rho, F32Slice G, F32Slice phi) override {
        first();
        f32cuda::picnic(g, D<float>(rho), D<float>(G), D<float>(phi));
        done("PICNIC");
    }
    void sym(const void* g, F32Slice rho, F32Slice G, float thr, F32Slice phi) override {
        first();
        f32cuda::sym(g, D<float>(rho), D<float>(G), thr, D<float>(phi));
        done("SPICNIC");
    }
    void corr(const void* g, F32Slice G, F32Slice corr, F32Slice dval, long long nc, F32Slice phi) override {
        first();
        f32cuda::corr(g, D<float>(G), D<int>(corr), D<float>(dval), nc, D<float>(phi));
        done("SPICNIC correction");
    }
    void grad(const void* g, F32Slice phi, const float* h, F32Slice ex, F32Slice ey, F32Slice ez) override {
        first();
        f32cuda::grad(g, D<float>(phi), h, D<float>(ex), D<float>(ey), D<float>(ez));
        done("field");
    }

private:
    struct Rec {
        void* dev = nullptr;
        void* host = nullptr;
        size_t bytes = 0;
        bool shared = false, managed = false, readMostly = false;
    };
    struct Gpu {
        int dev = 0;
        std::uint32_t* cnt = nullptr;  // 4 counters
        std::uint32_t* idx = nullptr;  // 3 lists of cap entries
        char* ent = nullptr;           // candidate entries: cSnap, cNp, sSnap, sElem, bEnt (cap each)
        size_t cap = 0;
        void* grp = nullptr;  // group sums
        size_t grpCap = 0;
        unsigned long long* acc = nullptr;  // deposit mesh (64-bit sums)
        size_t n3 = 0;
        float* fld = nullptr;  // copy of the mesh field (GPUs after the first)
        size_t fldCap = 0;
    };
    std::vector<int> devs_;
    std::string desc_;
    std::vector<Gpu> gpu_;
    std::vector<std::unique_ptr<Rec>> recs_;
    size_t lists_[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    size_t partSize_ = 0;  // sizeof(f32::PartF)
    std::vector<std::uint64_t> accHost_;
    void* tmp_[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};  // scratch of the mesh tables (first GPU)
    size_t tmpCap_[5] = {0, 0, 0, 0, 0};

    void* tmp(int k, size_t bytes) {
        if (tmpCap_[k] < bytes) {
            cudaFree(tmp_[k]);
            ck(cudaMalloc(&tmp_[k], bytes), "mesh scratch");
            tmpCap_[k] = bytes;
        }
        return tmp_[k];
    }
    void release(Rec& r) {
        if (r.shared) {
            cudaFreeHost(r.host);
        } else {
            cudaFree(r.dev);
            std::free(r.host);
        }
        r = Rec{};
    }
    void alloc(F32Buf& b, size_t bytes, bool shared) {
        if (b.impl && b.len >= bytes) return;
        sync();
        first();
        Rec* r = static_cast<Rec*>(b.impl);
        if (!r) {
            recs_.push_back(std::make_unique<Rec>());
            r = recs_.back().get();
        } else {
            release(*r);
        }
        const size_t n = bytes + 64;
        r->bytes = n;
        r->shared = shared;
        if (shared) {  // pinned host memory, mapped into the device (the same address with unified addressing)
            ck(cudaHostAlloc(&r->host, n, cudaHostAllocMapped | cudaHostAllocPortable), "pinned host memory");
            std::memset(r->host, 0, n);
            ck(cudaHostGetDevicePointer(&r->dev, r->host, 0), "pinned host memory");
        } else {
            r->managed = gpu_.size() > 1;  // several GPUs: one address for all of them (data moved only by copies)
            if (r->managed) {  // on the first GPU, mapped by the others (tables: tables(); particles: particles())
                ck(cudaMallocManaged(&r->dev, n), "device memory");
                cudaMemAdvise(r->dev, n, cudaMemAdviseSetPreferredLocation, devs_[0]);
                for (const Gpu& g : gpu_) cudaMemAdvise(r->dev, n, cudaMemAdviseSetAccessedBy, g.dev);
                cudaGetLastError();
            } else {
                ck(cudaMalloc(&r->dev, n), "device memory");
            }
            ck(cudaMemset(r->dev, 0, n), "device memory");
            r->host = std::calloc(n, 1);
            if (!r->host) throw EngineError("CUDA fp32: out of host memory (" + std::to_string(n) + " bytes)");
            ck(cudaDeviceSynchronize(), "device memory");
        }
        b.impl = r;
        b.host = r->host;
        b.dev = reinterpret_cast<std::uint64_t>(r->dev);
        b.len = bytes;
    }

    void sync() {
        for (const Gpu& g : gpu_) {
            cudaSetDevice(g.dev);
            ck(cudaDeviceSynchronize(), "kernel");
        }
    }
    void first() { ck(cudaSetDevice(devs_[0]), "set device"); }
    void done(const char* what) {
        ck(cudaGetLastError(), what);
        ck(cudaDeviceSynchronize(), what);
    }
    // particles of GPU k: whole reduction groups
    void range(int n, int k, int& lo, int& hi) const {
        const int ng = red_groups(n), G = static_cast<int>(gpu_.size());
        lo = std::min(n, static_cast<int>(static_cast<long long>(ng) * k / G) * kRedGroup);
        hi = std::min(n, static_cast<int>(static_cast<long long>(ng) * (k + 1) / G) * kRedGroup);
    }
    static std::uint64_t field(const void* F, size_t off) {
        std::uint64_t v;
        std::memcpy(&v, static_cast<const char*>(F) + off, 8);
        return v;
    }

    // Runs launch(lo, hi, F) on every GPU, each with its own candidate counters c0 .. c1-1, lists and
    // entries in device memory; afterwards they are appended to those of F (shared host buffers; the host
    // logic does not depend on the order of the candidates), only as many entries as there are candidates.
    template <class Launch>
    void split(int n, const void* F, Launch&& launch, int c0, int c1) {
        const int G = static_cast<int>(gpu_.size());
        std::vector<char> Fk(partSize_);
        std::memcpy(Fk.data(), F, Fk.size());
        for (int k = 0; k < G; ++k) {
            Gpu& g = gpu_[static_cast<size_t>(k)];
            ck(cudaSetDevice(g.dev), "set device");
            int lo, hi;
            range(n, k, lo, hi);
            const size_t cap = static_cast<size_t>(std::max(hi - lo, 1));
            if (g.cap < cap) {
                cudaFree(g.cnt);
                cudaFree(g.idx);
                cudaFree(g.ent);
                ck(cudaMalloc(reinterpret_cast<void**>(&g.cnt), 16), "candidate lists");
                ck(cudaMalloc(reinterpret_cast<void**>(&g.idx), 3 * cap * 4), "candidate lists");
                ck(cudaMalloc(reinterpret_cast<void**>(&g.ent), 42 * cap * 4), "candidate entries");
                g.cap = cap;
            }
            ck(cudaMemsetAsync(g.cnt, 0, 16), "candidate lists");
            const std::uint64_t pc = reinterpret_cast<std::uint64_t>(g.cnt), pi = reinterpret_cast<std::uint64_t>(g.idx),
                                pe = reinterpret_cast<std::uint64_t>(g.ent);
            const std::uint64_t ptrs[9] = {pc, pi, pi + g.cap * 4, pi + 2 * g.cap * 4, pe, pe + 12 * g.cap * 4,
                                           pe + 13 * g.cap * 4, pe + 25 * g.cap * 4, pe + 26 * g.cap * 4};
            for (int j = 0; j < 9; ++j) std::memcpy(Fk.data() + lists_[j], &ptrs[j], 8);
            std::vector<char> mine(Fk);
            launch(lo, hi, static_cast<const void*>(mine.data()));
            ck(cudaGetLastError(), "particle kernel");
        }
        sync();
        std::uint32_t* cnt = reinterpret_cast<std::uint32_t*>(field(F, lists_[0]));
        std::uint32_t* dst[3] = {reinterpret_cast<std::uint32_t*>(field(F, lists_[1])),
                                 reinterpret_cast<std::uint32_t*>(field(F, lists_[2])),
                                 reinterpret_cast<std::uint32_t*>(field(F, lists_[3]))};
        float* cSnap = reinterpret_cast<float*>(field(F, lists_[4]));
        int* cNp = reinterpret_cast<int*>(field(F, lists_[5]));
        float* sSnap = reinterpret_cast<float*>(field(F, lists_[6]));
        int* sElem = reinterpret_cast<int*>(field(F, lists_[7]));
        float* bEnt = reinterpret_cast<float*>(field(F, lists_[8]));
        for (int k = 0; k < G; ++k) {
            Gpu& g = gpu_[static_cast<size_t>(k)];
            ck(cudaSetDevice(g.dev), "set device");
            std::uint32_t c[4];
            ck(cudaMemcpy(c, g.cnt, 16, cudaMemcpyDeviceToHost), "candidate lists");
            for (int j = c0; j < c1; ++j) {
                if (c[j] == 0) continue;
                const size_t at = cnt[j], m = c[j];
                ck(cudaMemcpy(dst[j] + at, g.idx + static_cast<size_t>(j) * g.cap, m * 4, cudaMemcpyDeviceToHost), "candidate lists");
                const char* e = g.ent;
                if (j == 0) {
                    ck(cudaMemcpy(cSnap + 12 * at, e, 12 * m * 4, cudaMemcpyDeviceToHost), "candidate entries");
                    ck(cudaMemcpy(cNp + at, e + 12 * g.cap * 4, m * 4, cudaMemcpyDeviceToHost), "candidate entries");
                } else if (j == 1) {
                    ck(cudaMemcpy(sSnap + 12 * at, e + 13 * g.cap * 4, 12 * m * 4, cudaMemcpyDeviceToHost), "candidate entries");
                    ck(cudaMemcpy(sElem + at, e + 25 * g.cap * 4, m * 4, cudaMemcpyDeviceToHost), "candidate entries");
                } else {
                    ck(cudaMemcpy(bEnt + 16 * at, e + 26 * g.cap * 4, 16 * m * 4, cudaMemcpyDeviceToHost), "candidate entries");
                }
                cnt[j] += c[j];
            }
        }
    }
};

}  // namespace

std::unique_ptr<ScAccel> make_cuda_f32_accel(const std::vector<int>& devs, const std::string& desc, std::string& why) {
    return make_f32_accel(std::make_unique<CudaF32Device>(devs, desc), why);
}

}  // namespace avas
