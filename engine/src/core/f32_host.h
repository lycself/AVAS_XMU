// fp32 tier of the t-code step: the host logic shared by every fp32 device (M6c phase 4).
//
// F32Accel holds the particles in float relative to the synchronous particle between steps (the
// "authoritative" state, see ScAccel in device.h); the synchronous particle's push, the lattice
// positions relative to it and the RF factors come from the host every step; statistics are summed
// per reduction group in fixed point by the device; plane / superposeout / boundary candidates are
// completed in double here.  The device (F32Device) only allocates host-visible buffers and runs
// the kernels of core/f32_kernels.h: Metal (src/metal/), CUDA (src/cuda/) and the CPU
// (core/f32_cpu.cpp, the fp32 tier of the emu device) give the same bytes.
#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "core/device.h"

namespace avas {

// A buffer that the host reads and writes directly (unified / managed memory); dev is the address the
// kernels use (GPU address, or the host pointer).
struct F32Buf {
    void* host = nullptr;
    std::uint64_t dev = 0;
    size_t len = 0;
    void* impl = nullptr;  // device-specific handle
};
struct F32Slice {
    const F32Buf* b;
    size_t off;  // bytes
    template <class T>
    T* ptr() const { return reinterpret_cast<T*>(static_cast<char*>(b->host) + off); }
    std::uint64_t dev() const { return b->dev + off; }
};

// The kernels of core/f32_kernels.h on one device.  Every call has finished when it returns.
class F32Device {
public:
    virtual ~F32Device() = default;
    virtual bool init(std::string& why) = 0;
    virtual std::string describe() const = 0;
    virtual void ensure(F32Buf& b, size_t bytes) = 0;  // reallocates when smaller (contents not kept)
    // b holds per-particle columns of colBytes for n particles (a placement hint for several GPUs)
    virtual void particles(F32Buf& b, size_t colBytes, int n) { (void)b; (void)colBytes; (void)n; }
    // the host is about to read / write all of b (bulk conversion), or is done with it (a GPU with its own
    // memory moves the pages in one transfer instead of page by page)
    virtual void host_side(F32Buf& b) { (void)b; }
    virtual void device_side(F32Buf& b) { (void)b; }
    virtual void sizes(unsigned* o) = 0;               // sizeof the kernel structures (layout check)
    virtual void step(const void* L, const void* U, const void* F, F32Slice off, F32Slice rft, const int* sizes, int n) = 0;
    // Per reduction group of n particles (columns of length stride): ncol fixed-point sums of C, nmx
    // maxima of Mx (the first nmx0 start from 0, the others from -inf).
    virtual void group(F32Slice C, F32Slice Mx, F32Slice sums, F32Slice exps, F32Slice mx, int n, int stride, int ncol, int nmx,
                       int nmx0) = 0;
    // z-code rows, beam parameters and HESC sums (f32::zrow_particle, A = ZArgs)
    virtual void zrow(const void* A, const void* F, int n) = 0;
    virtual void bound(const void* U, const void* F, F32Slice geoms, F32Slice segStart, F32Slice segEnd, F32Slice bndZ,
                       F32Slice bndR1, F32Slice bndR2, int n) = 0;
    virtual void deposit(const void* M, F32Slice x, F32Slice y, F32Slice z, F32Slice q, F32Slice lost, F32Slice lo, F32Slice hi,
                         int n) = 0;
    virtual void interp(const void* M, F32Slice x, F32Slice y, F32Slice z, F32Slice lost, F32Slice ex, F32Slice ey, F32Slice ez,
                        F32Slice sc, int n) = 0;
    virtual void picnic(const void* g, F32Slice rho, F32Slice G, F32Slice phi) = 0;
    virtual void sym(const void* g, F32Slice rho, F32Slice G, float thr, F32Slice phi) = 0;
    virtual void corr(const void* g, F32Slice G, F32Slice corr, F32Slice dval, long long nc, F32Slice phi) = 0;
    virtual void grad(const void* g, F32Slice phi, const float* h, F32Slice ex, F32Slice ey, F32Slice ez) = 0;
};

// The accelerator of the device back end (device_emu.cpp) on an fp32 device.
std::unique_ptr<ScAccel> make_f32_accel(std::unique_ptr<F32Device> dev, std::string& why);
// The CPU device (core/f32_cpu.cpp).
std::unique_ptr<F32Device> make_f32_cpu_device();

}  // namespace avas
