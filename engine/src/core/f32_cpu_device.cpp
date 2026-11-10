// The CPU device of the fp32 tier (the emu device with precision fp32): host memory and the loops
// of f32_cpu.cpp.
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "core/errors.h"
#include "core/f32_host.h"

namespace avas {

namespace f32cpu {  // f32_cpu.cpp (compiled in the float world)
void sizes(unsigned* o);
void step(const void* L, const void* U, const void* F, const float* off, const float* rft, const int* sizes, int n);
void bound(const void* U, const void* F, const void* geoms, const float* segStart, const float* segEnd, const float* bndZ,
           const int* bndR1, const int* bndR2, int n);
void deposit(const void* M, const float* x, const float* y, const float* z, const float* q, const int* lost, unsigned* lo,
             unsigned* hi, int n);
void interp(const void* M, const float* x, const float* y, const float* z, const int* lost, const float* ex, const float* ey,
            const float* ez, float* sc, int n);
void picnic(const void* g, const float* rho, const float* G, float* phi);
void sym(const void* g, const float* rho, const float* G, float thr, float* phi);
void corr(const void* g, const float* G, const int* corr, const float* dval, long long nc, float* phi);
void grad(const void* g, const float* phi, const float* h, float* ex, float* ey, float* ez);
void group(const float* C, const float* Mx, long long* sums, int* exps, float* mx, int n, int stride, int ncol, int nmx, int nmx0);
void zrow(const void* A, const void* F, int n);
void fft(const void* g, const void* P, long long n3);
}  // namespace f32cpu

namespace {

// The fp32 tier of the emu device: plain host memory, the loops above.
class CpuF32Device : public F32Device {
public:
    ~CpuF32Device() override {
        for (F32Buf* b : bufs_) (void)b;
        for (void* p : live_) std::free(p);
    }
    bool init(std::string&) override { return true; }
    std::string describe() const override {
        return "emu fp32 (the fp32 kernels of the GPU back ends on the CPU, core/f32_kernels.h)";
    }
    void ensure(F32Buf& b, size_t bytes) override {
        if (b.host && b.len >= bytes) return;
        void* p = std::calloc(bytes + 64, 1);
        if (!p) throw EngineError("emu fp32: out of memory (" + std::to_string(bytes) + " bytes)");
        if (b.impl) {
            std::free(b.impl);
            live_.erase(std::find(live_.begin(), live_.end(), b.impl));
        }
        live_.push_back(p);
        b.impl = p;
        b.host = p;
        b.dev = reinterpret_cast<std::uint64_t>(p);
        b.len = bytes;
    }
    void sizes(unsigned* o) override { f32cpu::sizes(o); }
    void step(const void* L, const void* U, const void* F, F32Slice off, F32Slice rft, const int* sz, int n) override {
        f32cpu::step(L, U, F, off.ptr<float>(), rft.ptr<float>(), sz, n);
    }
    void group(F32Slice C, F32Slice Mx, F32Slice sums, F32Slice exps, F32Slice mx, int n, int stride, int ncol, int nmx,
               int nmx0) override {
        f32cpu::group(C.ptr<float>(), Mx.ptr<float>(), sums.ptr<long long>(), exps.ptr<int>(), mx.ptr<float>(), n, stride, ncol, nmx,
                      nmx0);
    }
    void fft(const void* g, const void* P, long long n3) override { f32cpu::fft(g, P, n3); }
    void zrow(const void* A, const void* F, int n) override {
        f32cpu::zrow(A, F, n);
    }
    void bound(const void* U, const void* F, F32Slice geoms, F32Slice segStart, F32Slice segEnd, F32Slice bndZ, F32Slice bndR1,
               F32Slice bndR2, int n) override {
        f32cpu::bound(U, F, geoms.ptr<void>(), segStart.ptr<float>(), segEnd.ptr<float>(), bndZ.ptr<float>(), bndR1.ptr<int>(),
                      bndR2.ptr<int>(), n);
    }
    void deposit(const void* M, F32Slice x, F32Slice y, F32Slice z, F32Slice q, F32Slice lost, F32Slice lo, F32Slice hi,
                 int n) override {
        f32cpu::deposit(M, x.ptr<float>(), y.ptr<float>(), z.ptr<float>(), q.ptr<float>(), lost.ptr<int>(), lo.ptr<unsigned>(),
                        hi.ptr<unsigned>(), n);
    }
    void interp(const void* M, F32Slice x, F32Slice y, F32Slice z, F32Slice lost, F32Slice ex, F32Slice ey, F32Slice ez, F32Slice sc,
                int n) override {
        f32cpu::interp(M, x.ptr<float>(), y.ptr<float>(), z.ptr<float>(), lost.ptr<int>(), ex.ptr<float>(), ey.ptr<float>(),
                       ez.ptr<float>(), sc.ptr<float>(), n);
    }
    void picnic(const void* g, F32Slice rho, F32Slice G, F32Slice phi) override {
        f32cpu::picnic(g, rho.ptr<float>(), G.ptr<float>(), phi.ptr<float>());
    }
    void sym(const void* g, F32Slice rho, F32Slice G, float thr, F32Slice phi) override {
        f32cpu::sym(g, rho.ptr<float>(), G.ptr<float>(), thr, phi.ptr<float>());
    }
    void corr(const void* g, F32Slice G, F32Slice cr, F32Slice dval, long long nc, F32Slice phi) override {
        f32cpu::corr(g, G.ptr<float>(), cr.ptr<int>(), dval.ptr<float>(), nc, phi.ptr<float>());
    }
    void grad(const void* g, F32Slice phi, const float* h, F32Slice ex, F32Slice ey, F32Slice ez) override {
        f32cpu::grad(g, phi.ptr<float>(), h, ex.ptr<float>(), ey.ptr<float>(), ez.ptr<float>());
    }

private:
    std::vector<F32Buf*> bufs_;
    std::vector<void*> live_;
};

}  // namespace

std::unique_ptr<F32Device> make_f32_cpu_device() { return std::make_unique<CpuF32Device>(); }

}  // namespace avas
