// CUDA fp32 tier (M6c phase 4): the fp32 device of core/f32_host.h on NVIDIA GPUs.
//
// The host logic is the one of the Metal back end (F32Accel, core/f32_host.cpp); this file only
// allocates host-visible (managed) buffers and launches the kernels of cuda_f32_kernels.cu.  Several
// GPUs of one process split the particles by whole reduction groups (4096); every GPU gets its own
// candidate counters and lists and its own deposit mesh (64-bit integer sums), which are combined on
// the host, so the result does not depend on the number of GPUs.  The mesh solve runs on the first GPU.
// The candidate lists and entries, the group sums and the per-step tables go through device buffers of
// each GPU and explicit copies: the host then touches no page of the particle state in a normal step
// (managed memory would move those pages back and forth between host and GPU every step).
#include <cuda_runtime.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "core/errors.h"
#include "core/f32_host.h"
#include "core/reduce.h"

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
void deposit(const void* M, const float* x, const float* y, const float* z, const float* q, const int* lost, unsigned* lo,
             unsigned* hi, int p0, int p1);
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

class CudaF32Device : public F32Device {
public:
    CudaF32Device(std::vector<int> devs, std::string desc) : devs_(std::move(devs)), desc_(std::move(desc)) {}
    ~CudaF32Device() override {
        sync();
        for (void* p : live_) cudaFree(p);
        for (Gpu& g : gpu_) {
            cudaSetDevice(g.dev);
            cudaFree(g.cnt);
            cudaFree(g.idx);
            cudaFree(g.ent);
            cudaFree(g.tab);
            cudaFree(g.grp);
            cudaFree(g.lo);
            cudaFree(g.hi);
        }
    }

    bool init(std::string& why) override {
        (void)why;
        f32cuda::partf_lists(lists_);
        unsigned sz[16] = {0};
        f32cuda::sizes(sz);
        partSize_ = sz[5];
        gpu_.resize(devs_.size());
        for (size_t k = 0; k < devs_.size(); ++k) gpu_[k].dev = devs_[k];
        ck(cudaSetDevice(devs_[0]), "set device");
        return true;
    }
    std::string describe() const override { return desc_; }

    void ensure(F32Buf& b, size_t bytes) override {
        if (b.host && b.len >= bytes) return;
        sync();
        void* p = nullptr;
        ck(cudaSetDevice(devs_[0]), "set device");
        ck(cudaMallocManaged(&p, bytes + 64), "allocation of managed memory");
        // zeroed by the GPU: a host memset touches every page of fresh managed memory one by one (seconds per
        // GB); the host side moves pages in bulk when it needs them (host_side)
        ck(cudaMemset(p, 0, bytes + 64), "allocation of managed memory");
        ck(cudaDeviceSynchronize(), "allocation of managed memory");
        if (b.impl) {
            cudaFree(b.impl);
            live_.erase(std::find(live_.begin(), live_.end(), b.impl));
        }
        live_.push_back(p);
        b.impl = p;
        b.host = p;
        b.dev = reinterpret_cast<std::uint64_t>(p);
        b.len = bytes;
        // tables, maps, mesh arrays and the host's lists are read by the GPU(s) and the host: read-mostly keeps a
        // copy on each side instead of moving pages back and forth; the particle columns are set by particles()
        cudaMemAdvise(p, bytes + 64, cudaMemAdviseSetReadMostly, devs_[0]);
        cudaGetLastError();
    }
    void host_side(F32Buf& b) override {
        if (!b.impl) return;
        sync();
        cudaMemPrefetchAsync(b.impl, b.len, cudaCpuDeviceId);  // not supported everywhere (Windows): only a hint
        cudaDeviceSynchronize();
        cudaGetLastError();
    }
    void device_side(F32Buf& b) override {
        if (!b.impl || gpu_.size() > 1) return;  // several GPUs: the pages follow the particle ranges on demand
        cudaMemPrefetchAsync(b.impl, b.len, devs_[0]);
        cudaDeviceSynchronize();
        cudaGetLastError();
    }
    void particles(F32Buf& b, size_t colBytes, int n) override {
        cudaMemAdvise(b.impl, b.len + 64, cudaMemAdviseUnsetReadMostly, devs_[0]);
        if (gpu_.size() < 2 || colBytes == 0) {  // one GPU: the particle pages live there (the host reads them in bulk)
            cudaMemAdvise(b.impl, b.len + 64, cudaMemAdviseSetPreferredLocation, devs_[0]);
            cudaGetLastError();
            return;
        }
        const size_t cols = b.len / colBytes;
        for (size_t k = 0; k < gpu_.size(); ++k) {
            int lo, hi;
            range(n, static_cast<int>(k), lo, hi);
            if (hi <= lo) continue;
            for (size_t c = 0; c < cols; ++c)
                cudaMemAdvise(static_cast<char*>(b.impl) + c * colBytes + static_cast<size_t>(lo) * 4, static_cast<size_t>(hi - lo) * 4,
                              cudaMemAdviseSetPreferredLocation, gpu_[k].dev);
        }
        cudaGetLastError();  // advice is only a hint
    }

    void sizes(unsigned* o) override { f32cuda::sizes(o); }

    void step(const void* L, const void* U, const void* F, F32Slice off, F32Slice rft, const int* sz, int n) override {
        // the per-step tables (offsets of the lattice positions, RF factors) as a copy on every GPU
        const size_t nOff = off.b->len - off.off, nRf = rft.b->len - rft.off;
        split(n, F, [&](int lo, int hi, const void* Fk, Gpu& g) {
            if (g.tabCap < nOff + nRf) {
                cudaFree(g.tab);
                ck(cudaMalloc(&g.tab, nOff + nRf), "step tables");
                g.tabCap = nOff + nRf;
            }
            char* t = static_cast<char*>(g.tab);
            ck(cudaMemcpyAsync(t, off.ptr<char>(), nOff, cudaMemcpyHostToDevice), "step tables");
            ck(cudaMemcpyAsync(t + nOff, rft.ptr<char>(), nRf, cudaMemcpyHostToDevice), "step tables");
            f32cuda::step(L, U, Fk, reinterpret_cast<const float*>(t), reinterpret_cast<const float*>(t + nOff), sz, lo, hi);
        }, 0, 2);
    }
    void bound(const void* U, const void* F, F32Slice geoms, F32Slice segStart, F32Slice segEnd, F32Slice bndZ, F32Slice bndR1,
               F32Slice bndR2, int n) override {
        split(n, F, [&](int lo, int hi, const void* Fk, Gpu&) {
            f32cuda::bound(U, Fk, geoms.ptr<void>(), segStart.ptr<float>(), segEnd.ptr<float>(), bndZ.ptr<float>(), bndR1.ptr<int>(),
                           bndR2.ptr<int>(), lo, hi);
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
        // into device buffers (layout of the host buffers), then copied: the host reads its own pages
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
            f32cuda::group(C.ptr<float>(), Mx.ptr<float>(), reinterpret_cast<long long*>(d), reinterpret_cast<int*>(d + bs),
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
        for (int k = 0; k < G; ++k) {
            Gpu& g = gpu_[static_cast<size_t>(k)];
            ck(cudaSetDevice(g.dev), "set device");
            if (g.n3 < n3) {
                cudaFree(g.lo);
                cudaFree(g.hi);
                ck(cudaMalloc(reinterpret_cast<void**>(&g.lo), n3 * 4), "deposit mesh");
                ck(cudaMalloc(reinterpret_cast<void**>(&g.hi), n3 * 4), "deposit mesh");
                g.n3 = n3;
            }
            ck(cudaMemsetAsync(g.lo, 0, n3 * 4), "deposit mesh");
            ck(cudaMemsetAsync(g.hi, 0, n3 * 4), "deposit mesh");
            int p0, p1;
            range(n, k, p0, p1);
            f32cuda::deposit(M, x.ptr<float>(), y.ptr<float>(), z.ptr<float>(), q.ptr<float>(), lost.ptr<int>(), g.lo, g.hi, p0, p1);
            ck(cudaGetLastError(), "deposit");
        }
        std::uint32_t* L = lo.ptr<std::uint32_t>();
        std::uint32_t* H = hi.ptr<std::uint32_t>();
        std::vector<std::uint32_t> l(n3), h(n3);
        for (int k = 0; k < G; ++k) {
            Gpu& g = gpu_[static_cast<size_t>(k)];
            ck(cudaSetDevice(g.dev), "set device");
            ck(cudaMemcpy(l.data(), g.lo, n3 * 4, cudaMemcpyDeviceToHost), "deposit download");
            ck(cudaMemcpy(h.data(), g.hi, n3 * 4, cudaMemcpyDeviceToHost), "deposit download");
            for (size_t e = 0; e < n3; ++e) {  // 64-bit sums modulo 2^64: order independent
                const std::uint64_t v = ((static_cast<std::uint64_t>(H[e]) << 32) | L[e]) +
                                        ((static_cast<std::uint64_t>(h[e]) << 32) | l[e]);
                L[e] = static_cast<std::uint32_t>(v);
                H[e] = static_cast<std::uint32_t>(v >> 32);
            }
        }
    }
    void interp(const void* M, F32Slice x, F32Slice y, F32Slice z, F32Slice lost, F32Slice ex, F32Slice ey, F32Slice ez, F32Slice sc,
                int n) override {
        const int G = static_cast<int>(gpu_.size());
        for (int k = 0; k < G; ++k) {
            ck(cudaSetDevice(gpu_[static_cast<size_t>(k)].dev), "set device");
            int p0, p1;
            range(n, k, p0, p1);
            f32cuda::interp(M, x.ptr<float>(), y.ptr<float>(), z.ptr<float>(), lost.ptr<int>(), ex.ptr<float>(), ey.ptr<float>(),
                            ez.ptr<float>(), sc.ptr<float>(), p0, p1);
            ck(cudaGetLastError(), "interpolation");
        }
        sync();
    }
    void picnic(const void* g, F32Slice rho, F32Slice G, F32Slice phi) override {
        first();
        f32cuda::picnic(g, rho.ptr<float>(), G.ptr<float>(), phi.ptr<float>());
        done("PICNIC");
    }
    void sym(const void* g, F32Slice rho, F32Slice G, float thr, F32Slice phi) override {
        first();
        f32cuda::sym(g, rho.ptr<float>(), G.ptr<float>(), thr, phi.ptr<float>());
        done("SPICNIC");
    }
    void corr(const void* g, F32Slice G, F32Slice corr, F32Slice dval, long long nc, F32Slice phi) override {
        first();
        f32cuda::corr(g, G.ptr<float>(), corr.ptr<int>(), dval.ptr<float>(), nc, phi.ptr<float>());
        done("SPICNIC correction");
    }
    void grad(const void* g, F32Slice phi, const float* h, F32Slice ex, F32Slice ey, F32Slice ez) override {
        first();
        f32cuda::grad(g, phi.ptr<float>(), h, ex.ptr<float>(), ey.ptr<float>(), ez.ptr<float>());
        done("field");
    }

private:
    struct Gpu {
        int dev = 0;
        std::uint32_t* cnt = nullptr;  // 4 counters
        std::uint32_t* idx = nullptr;  // 3 lists of cap entries
        char* ent = nullptr;           // candidate entries: cSnap, cNp, sSnap, sElem (cap each)
        size_t cap = 0;
        void* tab = nullptr;  // per-step tables
        size_t tabCap = 0;
        void* grp = nullptr;  // group sums
        size_t grpCap = 0;
        std::uint32_t* lo = nullptr;  // deposit mesh
        std::uint32_t* hi = nullptr;
        size_t n3 = 0;
    };
    std::vector<int> devs_;
    std::string desc_;
    std::vector<Gpu> gpu_;
    std::vector<void*> live_;
    size_t lists_[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    size_t partSize_ = 0;  // sizeof(f32::PartF)

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

    // Runs launch(lo, hi, F, gpu) on every GPU, each with its own candidate counters c0 .. c1-1, lists and
    // entries in device memory; afterwards they are appended to those of F (host buffers; the host logic
    // does not depend on the order of the candidates), only as many entries as there are candidates.
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
                ck(cudaMalloc(reinterpret_cast<void**>(&g.ent), 26 * cap * 4), "candidate entries");
                g.cap = cap;
            }
            ck(cudaMemsetAsync(g.cnt, 0, 16), "candidate lists");
            const std::uint64_t pc = reinterpret_cast<std::uint64_t>(g.cnt), pi = reinterpret_cast<std::uint64_t>(g.idx),
                                pe = reinterpret_cast<std::uint64_t>(g.ent);
            const std::uint64_t ptrs[8] = {pc, pi, pi + g.cap * 4, pi + 2 * g.cap * 4,
                                           pe, pe + 12 * g.cap * 4, pe + 13 * g.cap * 4, pe + 25 * g.cap * 4};
            for (int j = 0; j < 8; ++j) std::memcpy(Fk.data() + lists_[j], &ptrs[j], 8);
            std::vector<char> mine(Fk);
            launch(lo, hi, static_cast<const void*>(mine.data()), g);
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
