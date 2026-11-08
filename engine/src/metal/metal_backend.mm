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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "core/device.h"
#include "core/errors.h"
#include "core/format.h"
#include "core/kernels.h"
#include "core/lattice.h"
#include "core/physics.h"
#include "core/reduce.h"
#include "core/sc_kernels.h"

#include "avas_msl_source.inc"  // kAvasMslSource

namespace avas {

namespace f32cpu {  // core/f32_cpu.cpp: the same kernels on the CPU (fp32 tier of the emu device)
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
void group(const float* C, const float* Mx, long long* sums, int* exps, float* mxo, int n);
}  // namespace f32cpu

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

// Metal layouts of kern::ElemV and kern::LatV (double -> float, pointers -> GPU addresses).
struct ElemVF {
    int kind, fieldType, mapIndex, inSuperposeOut, rotMode;
    int nsub, sub0;
    int rfPlain, rfErr;
    int errUse, hetErr;
    float zStart, length, radius, strength, ke, kb, keErr, kbErr, phaseErr;
    float errDx, errDy, errDz;
    float esx, ecx, esy, ecy, esz, ecz;
};
struct SubVF {
    int type, map, hasErr, rfErr;
    float ke, kb, zStart, radius, length;
    float place[6];
    float s3, c3, s4, c4, s5, c5;
    float err[12];
    float es4, ec4, es5, ec5, es6, ec6, esin6;
};
struct GeomFH {  // GeomF of kernels.metal
    int shape, npar;
    float zStart;
    float par[6];
    float place[6];
    float s3, c3, s4, c4, s5, c5;
};
struct BoundUH {  // BoundU of kernels.metal
    float zs, post0, post2, totalLength, segEndLast;
    int nSeg, nBnd, spOutCur, spOutSize, n;
};
struct LatVF {
    std::uint64_t elems, eff, subs, maps, mapsF, elemZStart, elemZEnd, spOut, cavTime, cavPhase, rfSlots;
    int nElem, errOn, spOutCur, spOutCount, spOutSize;
    float totalLength, spExit;
    std::uint64_t geoms, segStart, segEnd;
    int nSeg;
    std::uint64_t bndZ, bndR1, bndR2;
    int nBnd;
};
struct PartFH {  // PartF of kernels.metal
    std::uint64_t d[3], du[3], E[3], B[3], sc[3], rel[6], hq, tq, w, spz0, spz1;
    std::uint64_t lost, elemIndex, nextPlane, boundaryMode, recFlag, spFlag, geomIndex, spOutCount, C, Mx, snap, oldS, crossCount,
        crossIdx, spIdx, bdIdx;
};
struct StepUH {  // StepU of kernels.metal
    float hEs[3], usm[3], gsm, Ks[9], usp[3], gsp, cdt;
    float post0, post2, zsPost;
    float cen[6];
    float gp, zFieldPre, segEnd, zNeg, rs, rc;
    float z0, spx, tot;
    int zeroField;
    int rotMode, nEvents, writeOld, n;
};
// The synchronous particle's push of a step (as SyncStep of cuda_f32.cuh).
struct SyncH {
    float hEs[3], usm[3], gsm, Ks[9], usp[3], gsp, cdt;
    double pre[6], post[6];
};

class MetalAccel : public ScAccel {
public:
    explicit MetalAccel(bool cpu = false) : cpu_(cpu) {}
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
            step_ = pipe("k_step");
            group_ = pipe("k_group");
            bound_ = pipe("k_bound");
            if (!sizes_ || !dep_ || !interp_ || !picnic_ || !sym_ || !corr_ || !grad_ || !step_ || !group_ || !bound_) return false;
            queue_ = [dev_ newCommandQueue];
            // layout check of the structures shared with the MSL code
            id<MTLBuffer> b = buf(sz_, 64);
            if (cpu_) f32cpu::sizes(static_cast<unsigned*>([b contents]));
            else run(sizes_, 1, [&](id<MTLComputeCommandEncoder> enc) { [enc setBuffer:b offset:0 atIndex:0]; });
            const std::uint32_t* s = static_cast<const std::uint32_t*>([b contents]);
            if (s[0] != sizeof(MeshF) || s[1] != sizeof(sck::SpGrid) || s[2] != sizeof(LatVF) || s[3] != sizeof(ElemVF) ||
                s[4] != sizeof(MapView<float, float>) || s[5] != sizeof(PartFH) || s[6] != sizeof(StepUH) || s[7] != sizeof(SubVF) ||
                s[8] != sizeof(GeomFH) || s[9] != sizeof(BoundUH)) {
                why = "Metal structure layout mismatch (MeshV " + std::to_string(s[0]) + " / " + std::to_string(sizeof(MeshF)) + ")";
                return false;
            }
            name_ = [[dev_ name] UTF8String];
        }
        return true;
    }

    std::string describe() const override {
        if (cpu_) return "emu fp32 (the kernels of the Metal back end on the CPU, core/f32_kernels.h)";
        return "Metal, " + name_ + ": t-code push, statistics and space-charge mesh work on the GPU in fp32 (z-code, FFT solve, "
               "secondary particles on the CPU)";
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
            if (cpu_) f32cpu::deposit(&F, hp<float>(bx_), hp<float>(by_), hp<float>(bz_), hp<float>(bq_), hp<int>(blost_), hp<unsigned>(lo),
                                      hp<unsigned>(hi), n);
            else run(dep_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
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
                if (cpu_) f32cpu::picnic(&g, hp<float>(brho_), hp<float>(bG_), hp<float>(phi));
                else run(picnic_, n3, [&](id<MTLComputeCommandEncoder> enc) {
                    [enc setBytes:&g length:sizeof(g) atIndex:0];
                    [enc setBuffer:brho_ offset:0 atIndex:1];
                    [enc setBuffer:bG_ offset:0 atIndex:2];
                    [enc setBuffer:phi offset:0 atIndex:3];
                });
            } else {
                const float thr = static_cast<float>(in.thr);
                if (cpu_) f32cpu::sym(&g, hp<float>(brho_), hp<float>(bG_), thr, hp<float>(phi));
                else run(sym_, static_cast<size_t>(g.n_targets()), [&](id<MTLComputeCommandEncoder> enc) {
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
                    if (cpu_) f32cpu::corr(&g, hp<float>(bG_), hp<int>(bcorr_), hp<float>(bdval_), nc, hp<float>(phi));
                    else run(corr_, n3, [&](id<MTLComputeCommandEncoder> enc) {
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
            if (cpu_) f32cpu::grad(&g, hp<float>(phi), h, hp<float>(fe), hp<float>(fe, n3 * 4), hp<float>(fe, 2 * n3 * 4));
            else run(grad_, n3, [&](id<MTLComputeCommandEncoder> enc) {
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
            if (cpu_) f32cpu::interp(&F, hp<float>(bx_), hp<float>(by_), hp<float>(bz_), hp<int>(blost_), hp<float>(fe), hp<float>(fe, n3 * 4),
                                     hp<float>(fe, 2 * n3 * 4), hp<float>(sc), n);
            else run(interp_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
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

    // ---------------- particle push (phase 2) ----------------
    void begin_run(const LatHost& L, int n) override {
        auth_ = false;
        n_ = n;
        stepOk_ = true;
        why_.clear();
        @autoreleasepool {
            set_elems(L);
            // field maps in float (the same conversion as the CUDA fp32 tier)
            size_t tot = 0;
            for (const MapView<double>& m : L.maps) tot += static_cast<size_t>(m.nx + 1) * (m.ny + 1) * (m.nz + 1) * m.ncomp;
            id<MTLBuffer> md = buf(bmapd_, std::max<size_t>(tot, 1) * 4);
            float* f = static_cast<float*>([md contents]);
            MapView<float, float>* mv =
                static_cast<MapView<float, float>*>([buf(bmaps_, std::max<size_t>(L.maps.size(), 1) * sizeof(MapView<float, float>)) contents]);
            size_t o = 0;
            for (size_t q = 0; q < L.maps.size(); ++q) {
                const MapView<double>& m = L.maps[q];
                const size_t cnt = static_cast<size_t>(m.nx + 1) * (m.ny + 1) * (m.nz + 1) * m.ncomp;
                for (size_t e = 0; e < cnt; ++e) f[o + e] = static_cast<float>(m.data[e]);
                MapView<float, float> g{};
                g.data = reinterpret_cast<const float*>(addr(md) + o * 4);
                g.ncomp = m.ncomp;
                g.nx = m.nx;
                g.ny = m.ny;
                g.nz = m.nz;
                g.xmin = static_cast<float>(m.xmin);
                g.xmax = static_cast<float>(m.xmax);
                g.ymin = static_cast<float>(m.ymin);
                g.ymax = static_cast<float>(m.ymax);
                g.zlen = static_cast<float>(m.zlen);
                g.dx = static_cast<float>(m.dx);
                g.dy = static_cast<float>(m.dy);
                g.dz = static_cast<float>(m.dz);
                mv[q] = g;
                o += cnt;
            }
            latZ_ = &L;
            // particle state: one shared buffer, slices
            const size_t m = static_cast<size_t>(std::max(n, 1));
            const size_t floats = 26 * m + 21 * m + 8 * m + 12 * m + 6 * m;
            buf(bstate_, floats * 4);
            buf(bints_, 8 * m * 4);
            buf(bcross_, (3 * m + 4) * 4);
            const std::uint64_t fa = addr(bstate_), ia = addr(bints_), ca = addr(bcross_);
            auto fs = [&](size_t k) { return fa + k * m * 4; };
            PartFH& P = pf_;
            for (int j = 0; j < 3; ++j) {
                P.d[j] = fs(j);
                P.du[j] = fs(3 + j);
                P.E[j] = fs(6 + j);
                P.B[j] = fs(9 + j);
                P.sc[j] = fs(12 + j);
            }
            for (int j = 0; j < 6; ++j) P.rel[j] = fs(15 + j);
            P.hq = fs(21);
            P.tq = fs(22);
            P.w = fs(23);
            P.spz0 = fs(24);
            P.spz1 = fs(25);
            P.C = fs(26);
            P.Mx = fs(26 + 21);
            P.snap = fs(26 + 21 + 8);
            P.oldS = fs(26 + 21 + 8 + 12);
            P.lost = ia;
            P.elemIndex = ia + m * 4;
            P.nextPlane = ia + 2 * m * 4;
            P.boundaryMode = ia + 3 * m * 4;
            P.recFlag = ia + 4 * m * 4;
            P.spFlag = ia + 5 * m * 4;
            P.geomIndex = ia + 6 * m * 4;
            P.spOutCount = ia + 7 * m * 4;
            P.crossCount = ca;
            P.crossIdx = ca + 16;
            P.spIdx = ca + 16 + m * 4;
            P.bdIdx = ca + 16 + 2 * m * 4;
        }
    }

    bool authoritative() const override { return auth_; }
    void invalidate() override { auth_ = false; }
    void rec_cleared(bool sp) override {
        if (n_ > 0 && bints_)
            std::memset(static_cast<char*>([bints_ contents]) + (sp ? 5 : 4) * static_cast<size_t>(std::max(n_, 1)) * 4, 0,
                        static_cast<size_t>(n_) * 4);
    }

    // element tables (also when the superposeout beam frame changes rotMode)
    void set_elems(const LatHost& L) override {
        @autoreleasepool {
            const size_t nE = L.elems.size();
            ElemVF* ev = static_cast<ElemVF*>([buf(belems_, std::max<size_t>(nE, 1) * sizeof(ElemVF)) contents]);
            for (size_t k = 0; k < nE; ++k) ev[k] = elemf(L.elems[k]);
            ElemVF* fv = static_cast<ElemVF*>([buf(beff_, std::max<size_t>(L.eff.size(), 1) * sizeof(ElemVF)) contents]);
            for (size_t k = 0; k < L.eff.size(); ++k) fv[k] = elemf(L.eff[k]);
            SubVF* sv = static_cast<SubVF*>([buf(bsubs_, std::max<size_t>(L.subs.size(), 1) * sizeof(SubVF)) contents]);
            for (size_t q = 0; q < L.subs.size(); ++q) sv[q] = subf(L.subs[q]);
            hasEff_ = !L.eff.empty();
            errOn_ = L.errOn;
            GeomFH* gv = static_cast<GeomFH*>([buf(bgeom_, std::max<size_t>(L.geoms.size(), 1) * sizeof(GeomFH)) contents]);
            for (size_t q = 0; q < L.geoms.size(); ++q) {
                const kern::GeomV& g = L.geoms[q];
                GeomFH f{};
                f.shape = g.shape;
                f.npar = g.npar;
                f.zStart = static_cast<float>(g.zStart);
                for (int j = 0; j < 6; ++j) {
                    f.par[j] = static_cast<float>(g.par[j]);
                    f.place[j] = static_cast<float>(g.place[j]);
                }
                f.s3 = static_cast<float>(g.s3);
                f.c3 = static_cast<float>(g.c3);
                f.s4 = static_cast<float>(g.s4);
                f.c4 = static_cast<float>(g.c4);
                f.s5 = static_cast<float>(g.s5);
                f.c5 = static_cast<float>(g.c5);
                gv[q] = f;
            }
            const size_t ns = L.segStart.size();
            float* sg = static_cast<float*>([buf(bseg_, std::max<size_t>(2 * ns, 1) * 4) contents]);
            for (size_t q = 0; q < ns; ++q) {
                sg[q] = static_cast<float>(L.segStart[q]);
                sg[ns + q] = static_cast<float>(L.segEnd[q]);
            }
            const size_t nb = L.bndR1.size();
            float* bz = static_cast<float*>([buf(bbndz_, std::max<size_t>(L.bndZ.size(), 1) * 4) contents]);
            for (size_t q = 0; q < L.bndZ.size(); ++q) bz[q] = static_cast<float>(L.bndZ[q]);
            int* br = static_cast<int*>([buf(bbndr_, std::max<size_t>(2 * nb, 1) * 4) contents]);
            for (size_t q = 0; q < nb; ++q) {
                br[q] = L.bndR1[q];
                br[nb + q] = L.bndR2[q];
            }
        }
    }

    bool step(const LatScalars& ls, const kern::StepArgs& a, const std::vector<double>& rf, const std::vector<double>& events,
              const kern::PartV& P, int n, std::vector<StepGroup>& out, long long& spCount, long long& recCount) override {
        if (!stepOk_ || n != n_ || n <= 0) {
            if (!stepOk_ && !warned_) {
                say("Metal: the particle push stays on the CPU for this lattice (" + why_ + ")");
                warned_ = true;
            }
            return false;
        }
        hostP_ = P;
        const double t0 = now();
        double t1 = t0, t2 = t0;
        @autoreleasepool {
            const SyncH S = sync_step(a);
            if (!auth_) {
                from64(P, n, S, a.dt);
                ++nFrom_;
            }
            auth_ = true;
            ref_ = S;
            // per-step tables relative to the synchronous particle (as f32_offsets of the CUDA tier)
            const double zs = S.post[4];
            const LatHost& L = *latZ_;
            const size_t nE = L.elems.size(), nS = L.subs.size(), nEv = events.size();
            std::vector<float> o;
            o.reserve(3 * nE + nS + 2 * nEv + 1);
            for (size_t k = 0; k < nE; ++k) o.push_back(static_cast<float>(L.zStart[k] - zs));
            for (size_t k = 0; k < nE; ++k) o.push_back(static_cast<float>(L.zEnd[k] - zs));
            for (size_t k = 0; k < nE; ++k) o.push_back(static_cast<float>(L.elems[k].zStart - zs));
            for (const kern::SubV& q : L.subs) o.push_back(static_cast<float>(q.zStart - zs));
            for (double e : events) o.push_back(static_cast<float>(e - zs));
            for (double e : events) o.push_back(e <= ls.spExit ? 1.0f : 0.0f);
            o.push_back(0.0f);
            std::memcpy([buf(boff_, o.size() * 4) contents], o.data(), o.size() * 4);
            float* rt = static_cast<float*>([buf(brf_, std::max<size_t>(rf.size(), 1) * 4) contents]);
            for (size_t q = 0; q < rf.size(); ++q) rt[q] = static_cast<float>(rf[q]);
            const int sizes[4] = {static_cast<int>(nE), static_cast<int>(nS), static_cast<int>(nEv), 0};
            StepUH U{};
            for (int j = 0; j < 3; ++j) {
                U.hEs[j] = S.hEs[j];
                U.usm[j] = S.usm[j];
                U.usp[j] = S.usp[j];
            }
            for (int j = 0; j < 9; ++j) U.Ks[j] = S.Ks[j];
            U.gsm = S.gsm;
            U.gsp = S.gsp;
            U.cdt = S.cdt;
            U.post0 = static_cast<float>(S.post[0]);
            U.post2 = static_cast<float>(S.post[2]);
            U.zsPost = static_cast<float>(zs);
            for (int j = 0; j < 6; ++j) U.cen[j] = static_cast<float>(a.cen[j]);
            U.gp = static_cast<float>(a.gp);
            U.zFieldPre = static_cast<float>(a.zField - S.pre[4]);
            U.segEnd = static_cast<float>(a.segEnd - zs);
            U.zNeg = static_cast<float>(0.0 - zs);
            U.rs = static_cast<float>(a.rs);
            U.rc = static_cast<float>(a.rc);
            U.z0 = static_cast<float>(0.0 - zs);
            U.spx = static_cast<float>(ls.spExit - zs);
            U.tot = static_cast<float>(ls.totalLength - zs);
            U.zeroField = a.zMinPrev <= a.zField ? 1 : 0;
            U.rotMode = a.rotMode;
            U.nEvents = a.nEvents;
            U.writeOld = a.writeOld;
            U.n = n;
            LatVF LV{};
            LV.elems = addr(belems_);
            LV.eff = hasEff_ ? addr(beff_) : addr(belems_);
            LV.subs = addr(bsubs_);
            LV.mapsF = addr(bmaps_);
            LV.nElem = static_cast<int>(nE);
            LV.errOn = errOn_ ? 1 : 0;
            LV.spOutCur = ls.spOutCur;
            LV.spOutCount = ls.spOutCount;
            LV.spOutSize = ls.spOutSize;
            LV.totalLength = static_cast<float>(ls.totalLength);
            LV.spExit = static_cast<float>(ls.spExit);
            for (int c = 0; c < 4; ++c) static_cast<std::uint32_t*>([bcross_ contents])[c] = 0;
            if (cpu_) f32cpu::step(&LV, &U, &pf_, hp<float>(boff_), hp<float>(brf_), sizes, n);
            else run(step_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
                [enc setBytes:&LV length:sizeof(LV) atIndex:0];
                [enc setBytes:&U length:sizeof(U) atIndex:1];
                [enc setBytes:&pf_ length:sizeof(pf_) atIndex:2];
                [enc setBuffer:boff_ offset:0 atIndex:3];
                [enc setBuffer:brf_ offset:0 atIndex:4];
                [enc setBytes:sizes length:sizeof(sizes) atIndex:5];
                [enc useResource:belems_ usage:MTLResourceUsageRead];
                [enc useResource:beff_ usage:MTLResourceUsageRead];
                [enc useResource:bsubs_ usage:MTLResourceUsageRead];
                [enc useResource:bmaps_ usage:MTLResourceUsageRead];
                [enc useResource:bmapd_ usage:MTLResourceUsageRead];
                [enc useResource:bstate_ usage:MTLResourceUsageRead | MTLResourceUsageWrite];
                [enc useResource:bints_ usage:MTLResourceUsageRead | MTLResourceUsageWrite];
                [enc useResource:bcross_ usage:MTLResourceUsageRead | MTLResourceUsageWrite];
            });
            t1 = now();
            // statistics per reduction group (fixed point on the GPU, merged in double here)
            const int ng = red_groups(n);
            id<MTLBuffer> gs = buf(bgs_, static_cast<size_t>(ng) * 21 * 8), ge = buf(bge_, static_cast<size_t>(ng) * 21 * 4),
                          gm = buf(bgm_, static_cast<size_t>(ng) * 8 * 4);
            if (cpu_) {
                const size_t m0 = static_cast<size_t>(std::max(n_, 1));
                f32cpu::group(hp<float>(bstate_, 26 * m0 * 4), hp<float>(bstate_, (26 + 21) * m0 * 4), hp<long long>(gs), hp<int>(ge),
                              hp<float>(gm), n);
            } else {
                id<MTLCommandBuffer> cb = [queue_ commandBuffer];
                id<MTLComputeCommandEncoder> enc = [cb computeCommandEncoder];
                [enc setComputePipelineState:group_];
                [enc setBuffer:bstate_ offset:26 * static_cast<size_t>(std::max(n_, 1)) * 4 atIndex:0];
                [enc setBuffer:bstate_ offset:(26 + 21) * static_cast<size_t>(std::max(n_, 1)) * 4 atIndex:1];
                [enc setBuffer:gs offset:0 atIndex:2];
                [enc setBuffer:ge offset:0 atIndex:3];
                [enc setBuffer:gm offset:0 atIndex:4];
                [enc setBytes:&n length:4 atIndex:5];
                [enc dispatchThreadgroups:MTLSizeMake(static_cast<NSUInteger>(ng), 1, 1) threadsPerThreadgroup:MTLSizeMake(256, 1, 1)];
                [enc endEncoding];
                [cb commit];
                [cb waitUntilCompleted];
            }
            t2 = now();
            const std::int64_t* sv = static_cast<const std::int64_t*>([gs contents]);
            const std::int32_t* ex = static_cast<const std::int32_t*>([ge contents]);
            const float* mx = static_cast<const float*>([gm contents]);
            out.assign(static_cast<size_t>(ng), StepGroup{});
            for (int g = 0; g < ng; ++g) {
                StepGroup& og = out[static_cast<size_t>(g)];
                const std::int64_t* v = sv + 21 * g;
                const std::int32_t* e = ex + 21 * g;
                for (int k = 0; k < kern::kStepSums; ++k) og.s[k] = std::ldexp(static_cast<double>(v[k]), -e[k]);
                og.sp = 0.0;
                og.rec = std::ldexp(static_cast<double>(v[20]), -e[20]);
                for (int j = 0; j < 6; ++j) og.mx[j] = mx[8 * g + j];
                const float mzn = mx[8 * g + 6], mzx = mx[8 * g + 7];
                og.zmin = std::isinf(mzn) ? 1e10 : S.pre[4] + static_cast<double>(-mzn);
                og.zmax = std::isinf(mzx) ? -1e10 : S.pre[4] + static_cast<double>(mzx);
            }
            // crossings of output planes: completed in double (as the CUDA fp32 tier)
            const std::uint32_t nc = *static_cast<const std::uint32_t*>([bcross_ contents]);
            const std::uint32_t* idx = static_cast<const std::uint32_t*>([bcross_ contents]) + 4;
            const float* snap = static_cast<const float*>([bstate_ contents]) + (26 + 21 + 8) * static_cast<size_t>(std::max(n_, 1));
            int* np = static_cast<int*>([bints_ contents]) + 2 * static_cast<size_t>(std::max(n_, 1));
            int* rflag = static_cast<int*>([bints_ contents]) + 4 * static_cast<size_t>(std::max(n_, 1));
            for (std::uint32_t c = 0; c < nc; ++c) {
                const int i = static_cast<int>(idx[c]);
                const float* sn = snap + 12 * static_cast<size_t>(i);
                double cur[6], old[6];
                for (int j = 0; j < 3; ++j) {
                    cur[2 * j] = S.post[2 * j] + static_cast<double>(sn[6 + j]);
                    cur[2 * j + 1] = S.post[2 * j + 1] + static_cast<double>(sn[9 + j]);
                    old[2 * j] = S.pre[2 * j] + static_cast<double>(sn[j]);
                    old[2 * j + 1] = S.pre[2 * j + 1] + static_cast<double>(sn[3 + j]);
                }
                const double pl = events[static_cast<size_t>(np[i])];
                if (pl <= cur[4]) {
                    const double r = (pl - old[4]) / (cur[4] - old[4]);
                    double* rec = P.rec + 8 * static_cast<long long>(i);
                    for (int j = 0; j < 6; ++j) rec[j] = (cur[j] - old[j]) * r + old[j];
                    kern::interp_momentum_norm(rec, old, cur, r);
                    rec[6] = (r - 1.0) * a.dt + a.tEnd;
                    if (rec[7] != 1.0) out[static_cast<size_t>(i / kRedGroup)].rec += 1.0;
                    rec[7] = 1.0;
                    rflag[i] = 1;
                    np[i] += 1;
                }
            }
            // superposeout exits: the candidates of the GPU decided and recorded in double
            const std::uint32_t ns = static_cast<const std::uint32_t*>([bcross_ contents])[1];
            const std::uint32_t* sidx = static_cast<const std::uint32_t*>([bcross_ contents]) + 4 + std::max(n_, 1);
            const int* eidx = static_cast<const int*>([bints_ contents]) + static_cast<size_t>(std::max(n_, 1));
            int* sflag = static_cast<int*>([bints_ contents]) + 5 * static_cast<size_t>(std::max(n_, 1));
            for (std::uint32_t c = 0; c < ns; ++c) {
                const int i = static_cast<int>(sidx[c]);
                const float* sn = snap + 12 * static_cast<size_t>(i);
                double cur[6], old[6];
                for (int j = 0; j < 3; ++j) {
                    cur[2 * j] = S.post[2 * j] + static_cast<double>(sn[6 + j]);
                    cur[2 * j + 1] = S.post[2 * j + 1] + static_cast<double>(sn[9 + j]);
                    old[2 * j] = S.pre[2 * j] + static_cast<double>(sn[j]);
                    old[2 * j + 1] = S.pre[2 * j + 1] + static_cast<double>(sn[3 + j]);
                }
                if (!(cur[4] > P.spOutZ0[i]) || P.spRec[8 * static_cast<long long>(i) + 7] != 0.0) continue;
                const kern::SpOutV& row = L.spOut[static_cast<size_t>(ls.spOutCur)];
                const double zst = L.elems[static_cast<size_t>(eidx[i])].zStart;
                double lc[6], lp[6];
                kern::sp_local(row, zst, cur, lc);
                kern::sp_local(row, zst, old, lp);
                if (lc[4] > 0.0) {
                    const double r = lp[4] / (lp[4] - lc[4]);
                    double* sr = P.spRec + 8 * static_cast<long long>(i);
                    for (int j = 0; j < 6; ++j) sr[j] = (lc[j] - lp[j]) * r + lp[j];
                    kern::interp_momentum_norm(sr, lp, lc, r);
                    sr[6] = (r - 1.0) * a.dt + a.tEnd;
                    sr[7] = 1.0;
                    sflag[i] = 1;
                    out[static_cast<size_t>(i / kRedGroup)].sp += 1.0;
                }
            }
            // old (writeOld, boundary.txt): kept in float, written into the fp64 array on demand
            oldValid_ = a.writeOld != 0;
            if (oldValid_)
                for (int j = 0; j < 6; ++j) oldPre_[j] = S.pre[j];
            spCount = recCount = 0;
            for (const StepGroup& og : out) {
                recCount += static_cast<long long>(og.rec);
                spCount += static_cast<long long>(og.sp);
            }
        }
        const double t3 = now();
        ++nStep_;
        tStep_ += t1 - t0;
        tGroup_ += t2 - t1;
        tHost_ += t3 - t2;
        return true;
    }

    // float state -> fp64 arrays (absolute coordinates from the synchronous particle after the last step)
    void to_host(const kern::PartV& P, int n) override {
        if (!auth_) return;
        ++nTo_;
        const size_t m = static_cast<size_t>(std::max(n_, 1));
        const float* f = static_cast<const float*>([bstate_ contents]);
        const int* iv = static_cast<const int*>([bints_ contents]);
        double* c[6] = {P.x, P.ux, P.y, P.uy, P.z, P.uz};
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < 3; ++j) {
                c[2 * j][i] = ref_.post[2 * j] + static_cast<double>(f[j * m + i]);
                c[2 * j + 1][i] = ref_.post[2 * j + 1] + static_cast<double>(f[(3 + j) * m + i]);
            }
            P.ex[i] = f[6 * m + i];
            P.ey[i] = f[7 * m + i];
            P.ez[i] = f[8 * m + i];
            P.bx[i] = f[9 * m + i];
            P.by[i] = f[10 * m + i];
            P.bz[i] = f[11 * m + i];
            P.scx[i] = f[12 * m + i];
            P.scy[i] = f[13 * m + i];
            P.scz[i] = f[14 * m + i];
            for (int j = 0; j < 6; ++j) P.rel[j][i] = f[(15 + j) * m + i];
            P.lost[i] = iv[i];
            P.elemIndex[i] = iv[m + i];
            P.nextPlane[i] = iv[2 * m + i];
            P.boundaryMode[i] = iv[3 * m + i];
            P.geomIndex[i] = iv[6 * m + i];
        }
        if (oldValid_ && P.old) {
            const float* os = f + (26 + 21 + 8 + 12) * m;
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < 6; ++j) P.old[6 * static_cast<long long>(i) + j] = oldPre_[j] + static_cast<double>(os[6 * i + j]);
        }
        auth_ = false;
    }

    // boundary.txt: float pre-selection on the GPU (k_bound), the candidates in double here with the
    // shared kern::boundary_particle (the same decisions and wall coordinates as the CPU engine).
    bool boundary(const LatScalars& ls, double t, double dt, std::vector<BoundaryHit>& hits) override {
        if (!auth_ || !latZ_ || latZ_->segEnd.empty()) return false;
        @autoreleasepool {
            const LatHost& L = *latZ_;
            const size_t m = static_cast<size_t>(std::max(n_, 1));
            BoundUH U{};
            U.zs = static_cast<float>(ref_.post[4]);
            U.post0 = static_cast<float>(ref_.post[0]);
            U.post2 = static_cast<float>(ref_.post[2]);
            U.totalLength = static_cast<float>(ls.totalLength);
            U.segEndLast = static_cast<float>(L.segEnd.back());
            U.nSeg = static_cast<int>(L.segStart.size());
            U.nBnd = static_cast<int>(L.bndR1.size());
            U.spOutCur = ls.spOutCur;
            U.spOutSize = ls.spOutSize;
            U.n = n_;
            static_cast<std::uint32_t*>([bcross_ contents])[2] = 0;
            const size_t ns = L.segStart.size(), nb = L.bndR1.size();
            if (cpu_) f32cpu::bound(&U, &pf_, [bgeom_ contents], hp<float>(bseg_), hp<float>(bseg_, ns * 4), hp<float>(bbndz_),
                                    hp<int>(bbndr_), hp<int>(bbndr_, nb * 4), n_);
            else run(bound_, static_cast<size_t>(n_), [&](id<MTLComputeCommandEncoder> enc) {
                [enc setBytes:&U length:sizeof(U) atIndex:0];
                [enc setBytes:&pf_ length:sizeof(pf_) atIndex:1];
                [enc setBuffer:bgeom_ offset:0 atIndex:2];
                [enc setBuffer:bseg_ offset:0 atIndex:3];
                [enc setBuffer:bseg_ offset:ns * 4 atIndex:4];
                [enc setBuffer:bbndz_ offset:0 atIndex:5];
                [enc setBuffer:bbndr_ offset:0 atIndex:6];
                [enc setBuffer:bbndr_ offset:nb * 4 atIndex:7];
                [enc useResource:bstate_ usage:MTLResourceUsageRead];
                [enc useResource:bints_ usage:MTLResourceUsageRead | MTLResourceUsageWrite];
                [enc useResource:bcross_ usage:MTLResourceUsageRead | MTLResourceUsageWrite];
            });
            const std::uint32_t nc = static_cast<const std::uint32_t*>([bcross_ contents])[2];
            std::vector<std::uint32_t> cand(static_cast<const std::uint32_t*>([bcross_ contents]) + 4 + 2 * m,
                                            static_cast<const std::uint32_t*>([bcross_ contents]) + 4 + 2 * m + nc);
            std::sort(cand.begin(), cand.end());  // hits in particle order, as on the CPU
            kern::LatV LV = L.view(Lattice());
            LV.spOutCur = ls.spOutCur;
            LV.spOutCount = ls.spOutCount;
            LV.spOutSize = ls.spOutSize;
            LV.totalLength = ls.totalLength;
            LV.spExit = ls.spExit;
            const float* f = static_cast<const float*>([bstate_ contents]);
            int* iv = static_cast<int*>([bints_ contents]);
            hits.clear();
            const kern::PartV& PH = hostP_;
            for (std::uint32_t c : cand) {
                const size_t i = c;
                double x[6], old[6], spz0 = PH.spOutZ0[i];
                int lost = iv[i], spc = iv[7 * m + i], gi = iv[6 * m + i];
                for (int j = 0; j < 3; ++j) {
                    x[2 * j] = ref_.post[2 * j] + static_cast<double>(f[j * m + i]);
                    x[2 * j + 1] = ref_.post[2 * j + 1] + static_cast<double>(f[(3 + j) * m + i]);
                }
                const float* os = f + (26 + 21 + 8 + 12) * m + 6 * i;
                for (int j = 0; j < 6; ++j) old[j] = oldValid_ ? oldPre_[j] + static_cast<double>(os[j]) : PH.old[6 * i + j];
                kern::PartV T{};
                T.x = &x[0];
                T.ux = &x[1];
                T.y = &x[2];
                T.uy = &x[3];
                T.z = &x[4];
                T.uz = &x[5];
                T.lost = &lost;
                T.spOutZ0 = &spz0;
                T.spOutCount = &spc;
                T.geomIndex = &gi;
                T.old = old;
                T.sync = -1;
                BoundaryHit h{};
                h.tHit = t;
                const int code = kern::boundary_particle<PortableMath>(LV, T, 0, t, dt, false, h.geom, h.P, h.tHit);
                iv[6 * m + i] = gi;
                iv[i] = lost;
                if (code == 0) continue;
                if (code == 1) iv[i] = 1;
                h.i = static_cast<int>(i);
                h.code = code;
                hits.push_back(h);
            }
            ++nBound_;
            nCand_ += nc;
        }
        return true;
    }

    void deposit_state(const kern::MeshV& M, int n, std::vector<long long>& acc) override {
        @autoreleasepool {
            const size_t m = static_cast<size_t>(std::max(n_, 1));
            const size_t n3 = static_cast<size_t>(M.nx) * M.ny * M.nz;
            float* q = static_cast<float*>([buf(bq_, static_cast<size_t>(n) * 4) contents]);
            for (int i = 0; i < n; ++i) q[i] = charge_[static_cast<size_t>(i)];
            id<MTLBuffer> lo = buf(blo_, n3 * 4), hi = buf(bhi_, n3 * 4);
            std::memset([lo contents], 0, n3 * 4);
            std::memset([hi contents], 0, n3 * 4);
            MeshF F = mesh(M);
            const std::uint32_t nn = static_cast<std::uint32_t>(n);
            if (cpu_) f32cpu::deposit(&F, hp<float>(bstate_, 15 * m * 4), hp<float>(bstate_, 17 * m * 4), hp<float>(bstate_, 19 * m * 4),
                                      hp<float>(bq_), hp<int>(bints_), hp<unsigned>(lo), hp<unsigned>(hi), n);
            else run(dep_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
                [enc setBytes:&F length:sizeof(F) atIndex:0];
                [enc setBuffer:bstate_ offset:15 * m * 4 atIndex:1];
                [enc setBuffer:bstate_ offset:17 * m * 4 atIndex:2];
                [enc setBuffer:bstate_ offset:19 * m * 4 atIndex:3];
                [enc setBuffer:bq_ offset:0 atIndex:4];
                [enc setBuffer:bints_ offset:0 atIndex:5];
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

    void interp_state(const kern::MeshV& M, const double* ex, const double* ey, const double* ez, int n) override {
        @autoreleasepool {
            const size_t m = static_cast<size_t>(std::max(n_, 1));
            const size_t n3 = static_cast<size_t>(M.nx) * M.ny * M.nz;
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
            if (cpu_) f32cpu::interp(&F, hp<float>(bstate_, 15 * m * 4), hp<float>(bstate_, 17 * m * 4), hp<float>(bstate_, 19 * m * 4),
                                     hp<int>(bints_), hp<float>(fe), hp<float>(fe, n3 * 4), hp<float>(fe, 2 * n3 * 4), hp<float>(sc), n);
            else run(interp_, static_cast<size_t>(n), [&](id<MTLComputeCommandEncoder> enc) {
                [enc setBytes:&F length:sizeof(F) atIndex:0];
                [enc setBuffer:bstate_ offset:15 * m * 4 atIndex:1];
                [enc setBuffer:bstate_ offset:17 * m * 4 atIndex:2];
                [enc setBuffer:bstate_ offset:19 * m * 4 atIndex:3];
                [enc setBuffer:bints_ offset:0 atIndex:4];
                [enc setBuffer:fe offset:0 atIndex:5];
                [enc setBuffer:fe offset:n3 * 4 atIndex:6];
                [enc setBuffer:fe offset:2 * n3 * 4 atIndex:7];
                [enc setBuffer:sc offset:0 atIndex:8];
                [enc setBytes:&nn length:4 atIndex:9];
                [enc useResource:bnodes_ usage:MTLResourceUsageRead];
            });
            const float* sv = static_cast<const float*>([sc contents]);
            float* st = static_cast<float*>([bstate_ contents]);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < 3; ++j) st[(12 + j) * m + static_cast<size_t>(i)] = sv[3 * i + j];
        }
    }

    ~MetalAccel() override {
        if (prof_ && nStep_ > 0)
            std::fprintf(stderr, "[metal profile] %lld steps: step kernel %.3f ms, group sums %.3f ms, host %.3f ms per step; "
                         "fp64->fp32 %lld, fp32->fp64 %lld conversions; boundary.txt %lld calls on the GPU, %lld candidates\n", nStep_,
                         1e3 * tStep_ / nStep_, 1e3 * tGroup_ / nStep_, 1e3 * tHost_ / nStep_, nFrom_, nTo_, nBound_, nCand_);
    }

private:
    bool cpu_ = false;  // emu fp32: the kernels of f32_cpu.cpp on the CPU, the same buffers and host logic
    bool prof_ = std::getenv("AVAS_ENGINE_PROFILE") != nullptr;
    // address of a buffer as the kernels see it (GPU address, or the host pointer on the CPU)
    std::uint64_t addr(id<MTLBuffer> b) const { return cpu_ ? reinterpret_cast<std::uint64_t>([b contents]) : [b gpuAddress]; }
    template <class T>
    T* hp(id<MTLBuffer> b, size_t off = 0) const { return reinterpret_cast<T*>(static_cast<char*>([b contents]) + off); }
    long long nStep_ = 0, nFrom_ = 0, nTo_ = 0;
    double tStep_ = 0, tGroup_ = 0, tHost_ = 0;
    static double now() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
    bool auth_ = false, stepOk_ = false, warned_ = false, hasEff_ = false, errOn_ = false, oldValid_ = false;
    kern::PartV hostP_{};
    long long nBound_ = 0, nCand_ = 0;
    double oldPre_[6] = {0, 0, 0, 0, 0, 0};
    int n_ = 0;
    std::string why_;
    const LatHost* latZ_ = nullptr;
    PartFH pf_{};
    SyncH ref_{};
    std::vector<float> charge_;
    id<MTLComputePipelineState> step_ = nil, group_ = nil;
    id<MTLBuffer> beff_ = nil, bsubs_ = nil, bgeom_ = nil, bseg_ = nil, bbndz_ = nil, bbndr_ = nil;
    id<MTLComputePipelineState> bound_ = nil;
    id<MTLBuffer> belems_ = nil, bmaps_ = nil, bmapd_ = nil, bstate_ = nil, bints_ = nil, bcross_ = nil, boff_ = nil, brf_ = nil,
                  bgs_ = nil, bge_ = nil, bgm_ = nil;

    static ElemVF elemf(const kern::ElemV& e) {
        ElemVF f{};
        f.kind = e.kind;
        f.fieldType = e.fieldType;
        f.mapIndex = e.mapIndex;
        f.inSuperposeOut = e.inSuperposeOut;
        f.rotMode = e.rotMode;
        f.nsub = e.nsub;
        f.sub0 = e.sub0;
        f.rfPlain = e.rfPlain;
        f.rfErr = e.rfErr;
        f.errUse = e.errUse;
        f.hetErr = e.hetErr;
        const double* src[18] = {&e.zStart, &e.length, &e.radius, &e.strength, &e.ke, &e.kb, &e.keErr, &e.kbErr, &e.phaseErr,
                                 &e.errDx, &e.errDy, &e.errDz, &e.esx, &e.ecx, &e.esy, &e.ecy, &e.esz, &e.ecz};
        float* dst[18] = {&f.zStart, &f.length, &f.radius, &f.strength, &f.ke, &f.kb, &f.keErr, &f.kbErr, &f.phaseErr,
                          &f.errDx, &f.errDy, &f.errDz, &f.esx, &f.ecx, &f.esy, &f.ecy, &f.esz, &f.ecz};
        for (int j = 0; j < 18; ++j) *dst[j] = static_cast<float>(*src[j]);
        return f;
    }

    static SubVF subf(const kern::SubV& q) {
        SubVF f{};
        f.type = q.type;
        f.map = q.map;
        f.hasErr = q.hasErr;
        f.rfErr = q.rfErr;
        f.ke = static_cast<float>(q.ke);
        f.kb = static_cast<float>(q.kb);
        f.zStart = static_cast<float>(q.zStart);
        f.radius = static_cast<float>(q.radius);
        f.length = static_cast<float>(q.length);
        for (int j = 0; j < 6; ++j) f.place[j] = static_cast<float>(q.place[j]);
        f.s3 = static_cast<float>(q.s3);
        f.c3 = static_cast<float>(q.c3);
        f.s4 = static_cast<float>(q.s4);
        f.c4 = static_cast<float>(q.c4);
        f.s5 = static_cast<float>(q.s5);
        f.c5 = static_cast<float>(q.c5);
        for (int j = 0; j < 12; ++j) f.err[j] = static_cast<float>(q.err[j]);
        f.es4 = static_cast<float>(q.es4);
        f.ec4 = static_cast<float>(q.ec4);
        f.es5 = static_cast<float>(q.es5);
        f.ec5 = static_cast<float>(q.ec5);
        f.es6 = static_cast<float>(q.es6);
        f.ec6 = static_cast<float>(q.ec6);
        f.esin6 = static_cast<float>(q.esin6);
        return f;
    }

    // The synchronous particle's Boris push of this step (sync_step of the CUDA back end).
    SyncH sync_step(const kern::StepArgs& a) {
        SyncH S{};
        const double* p = a.syncPre;
        const double* E = a.syncTot;
        const double* B = a.syncTot + 3;
        const double qm = a.qmSync, dt = a.dt;
        const double h = qm * k::half_inv_c * dt;
        const double ux = h * E[0] + p[1], uy = h * E[1] + p[3], uz = h * E[2] + p[5];
        S.hEs[0] = static_cast<float>(h * E[0]);
        S.hEs[1] = static_cast<float>(h * E[1]);
        S.hEs[2] = static_cast<float>(h * E[2]);
        S.usm[0] = static_cast<float>(ux);
        S.usm[1] = static_cast<float>(uy);
        S.usm[2] = static_cast<float>(uz);
        const double g = std::sqrt((uz * uz + uy * uy) + (ux * ux + 1.0));
        S.gsm = static_cast<float>(g);
        const double sq = (dt * qm) / g;
        const double tx = sq * B[0], ty = sq * B[1], tz = sq * B[2];
        const double tx2 = tx * tx, ty2 = ty * ty, tz2 = tz * tz;
        const double d = 1.0 / ((tx2 + ty2 + tz2) * 0.25 + 1.0);
        const double txy = ty * tx * 0.5, txz = tz * tx * 0.5, tyz = tz * ty * 0.5;
        const double K[9] = {-(ty2 + tz2) * 0.5 * d, (txy + tz) * d, (txz - ty) * d, (txy - tz) * d, -(tz2 + tx2) * 0.5 * d,
                             (tx + tyz) * d, (txz + ty) * d, (tyz - tx) * d, -(ty2 + tx2) * 0.5 * d};
        for (int j = 0; j < 9; ++j) S.Ks[j] = static_cast<float>(K[j]);
        double x = p[0], y = p[2], z = p[4], qx = p[1], qy = p[3], qz = p[5];
        boris_push(x, qx, y, qy, z, qz, E, B, qm, dt);
        S.usp[0] = static_cast<float>(qx);
        S.usp[1] = static_cast<float>(qy);
        S.usp[2] = static_cast<float>(qz);
        S.gsp = static_cast<float>(std::sqrt((qy * qy + qz * qz) + (qx * qx + 1.0)));
        S.cdt = static_cast<float>(k::c * dt);
        for (int j = 0; j < 6; ++j) S.pre[j] = p[j];
        S.post[0] = x;
        S.post[1] = qx;
        S.post[2] = y;
        S.post[3] = qy;
        S.post[4] = z;
        S.post[5] = qz;
        return S;
    }

    // fp64 arrays -> float state relative to the synchronous particle before this step
    void from64(const kern::PartV& P, int n, const SyncH& S, double dt) {
        const size_t m = static_cast<size_t>(std::max(n_, 1));
        float* f = static_cast<float*>([bstate_ contents]);
        int* iv = static_cast<int*>([bints_ contents]);
        const double* c[6] = {P.x, P.ux, P.y, P.uy, P.z, P.uz};
        charge_.assign(static_cast<size_t>(n), 0.0f);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < 3; ++j) {
                f[j * m + i] = static_cast<float>(c[2 * j][i] - S.pre[2 * j]);
                f[(3 + j) * m + i] = static_cast<float>(c[2 * j + 1][i] - S.pre[2 * j + 1]);
            }
            f[6 * m + i] = static_cast<float>(P.ex[i]);
            f[7 * m + i] = static_cast<float>(P.ey[i]);
            f[8 * m + i] = static_cast<float>(P.ez[i]);
            f[9 * m + i] = static_cast<float>(P.bx[i]);
            f[10 * m + i] = static_cast<float>(P.by[i]);
            f[11 * m + i] = static_cast<float>(P.bz[i]);
            f[12 * m + i] = static_cast<float>(P.scx[i]);
            f[13 * m + i] = static_cast<float>(P.scy[i]);
            f[14 * m + i] = static_cast<float>(P.scz[i]);
            for (int j = 0; j < 6; ++j) f[(15 + j) * m + i] = static_cast<float>(P.rel[j][i]);
            f[21 * m + i] = static_cast<float>(P.qOverM[i] * k::half_inv_c * dt);
            f[22 * m + i] = static_cast<float>(dt * P.qOverM[i]);
            f[23 * m + i] = static_cast<float>(P.weight[i]);
            f[24 * m + i] = static_cast<float>(P.spOutZ0[i]);
            f[25 * m + i] = static_cast<float>(P.spOutZ1[i]);
            iv[i] = P.lost[i];
            iv[m + i] = P.elemIndex[i];
            iv[2 * m + i] = P.nextPlane[i];
            iv[3 * m + i] = P.boundaryMode[i];
            iv[4 * m + i] = P.rec[8 * static_cast<long long>(i) + 7] == 1.0 ? 1 : 0;
            iv[5 * m + i] = P.spRec && P.spRec[8 * static_cast<long long>(i) + 7] != 0.0 ? 1 : 0;
            iv[6 * m + i] = P.geomIndex[i];
            iv[7 * m + i] = P.spOutCount[i];
            charge_[static_cast<size_t>(i)] = static_cast<float>(P.charge[i]);
        }
    }

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
        const std::uint64_t a = addr(b);
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

std::unique_ptr<ScAccel> make_f32_cpu_accel(std::string& why) {
    auto a = std::make_unique<MetalAccel>(true);
    if (!a->init(why)) return nullptr;
    return a;
}

std::unique_ptr<ScAccel> make_metal_accel(std::string& why) {
    auto a = std::make_unique<MetalAccel>();
    if (!a->init(why)) return nullptr;
    return a;
}

}  // namespace avas
