// Metal back end (macOS, AVAS_WITH_METAL), M6c phase 1: the space-charge mesh work on the GPU.
//
// The particles are tracked by the device back end of the CPU (device_emu.cpp, fp64); this file
// runs the CIC deposit, the PICNIC / SPICNIC sums with the field differences and the interpolation
// on the GPU in fp32.  The Metal source is the shared code of core/hd.h, core/mesh_kernels.h and
// core/sc_kernels.h plus src/metal/kernels.metal, embedded at build time (avas_msl_source.inc) and
// compiled at run time (Command Line Tools have no offline Metal compiler): `double` is defined as
// float, fast math is off and FP contraction is disabled, so the results do not depend on the
// scheduling -- the deposit is a fixed-point integer sum -- and a rerun is bit for bit the same.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "core/device.h"
#include "core/errors.h"
#include "core/sc_kernels.h"

#include "avas_msl_source.inc"  // kAvasMslSource

namespace avas {

namespace {

// kern::MeshV as compiled for Metal (double -> float, pointers -> GPU addresses); checked against
// sizeof on the GPU at start-up.
struct MeshF {
    int nx, ny, nz;
    float dx, dy, dz, vol, gamma, igamma;
    float c0, c2, c4;
    int lpRel, lpc, compat;
    std::uint64_t xs, ys, zs;
    float Q;
    int rotMode;
    float rs, rc;
    std::int64_t fixScaleExp;
};

class MetalAccel : public ScAccel {
public:
    bool init(std::string& why) {
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
            NSString* src = [NSString stringWithUTF8String:kAvasMslSource];
            id<MTLLibrary> lib = [dev_ newLibraryWithSource:src options:opt error:&err];
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
            if (!sizes_ || !dep_ || !interp_ || !picnic_ || !sym_ || !corr_ || !grad_) return false;
            queue_ = [dev_ newCommandQueue];
            // layout check of the structures shared with the MSL code
            id<MTLBuffer> b = buf(sz_, 16);
            run(sizes_, 1, [&](id<MTLComputeCommandEncoder> enc) { [enc setBuffer:b offset:0 atIndex:0]; });
            const std::uint32_t* s = static_cast<const std::uint32_t*>([b contents]);
            if (s[0] != sizeof(MeshF) || s[1] != sizeof(sck::SpGrid)) {
                why = "Metal structure layout mismatch (MeshV " + std::to_string(s[0]) + " / " + std::to_string(sizeof(MeshF)) + ")";
                return false;
            }
            name_ = [[dev_ name] UTF8String];
        }
        return true;
    }

    std::string describe() const override {
        return "Metal, " + name_ + ": space-charge mesh work on the GPU in fp32, particles on the CPU in fp64 (M6c phase 1)";
    }

    void deposit(const kern::MeshV& M, int n, const double* x, const double* y, const double* z, const double* q,
                 const int* lost, std::vector<long long>& acc) override {
        if (n <= 0) return;
        @autoreleasepool {
            const size_t n3 = static_cast<size_t>(M.nx) * M.ny * M.nz;
            float* fx = fl(bx_, n, x);
            (void)fx;
            fl(by_, n, y);
            fl(bz_, n, z);
            fl(bq_, n, q);
            std::memcpy([buf(blost_, n * 4) contents], lost, static_cast<size_t>(n) * 4);
            id<MTLBuffer> lo = buf(blo_, n3 * 4), hi = buf(bhi_, n3 * 4);
            std::memset([lo contents], 0, n3 * 4);
            std::memset([hi contents], 0, n3 * 4);
            MeshF F = mesh(M);
            const std::uint32_t nn = static_cast<std::uint32_t>(n);
            run(dep_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
                [enc setBytes:&F length:sizeof(F) atIndex:0];
                [enc setBuffer:bx_ offset:0 atIndex:1];
                [enc setBuffer:by_ offset:0 atIndex:2];
                [enc setBuffer:bz_ offset:0 atIndex:3];
                [enc setBuffer:bq_ offset:0 atIndex:4];
                [enc setBuffer:blost_ offset:0 atIndex:5];
                [enc setBuffer:lo offset:0 atIndex:6];
                [enc setBuffer:hi offset:0 atIndex:7];
                [enc setBytes:&nn length:4 atIndex:8];
                [enc useResource:bnodes_ usage:MTLResourceUsageRead];
            });
            const std::uint32_t* l = static_cast<const std::uint32_t*>([lo contents]);
            const std::uint32_t* h = static_cast<const std::uint32_t*>([hi contents]);
            for (size_t e = 0; e < n3; ++e) {
                const std::uint64_t v = (static_cast<std::uint64_t>(h[e]) << 32) | l[e];
                acc[e] = static_cast<long long>(static_cast<std::uint64_t>(acc[e]) + v);
            }
        }
    }

    bool solve(const ScSolveIn& in, const double* G, const int* corr, const double* dval, long long nc,
               std::vector<double>& ex, std::vector<double>& ey, std::vector<double>& ez) override {
        if (in.method != 1 && in.method != 2) return false;
        @autoreleasepool {
            const sck::SpGrid g{in.nx, in.ny, in.nz, in.mx, in.my, in.mz};
            const size_t n3 = static_cast<size_t>(g.n3());
            const size_t nG = static_cast<size_t>(2 * in.nx + 1) * (2 * in.ny + 1) * (2 * in.nz + 1);
            fl(brho_, n3, in.rho);
            fl(bG_, nG, G);
            id<MTLBuffer> phi = buf(bphi_, n3 * 4);
            std::memset([phi contents], 0, n3 * 4);
            if (in.method == 2) {
                run(picnic_, n3, [&](id<MTLComputeCommandEncoder> enc) {
                    [enc setBytes:&g length:sizeof(g) atIndex:0];
                    [enc setBuffer:brho_ offset:0 atIndex:1];
                    [enc setBuffer:bG_ offset:0 atIndex:2];
                    [enc setBuffer:phi offset:0 atIndex:3];
                });
            } else {
                const float thr = static_cast<float>(in.thr);
                run(sym_, static_cast<size_t>(g.n_targets()), [&](id<MTLComputeCommandEncoder> enc) {
                    [enc setBytes:&g length:sizeof(g) atIndex:0];
                    [enc setBuffer:brho_ offset:0 atIndex:1];
                    [enc setBuffer:bG_ offset:0 atIndex:2];
                    [enc setBytes:&thr length:4 atIndex:3];
                    [enc setBuffer:phi offset:0 atIndex:4];
                });
                if (nc > 0) {
                    std::memcpy([buf(bcorr_, static_cast<size_t>(3 * nc) * 4) contents], corr, static_cast<size_t>(3 * nc) * 4);
                    fl(bdval_, static_cast<size_t>(nc), dval);
                    const long nl = static_cast<long>(nc);
                    run(corr_, n3, [&](id<MTLComputeCommandEncoder> enc) {
                        [enc setBytes:&g length:sizeof(g) atIndex:0];
                        [enc setBuffer:bG_ offset:0 atIndex:1];
                        [enc setBuffer:bcorr_ offset:0 atIndex:2];
                        [enc setBuffer:bdval_ offset:0 atIndex:3];
                        [enc setBytes:&nl length:8 atIndex:4];
                        [enc setBuffer:phi offset:0 atIndex:5];
                    });
                }
            }
            const float h[3] = {static_cast<float>(1.0 / (in.dx + in.dx)), static_cast<float>(1.0 / (in.dy + in.dy)),
                                static_cast<float>(1.0 / (in.dz + in.dz))};
            id<MTLBuffer> fe = buf(bfield_, 3 * n3 * 4);
            run(grad_, n3, [&](id<MTLComputeCommandEncoder> enc) {
                [enc setBytes:&g length:sizeof(g) atIndex:0];
                [enc setBuffer:phi offset:0 atIndex:1];
                [enc setBytes:h length:12 atIndex:2];
                [enc setBuffer:fe offset:0 atIndex:3];
                [enc setBuffer:fe offset:n3 * 4 atIndex:4];
                [enc setBuffer:fe offset:2 * n3 * 4 atIndex:5];
            });
            const float* f = static_cast<const float*>([fe contents]);
            ex.assign(f, f + n3);
            ey.assign(f + n3, f + 2 * n3);
            ez.assign(f + 2 * n3, f + 3 * n3);
        }
        return true;
    }

    void interp(const kern::MeshV& M, const double* ex, const double* ey, const double* ez, int n, const double* x,
                const double* y, const double* z, const int* lost, double* scx, double* scy, double* scz) override {
        if (n <= 0) return;
        @autoreleasepool {
            const size_t n3 = static_cast<size_t>(M.nx) * M.ny * M.nz;
            fl(bx_, n, x);
            fl(by_, n, y);
            fl(bz_, n, z);
            std::memcpy([buf(blost_, n * 4) contents], lost, static_cast<size_t>(n) * 4);
            id<MTLBuffer> fe = buf(bmf_, 3 * n3 * 4);
            float* f = static_cast<float*>([fe contents]);
            for (size_t e = 0; e < n3; ++e) {
                f[e] = static_cast<float>(ex[e]);
                f[n3 + e] = static_cast<float>(ey[e]);
                f[2 * n3 + e] = static_cast<float>(ez[e]);
            }
            id<MTLBuffer> sc = buf(bsc_, static_cast<size_t>(n) * 12);
            MeshF F = mesh(M);
            const std::uint32_t nn = static_cast<std::uint32_t>(n);
            run(interp_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
                [enc setBytes:&F length:sizeof(F) atIndex:0];
                [enc setBuffer:bx_ offset:0 atIndex:1];
                [enc setBuffer:by_ offset:0 atIndex:2];
                [enc setBuffer:bz_ offset:0 atIndex:3];
                [enc setBuffer:blost_ offset:0 atIndex:4];
                [enc setBuffer:fe offset:0 atIndex:5];
                [enc setBuffer:fe offset:n3 * 4 atIndex:6];
                [enc setBuffer:fe offset:2 * n3 * 4 atIndex:7];
                [enc setBuffer:sc offset:0 atIndex:8];
                [enc setBytes:&nn length:4 atIndex:9];
                [enc useResource:bnodes_ usage:MTLResourceUsageRead];
            });
            const float* s = static_cast<const float*>([sc contents]);
            for (int p = 0; p < n; ++p) {
                scx[p] = s[3 * p];
                scy[p] = s[3 * p + 1];
                scz[p] = s[3 * p + 2];
            }
        }
    }

private:
    id<MTLDevice> dev_ = nil;
    id<MTLCommandQueue> queue_ = nil;
    id<MTLComputePipelineState> sizes_ = nil, dep_ = nil, interp_ = nil, picnic_ = nil, sym_ = nil, corr_ = nil, grad_ = nil;
    id<MTLBuffer> sz_ = nil, bx_ = nil, by_ = nil, bz_ = nil, bq_ = nil, blost_ = nil, blo_ = nil, bhi_ = nil, bnodes_ = nil,
                  brho_ = nil, bG_ = nil, bphi_ = nil, bfield_ = nil, bmf_ = nil, bsc_ = nil, bcorr_ = nil, bdval_ = nil;
    std::string name_;

    // shared (unified memory) buffer of at least `bytes`, kept for the next call
    id<MTLBuffer> buf(__strong id<MTLBuffer>& b, size_t bytes) {
        if (bytes == 0) bytes = 4;
        if (!b || [b length] < bytes) {
            b = [dev_ newBufferWithLength:bytes options:MTLResourceStorageModeShared];
            if (!b) throw EngineError("Metal: out of memory (" + std::to_string(bytes) + " bytes)");
        }
        return b;
    }
    // double -> float into a buffer
    float* fl(__strong id<MTLBuffer>& b, size_t n, const double* v) {
        float* f = static_cast<float*>([buf(b, n * 4) contents]);
        for (size_t i = 0; i < n; ++i) f[i] = static_cast<float>(v[i]);
        return f;
    }
    // MeshV in the Metal layout; the node coordinates go into bnodes_
    MeshF mesh(const kern::MeshV& M) {
        id<MTLBuffer> b = buf(bnodes_, static_cast<size_t>(M.nx + M.ny + M.nz) * 4);
        float* f = static_cast<float*>([b contents]);
        for (int i = 0; i < M.nx; ++i) f[i] = static_cast<float>(M.xs[i]);
        for (int j = 0; j < M.ny; ++j) f[M.nx + j] = static_cast<float>(M.ys[j]);
        for (int k = 0; k < M.nz; ++k) f[M.nx + M.ny + k] = static_cast<float>(M.zs[k]);
        MeshF F{};
        F.nx = M.nx;
        F.ny = M.ny;
        F.nz = M.nz;
        F.dx = static_cast<float>(M.dx);
        F.dy = static_cast<float>(M.dy);
        F.dz = static_cast<float>(M.dz);
        F.vol = static_cast<float>(M.vol);
        F.gamma = static_cast<float>(M.gamma);
        F.igamma = static_cast<float>(M.igamma);
        F.c0 = static_cast<float>(M.c0);
        F.c2 = static_cast<float>(M.c2);
        F.c4 = static_cast<float>(M.c4);
        F.lpRel = M.lpRel;
        F.lpc = M.lpc;
        F.compat = M.compat;
        const std::uint64_t a = [b gpuAddress];
        F.xs = a;
        F.ys = a + static_cast<std::uint64_t>(M.nx) * 4;
        F.zs = a + static_cast<std::uint64_t>(M.nx + M.ny) * 4;
        F.Q = static_cast<float>(M.Q);
        F.rotMode = M.rotMode;
        F.rs = static_cast<float>(M.rs);
        F.rc = static_cast<float>(M.rc);
        F.fixScaleExp = M.fixScaleExp;
        return F;
    }
    template <class Setup>
    void run(id<MTLComputePipelineState> ps, size_t threads, Setup&& setup) {
        if (threads == 0) return;
        id<MTLCommandBuffer> cb = [queue_ commandBuffer];
        id<MTLComputeCommandEncoder> enc = [cb computeCommandEncoder];
        [enc setComputePipelineState:ps];
        setup(enc);
        const NSUInteger tg = std::min<NSUInteger>(256, [ps maxTotalThreadsPerThreadgroup]);
        [enc dispatchThreads:MTLSizeMake(threads, 1, 1) threadsPerThreadgroup:MTLSizeMake(tg, 1, 1)];
        [enc endEncoding];
        [cb commit];
        [cb waitUntilCompleted];
        if ([cb status] == MTLCommandBufferStatusError)
            throw EngineError(std::string("Metal command failed: ") + [[[cb error] localizedDescription] UTF8String]);
    }
};

}  // namespace

bool metal_compiled() { return true; }

std::unique_ptr<ScAccel> make_metal_accel(std::string& why) {
    auto a = std::make_unique<MetalAccel>();
    if (!a->init(why)) return nullptr;
    return a;
}

}  // namespace avas
