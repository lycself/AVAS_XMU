// Metal back end (macOS, AVAS_WITH_METAL): the fp32 device of core/f32_host.h on the GPU.
//
// The Metal source is the shared code of core/hd.h, mesh_kernels.h, physics.h, kernels.h,
// sc_kernels.h and f32_kernels.h plus src/metal/kernels.metal, embedded at build time
// (avas_msl_source.inc) and compiled at run time (Command Line Tools have no offline Metal compiler):
// `double` is defined as float, fast math is off and FP contraction is disabled, so the kernels
// compute the same floats as the CPU fp32 tier (core/f32_cpu.cpp).  Buffers are shared (unified
// memory): the host logic reads and writes them directly; structures hold GPU addresses.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "core/errors.h"
#include "core/f32_host.h"

#include "avas_msl_source.inc"  // kAvasMslSource

namespace avas {

namespace {

class MetalF32Device : public F32Device {
public:
    ~MetalF32Device() override {
        for (auto& kv : live_) CFRelease(kv);
    }

    bool init(std::string& why) override {
        @autoreleasepool {
            dev_ = MTLCreateSystemDefaultDevice();
            if (!dev_) {
                why = "no Metal device";
                return false;
            }
            if (![dev_ supportsFamily:MTLGPUFamilyMetal3]) {
                why = std::string("the GPU ") + [[dev_ name] UTF8String] + " does not support Metal 3 (GPU addresses)";
                return false;
            }
            MTLCompileOptions* opt = [MTLCompileOptions new];
            if (@available(macOS 15.0, *)) {
                opt.mathMode = MTLMathModeSafe;
                opt.mathFloatingPointFunctions = MTLMathFloatingPointFunctionsPrecise;
            } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                opt.fastMathEnabled = NO;
#pragma clang diagnostic pop
            }
            NSError* err = nil;
            id<MTLLibrary> lib = [dev_ newLibraryWithSource:[NSString stringWithUTF8String:kAvasMslSource] options:opt error:&err];
            if (!lib) {
                why = std::string("Metal compilation failed: ") + [[err localizedDescription] UTF8String];
                return false;
            }
            auto pipe = [&](const char* name) -> id<MTLComputePipelineState> {
                id<MTLFunction> f = [lib newFunctionWithName:[NSString stringWithUTF8String:name]];
                NSError* e2 = nil;
                id<MTLComputePipelineState> ps = f ? [dev_ newComputePipelineStateWithFunction:f error:&e2] : nil;
                if (!ps) why = std::string("Metal pipeline ") + name + " failed";
                return ps;
            };
            sizes_ = pipe("k_sizes");
            dep_ = pipe("k_deposit");
            interp_ = pipe("k_interp");
            picnic_ = pipe("k_picnic");
            sym_ = pipe("k_sym");
            corr_ = pipe("k_corr");
            grad_ = pipe("k_grad");
            step_ = pipe("k_step");
            group_ = pipe("k_group");
            bound_ = pipe("k_bound");
            zrow_ = pipe("k_zrow");
            fft_ = pipe("k_fft");
            fftLine_ = pipe("k_fft_line");
            if (!sizes_ || !dep_ || !interp_ || !picnic_ || !sym_ || !corr_ || !grad_ || !step_ || !group_ || !bound_ || !zrow_ || !fft_ ||
                !fftLine_)
                return false;
            queue_ = [dev_ newCommandQueue];
            name_ = [[dev_ name] UTF8String];
        }
        sizes(sz_);  // the structures are passed by value with their Metal sizes
        return true;
    }

    std::string describe() const override {
        return "Metal, " + name_ + ": t-code push, z-code rows, statistics, boundary.txt, lpboundary / longlimits and the "
               "space-charge mesh work (PICNIC, SPICNIC, FFT) on the GPU in fp32 (secondary particles on the CPU)";
    }

    void ensure(F32Buf& b, size_t bytes) override {
        if (b.impl && b.len >= bytes) return;
        @autoreleasepool {
            id<MTLBuffer> m = [dev_ newBufferWithLength:bytes options:MTLResourceStorageModeShared];
            if (!m) throw EngineError("Metal: out of memory (" + std::to_string(bytes) + " bytes)");
            if (b.impl) {
                live_.erase(std::find(live_.begin(), live_.end(), b.impl));
                CFRelease(b.impl);
            }
            b.impl = (__bridge_retained void*)m;
            live_.push_back(b.impl);
            b.host = [m contents];
            b.dev = [m gpuAddress];
            b.len = bytes;
        }
    }

    void sizes(unsigned* o) override {
        F32Buf b;
        ensure(b, 64);
        run(sizes_, 1, [&](id<MTLComputeCommandEncoder> enc) { set(enc, F32Slice{&b, 0}, 0); });
        std::memcpy(o, b.host, 12 * 4);
        live_.erase(std::find(live_.begin(), live_.end(), b.impl));
        CFRelease(b.impl);
    }

    void step(const void* L, const void* U, const void* F, F32Slice off, F32Slice rft, const int* sz, int n) override {
        run(step_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:L length:sz_[2] atIndex:0];
            [enc setBytes:U length:sz_[6] atIndex:1];
            [enc setBytes:F length:sz_[5] atIndex:2];
            set(enc, off, 3);
            set(enc, rft, 4);
            [enc setBytes:sz length:16 atIndex:5];
        });
    }

    void batch_begin() override {
        if (batchDepth_++ > 0) return;  // nested: the outer batch submits
        if (benc_) return;
        bcb_ = [queue_ commandBuffer];
        benc_ = [bcb_ computeCommandEncoder];
    }
    void batch_end() override {
        if (batchDepth_ > 0 && --batchDepth_ > 0) return;
        if (!benc_) return;
        [benc_ endEncoding];
        [bcb_ commit];
        [bcb_ waitUntilCompleted];
        const bool failed = [bcb_ status] == MTLCommandBufferStatusError;
        const std::string why = failed ? std::string([[[bcb_ error] localizedDescription] UTF8String]) : std::string();
        benc_ = nil;
        bcb_ = nil;
        if (failed) throw EngineError("Metal command failed: " + why);
    }

    void fft(const void* g, const void* P, long long n3) override {
        run(fft_, static_cast<size_t>(n3), [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:g length:sz_[1] atIndex:0];
            [enc setBytes:P length:sz_[11] atIndex:1];
        });
    }
    bool fft_lines(const void* g, const void* P, int pass) override {
        if (pass != 1 && pass != 2 && pass != 3 && pass != 5 && pass != 6 && pass != 7) return false;
        const int* d = static_cast<const int*>(g);  // SpGrid: nx, ny, nz, ...
        const bool z = pass == 3 || pass == 5, x = pass == 1 || pass == 6;
        const int N = z ? d[2] : (x ? d[0] : d[1]);
        const long long lines = z ? static_cast<long long>(d[0]) * d[1] : (x ? static_cast<long long>(d[1]) * d[2] : static_cast<long long>(d[0]) * d[2]);
        const int maxT = static_cast<int>(std::min<NSUInteger>(1024, [fftLine_ maxTotalThreadsPerThreadgroup]));
        if (N < 1 || N > maxT || lines > 0x7fffffff) return false;
        const int G = std::max(1, std::min(256, maxT) / N);  // lines per threadgroup
        const size_t shBytes = ((static_cast<size_t>(2 * G * N + 3 * N + 2) * 4) + 15) / 16 * 16;
        if (shBytes > [dev_ maxThreadgroupMemoryLength]) return false;
        const int lg[2] = {G, static_cast<int>(lines)};
        run_groups(fftLine_, static_cast<size_t>((lines + G - 1) / G), static_cast<size_t>(G) * N, [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:g length:sz_[1] atIndex:0];
            [enc setBytes:P length:sz_[11] atIndex:1];
            [enc setBytes:lg length:8 atIndex:2];
            [enc setThreadgroupMemoryLength:shBytes atIndex:0];
        });
        return true;
    }
    void zrow(const void* A, const void* F, int n) override {
        run(zrow_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:A length:sz_[10] atIndex:0];
            [enc setBytes:F length:sz_[5] atIndex:1];
        });
    }

    void group(F32Slice C, F32Slice Mx, F32Slice sums, F32Slice exps, F32Slice mx, int n, int stride, int ncol, int nmx,
               int nmx0) override {
        const int ng = (n + 4095) / 4096;
        const int cols[4] = {ncol, nmx, nmx0, 0};
        run_groups(group_, static_cast<size_t>(ng), 256, [&](id<MTLComputeCommandEncoder> enc) {
            set(enc, C, 0);
            set(enc, Mx, 1);
            set(enc, sums, 2);
            set(enc, exps, 3);
            set(enc, mx, 4);
            [enc setBytes:&n length:4 atIndex:5];
            [enc setBytes:&stride length:4 atIndex:6];
            [enc setBytes:cols length:16 atIndex:7];
        });
    }

    void bound(const void* U, const void* F, F32Slice geoms, F32Slice segStart, F32Slice segEnd, F32Slice bndZ, F32Slice bndR1,
               F32Slice bndR2, int n) override {
        run(bound_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:U length:sz_[9] atIndex:0];
            [enc setBytes:F length:sz_[5] atIndex:1];
            set(enc, geoms, 2);
            set(enc, segStart, 3);
            set(enc, segEnd, 4);
            set(enc, bndZ, 5);
            set(enc, bndR1, 6);
            set(enc, bndR2, 7);
        });
    }

    void deposit(const void* M, F32Slice x, F32Slice y, F32Slice z, F32Slice q, F32Slice lost, F32Slice lo, F32Slice hi,
                 int n) override {
        const std::uint32_t nn = static_cast<std::uint32_t>(n);
        run(dep_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:M length:sz_[0] atIndex:0];
            set(enc, x, 1);
            set(enc, y, 2);
            set(enc, z, 3);
            set(enc, q, 4);
            set(enc, lost, 5);
            set(enc, lo, 6);
            set(enc, hi, 7);
            [enc setBytes:&nn length:4 atIndex:8];
        });
    }

    void interp(const void* M, F32Slice x, F32Slice y, F32Slice z, F32Slice lost, F32Slice ex, F32Slice ey, F32Slice ez, F32Slice sc,
                int n) override {
        const std::uint32_t nn = static_cast<std::uint32_t>(n);
        run(interp_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:M length:sz_[0] atIndex:0];
            set(enc, x, 1);
            set(enc, y, 2);
            set(enc, z, 3);
            set(enc, lost, 4);
            set(enc, ex, 5);
            set(enc, ey, 6);
            set(enc, ez, 7);
            set(enc, sc, 8);
            [enc setBytes:&nn length:4 atIndex:9];
        });
    }

    void picnic(const void* g, F32Slice rho, F32Slice G, F32Slice phi) override {
        const int* d = static_cast<const int*>(g);
        run(picnic_, static_cast<size_t>(d[0]) * d[1] * d[2], [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:g length:sz_[1] atIndex:0];
            set(enc, rho, 1);
            set(enc, G, 2);
            set(enc, phi, 3);
        });
    }

    void sym(const void* g, F32Slice rho, F32Slice G, float thr, F32Slice phi) override {
        const int* d = static_cast<const int*>(g);
        const size_t nt = static_cast<size_t>(d[0] - d[0] / 2) * (d[1] - d[1] / 2) * (d[2] - d[2] / 2);
        run(sym_, nt, [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:g length:sz_[1] atIndex:0];
            set(enc, rho, 1);
            set(enc, G, 2);
            [enc setBytes:&thr length:4 atIndex:3];
            set(enc, phi, 4);
        });
    }

    void corr(const void* g, F32Slice G, F32Slice cr, F32Slice dval, long long nc, F32Slice phi) override {
        const int* d = static_cast<const int*>(g);
        const long nl = static_cast<long>(nc);
        run(corr_, static_cast<size_t>(d[0]) * d[1] * d[2], [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:g length:sz_[1] atIndex:0];
            set(enc, G, 1);
            set(enc, cr, 2);
            set(enc, dval, 3);
            [enc setBytes:&nl length:8 atIndex:4];
            set(enc, phi, 5);
        });
    }

    void grad(const void* g, F32Slice phi, const float* h, F32Slice ex, F32Slice ey, F32Slice ez) override {
        const int* d = static_cast<const int*>(g);
        run(grad_, static_cast<size_t>(d[0]) * d[1] * d[2], [&](id<MTLComputeCommandEncoder> enc) {
            [enc setBytes:g length:sz_[1] atIndex:0];
            set(enc, phi, 1);
            [enc setBytes:h length:12 atIndex:2];
            set(enc, ex, 3);
            set(enc, ey, 4);
            set(enc, ez, 5);
        });
    }

private:
    unsigned sz_[16] = {0};  // sizes of the kernel structures (k_sizes)
    id<MTLDevice> dev_ = nil;
    id<MTLCommandQueue> queue_ = nil;
    id<MTLComputePipelineState> sizes_ = nil, dep_ = nil, interp_ = nil, picnic_ = nil, sym_ = nil, corr_ = nil, grad_ = nil,
                                step_ = nil, group_ = nil, bound_ = nil, zrow_ = nil, fft_ = nil,
                                fftLine_ = nil;
    std::vector<void*> live_;  // every buffer: made resident for each dispatch (structures hold GPU addresses)
    std::string name_;

    static void set(id<MTLComputeCommandEncoder> enc, F32Slice s, NSUInteger idx) {
        [enc setBuffer:(__bridge id<MTLBuffer>)s.b->impl offset:s.off atIndex:idx];
    }
    id<MTLCommandBuffer> bcb_ = nil;  // open batch (batch_begin .. batch_end)
    id<MTLComputeCommandEncoder> benc_ = nil;
    int batchDepth_ = 0;

    template <class Setup>
    void run(id<MTLComputePipelineState> ps, size_t threads, Setup&& setup) {
        run_groups(ps, 0, threads, setup);
    }

    // groups == 0: `threads` threads in groups of up to 256; otherwise `groups` threadgroups of `threads`
    template <class Setup>
    void run_groups(id<MTLComputePipelineState> ps, size_t groups, size_t threads, Setup&& setup) {
        if (threads == 0) return;
        auto dispatch = [&](id<MTLComputeCommandEncoder> enc) {
            if (groups) {
                [enc dispatchThreadgroups:MTLSizeMake(groups, 1, 1) threadsPerThreadgroup:MTLSizeMake(threads, 1, 1)];
            } else {
                const NSUInteger tg = std::min<NSUInteger>(256, [ps maxTotalThreadsPerThreadgroup]);
                [enc dispatchThreads:MTLSizeMake(threads, 1, 1) threadsPerThreadgroup:MTLSizeMake(tg, 1, 1)];
            }
        };
        if (benc_) {  // in a batch: queued in the open encoder, ordered by a buffer barrier
            [benc_ setComputePipelineState:ps];
            setup(benc_);
            for (void* r : live_) [benc_ useResource:(__bridge id<MTLBuffer>)r usage:MTLResourceUsageRead | MTLResourceUsageWrite];
            dispatch(benc_);
            [benc_ memoryBarrierWithScope:MTLBarrierScopeBuffers];
            return;
        }
        @autoreleasepool {
            id<MTLCommandBuffer> cb = [queue_ commandBuffer];
            id<MTLComputeCommandEncoder> enc = [cb computeCommandEncoder];
            [enc setComputePipelineState:ps];
            setup(enc);
            for (void* r : live_) [enc useResource:(__bridge id<MTLBuffer>)r usage:MTLResourceUsageRead | MTLResourceUsageWrite];
            dispatch(enc);
            [enc endEncoding];
            [cb commit];
            [cb waitUntilCompleted];
            if ([cb status] == MTLCommandBufferStatusError)
                throw EngineError(std::string("Metal command failed: ") + [[[cb error] localizedDescription] UTF8String]);
        }
    }
};

}  // namespace

bool metal_compiled() { return true; }

bool metal_probe(std::string& name, std::string& why) {
    @autoreleasepool {
        id<MTLDevice> dev = MTLCreateSystemDefaultDevice();
        if (!dev) {
            why = "no Metal device";
            return false;
        }
        name = [[dev name] UTF8String];
        if (![dev supportsFamily:MTLGPUFamilyMetal3]) {
            why = "the GPU " + name + " does not support Metal 3 (GPU addresses)";
            return false;
        }
        return true;
    }
}

std::unique_ptr<ScAccel> make_metal_accel(std::string& why) {
    return make_f32_accel(std::make_unique<MetalF32Device>(), why);
}

}  // namespace avas
