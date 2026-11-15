// Host logic of the fp32 tier (see f32_host.h).  Moved from the Metal back end (M6c phase 4).
#include "core/f32_host.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "core/errors.h"
#include "core/options.h"
#include "core/format.h"
#include "core/kernels.h"
#include "core/lattice.h"
#include "core/physics.h"
#include "core/reduce.h"
#include "core/sc_kernels.h"

namespace avas {

namespace {

// kern::MeshV of the float kernels (double -> float, pointers -> device addresses); checked against
// sizeof of the device at start-up.
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
    int rfq0, rfqN;
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
    std::uint64_t rfqCells;  // never used on the GPU (rfq runs on the CPU only)
};
struct PartFH {  // PartF of kernels.metal
    std::uint64_t d[3], du[3], E[3], B[3], sc[3], rel[6], hq, tq, w, spz0, spz1;
    std::uint64_t lost, elemIndex, nextPlane, boundaryMode, recFlag, spFlag, geomIndex, spOutCount, C, Mx, oldS, crossCount,
        crossIdx, spIdx, bdIdx;
    std::uint64_t row[8], qm;
    std::uint64_t cSnap, cNp, sSnap, sElem, aIdx, aRow, scTmp, bEnt, rm;
};
struct FftPH {  // f32::FftP
    std::uint64_t rho, A, B, C, D, sx, sy, cz, sz, lx, ly, lz, phi, ex, ey, ez;
    int pass;
    float scale, norm, hx, hy, hz;
};
struct StepUH {  // StepU of kernels.metal
    float hEs[3], usm[3], gsm, Ks[9], usp[3], gsp, cdt;
    float post0, post2, zsPost;
    float cen[6];
    float gp, zFieldPre, segEnd, zNeg, rs, rc;
    float z0, spx, tot;
    int zeroField;
    int rotMode, nEvents, writeOld, n;
    int stride;
};
struct BendStepH {  // BendStep<float>
    float s, h, kx, kx2, ky, sx, cx, sy, cy;
    int hv;
};
struct RowOpH {  // kern::RowOp with double = float
    int kind, scKick, err, hv;
    float strength, len, uzs, k1, k2;
    BendStepH bs;
    ElemVF e;
};
struct ZArgsH {  // f32::ZArgs
    RowOpH op;
    int code, kind, stride, n;
    float shift, t, zpl, uzs, gc, wx, wy, len;
    float off[6], cen[6];
};
enum : int { kZHet = 1, kZKick = 2, kZTref = 3, kZHesc1 = 4, kZHesc2 = 5, kZTot = 6, kZUbp1 = 7, kZUbp2 = 8, kZApply = 9, kZClear = 10, kZSc = 11,
             kZLp = 12, kZLong = 13 };

// The synchronous particle's push of a step.
struct SyncH {
    float hEs[3], usm[3], gsm, Ks[9], usp[3], gsp, cdt;
    double pre[6], post[6];
};

class F32Accel : public ScAccel {
public:
    explicit F32Accel(std::unique_ptr<F32Device> dev) : D_(std::move(dev)) {}

    bool init(std::string& why) {
        if (!D_->init(why)) return false;
        // layout check of the structures shared with the kernels
        unsigned s[16] = {0};  // (s[10]: ZArgs)
        D_->sizes(s);
        if (s[0] != sizeof(MeshF) || s[1] != sizeof(sck::SpGrid) || s[2] != sizeof(LatVF) || s[3] != sizeof(ElemVF) ||
            s[4] != sizeof(MapView<float, float>) || s[5] != sizeof(PartFH) || s[6] != sizeof(StepUH) || s[7] != sizeof(SubVF) ||
            s[8] != sizeof(GeomFH) || s[9] != sizeof(BoundUH) || s[10] != sizeof(ZArgsH) || s[11] != sizeof(FftPH)) {
            why = "fp32 kernel structure layout mismatch (MeshV " + std::to_string(s[0]) + " / " + std::to_string(sizeof(MeshF)) + ")";
            return false;
        }
        return true;
    }

    std::string describe() const override { return D_->describe(); }

    void deposit(const kern::MeshV& M, int n, const double* x, const double* y, const double* z, const double* q,
                 const int* lost, std::vector<long long>& acc) override {
        if (n <= 0) return;
        {
            const size_t n3 = static_cast<size_t>(M.nx) * M.ny * M.nz;
            flu(bx_, n, x);
            flu(by_, n, y);
            flu(bz_, n, z);
            flu(bq_, n, q);
            std::memcpy(buf(blost_, n * 4).host, lost, static_cast<size_t>(n) * 4);
            D_->upload(blost_, 0, static_cast<size_t>(n) * 4);
            F32Buf& lo = shared(blo_, n3 * 4);
            F32Buf& hi = shared(bhi_, n3 * 4);
            std::memset(lo.host, 0, n3 * 4);
            std::memset(hi.host, 0, n3 * 4);
            MeshF F = mesh(M);
            D_->deposit(&F, SL(bx_, 0), SL(by_, 0), SL(bz_, 0), SL(bq_, 0), SL(blost_, 0), SL(lo, 0),
                                      SL(hi, 0), n);
            const std::uint32_t* l = static_cast<const std::uint32_t*>(lo.host);
            const std::uint32_t* h = static_cast<const std::uint32_t*>(hi.host);
            for (size_t e = 0; e < n3; ++e) {
                const std::uint64_t v = (static_cast<std::uint64_t>(h[e]) << 32) | l[e];
                acc[e] = static_cast<long long>(static_cast<std::uint64_t>(acc[e]) + v);
            }
        }
    }

    // PICNIC / SPICNIC on the device (phase 5): the charge goes up in double and becomes float there, the Green
    // table and the correction list are made by the device in fp64 where it can (CUDA; else here, the same
    // numbers), and the field stays on the device for interp_state (fields() fetches it when needed).
    bool solve_mesh(const ScSolveIn& in) override {
        Timer timer{tSolve_, now()};
        if (in.method == 0) return solve_fft(in);
        if (in.method != 1 && in.method != 2) return false;
        const sck::SpGrid g{in.nx, in.ny, in.nz, in.mx, in.my, in.mz};
        const size_t n3 = static_cast<size_t>(g.n3());
        const size_t nG = static_cast<size_t>(2 * in.nx + 1) * (2 * in.ny + 1) * (2 * in.nz + 1);
        const long long nq = in.method == 1 ? 8 * g.n_targets() : 0;
        F32Buf& rd = buf(brhod_, n3 * 8);
        std::memcpy(rd.host, in.rho, n3 * 8);
        D_->upload(rd, 0, n3 * 8);
        F32Buf& rf = buf(brho_, n3 * 4);
        F32Buf& cr = buf(bcorr_, static_cast<size_t>(std::max<long long>(3 * nq, 1)) * 4);
        F32Buf& dv = buf(bdval_, static_cast<size_t>(std::max<long long>(nq, 1)) * 4);
        long long nc = 0;
        const double tc0 = now();
        if (!D_->rho_corr(&g, SL(rd, 0), SL(rf, 0), static_cast<long long>(n3), in.method == 1, in.thr, SL(cr, 0), SL(dv, 0), nc)) {
            float* f = static_cast<float*>(rf.host);
            for (size_t e = 0; e < n3; ++e) f[e] = static_cast<float>(in.rho[e]);
            D_->upload(rf, 0, n3 * 4);
            if (in.method == 1) {
                int* c = static_cast<int*>(cr.host);
                float* d = static_cast<float*>(dv.host);
                for (long long q = 0; q < nq; ++q) {  // the order of the GPU's stable compaction
                    int I, J, K;
                    const double v = sck::corr_value(g, in.rho, in.thr, q / 8, static_cast<int>(q % 8), I, J, K);
                    if (v == 0.0) continue;
                    c[3 * nc] = I;
                    c[3 * nc + 1] = J;
                    c[3 * nc + 2] = K;
                    d[nc] = static_cast<float>(v);
                    ++nc;
                }
                D_->upload(cr, 0, static_cast<size_t>(3 * nc) * 4);
                D_->upload(dv, 0, static_cast<size_t>(nc) * 4);
            }
        }
        const double tc1 = now();
        tCharge_ += tc1 - tc0;
        F32Buf& G = buf(bG_, nG * 4);
        if (!D_->green(&g, in.dx, in.dy, in.dz, in.vol, SL(G, 0))) {
            host_green(g, in, static_cast<float*>(G.host));
            D_->upload(G, 0, nG * 4);
        }
        const double tc2 = now();
        tGreen_ += tc2 - tc1;
        F32Buf& phi = buf(bphi_, n3 * 4);
        std::memset(phi.host, 0, n3 * 4);
        D_->upload(phi, 0, n3 * 4);
        const double tc3 = now();
        tPhi0_ += tc3 - tc2;
        const float h[3] = {static_cast<float>(1.0 / (in.dx + in.dx)), static_cast<float>(1.0 / (in.dy + in.dy)),
                            static_cast<float>(1.0 / (in.dz + in.dz))};
        F32Buf& fe = buf(bfield_, 3 * n3 * 4);
        D_->batch_begin();  // potential and field: one submission
        if (in.method == 2) {
            D_->picnic(&g, SL(rf, 0), SL(G, 0), SL(phi, 0));
        } else {
            D_->sym(&g, SL(rf, 0), SL(G, 0), static_cast<float>(in.thr), SL(phi, 0));
            if (nc > 0) D_->corr(&g, SL(G, 0), SL(cr, 0), SL(dv, 0), nc, SL(phi, 0));
        }
        D_->grad(&g, SL(phi, 0), h, SL(fe, 0), SL(fe, n3 * 4), SL(fe, 2 * n3 * 4));
        D_->batch_end();
        tKern_ += now() - tc3;
        fieldN3_ = n3;
        ++nSolve_;
        return true;
    }

    // FFT on the device (phase 5): the direct transforms of the device back end in float, passes 0 .. 9 of
    // f32::fft_node; the tables come from the host (sck::fft_tables, lx / ly / lz of the solve)
    bool solve_fft(const ScSolveIn& in) {
        const sck::SpGrid g{in.nx, in.ny, in.nz, in.mx, in.my, in.mz};
        const size_t n3 = static_cast<size_t>(g.n3());
        F32Buf& rd = buf(brhod_, n3 * 8);
        std::memcpy(rd.host, in.rho, n3 * 8);
        D_->upload(rd, 0, n3 * 8);
        F32Buf& rf = buf(brho_, n3 * 4);
        long long nc = 0;
        if (!D_->rho_corr(&g, SL(rd, 0), SL(rf, 0), static_cast<long long>(n3), false, 0.0, SL(rf, 0), SL(rf, 0), nc)) {
            float* f = static_cast<float*>(rf.host);
            for (size_t e = 0; e < n3; ++e) f[e] = static_cast<float>(in.rho[e]);
            D_->upload(rf, 0, n3 * 4);
        }
        const size_t nsx = 2 * static_cast<size_t>(in.nx + 1), nsy = 2 * static_cast<size_t>(in.ny + 1), nz = static_cast<size_t>(in.nz);
        F32Buf& tb = buf(bfft_, (nsx + nsy + 2 * nz + in.nx + in.ny + in.nz) * 4);
        float* t = static_cast<float*>(tb.host);
        if (fftKey_[0] != in.nx || fftKey_[1] != in.ny || fftKey_[2] != in.nz) {
            const sck::FftTables T = sck::fft_tables(in.nx, in.ny, in.nz);
            for (size_t q = 0; q < nsx; ++q) t[q] = static_cast<float>(T.sx[q]);
            for (size_t q = 0; q < nsy; ++q) t[nsx + q] = static_cast<float>(T.sy[q]);
            for (size_t q = 0; q < nz; ++q) {
                t[nsx + nsy + q] = static_cast<float>(T.cz[q]);
                t[nsx + nsy + nz + q] = static_cast<float>(T.sz[q]);
            }
            D_->tables(tb);
            D_->upload(tb, 0, (nsx + nsy + 2 * nz) * 4);
            fftKey_[0] = in.nx;
            fftKey_[1] = in.ny;
            fftKey_[2] = in.nz;
        }
        const size_t lo = nsx + nsy + 2 * nz;
        for (int q = 0; q < in.nx; ++q) t[lo + q] = static_cast<float>(in.lx[q]);
        for (int q = 0; q < in.ny; ++q) t[lo + in.nx + q] = static_cast<float>(in.ly[q]);
        for (int q = 0; q < in.nz; ++q) t[lo + in.nx + in.ny + q] = static_cast<float>(in.lz[q]);
        D_->upload(tb, lo * 4, static_cast<size_t>(in.nx + in.ny + in.nz) * 4);
        F32Buf& w = buf(bwork_, 4 * n3 * 4);
        F32Buf& phi = buf(bphi_, n3 * 4);
        F32Buf& fe = buf(bfield_, 3 * n3 * 4);
        FftPH P{};
        const std::uint64_t td = tb.dev;
        P.rho = rf.dev;
        P.A = w.dev;
        P.B = w.dev + n3 * 4;
        P.C = w.dev + 2 * n3 * 4;
        P.D = w.dev + 3 * n3 * 4;
        P.sx = td;
        P.sy = td + nsx * 4;
        P.cz = td + (nsx + nsy) * 4;
        P.sz = td + (nsx + nsy + nz) * 4;
        P.lx = td + lo * 4;
        P.ly = td + (lo + in.nx) * 4;
        P.lz = td + (lo + in.nx + in.ny) * 4;
        P.phi = phi.dev;
        P.ex = fe.dev;
        P.ey = fe.dev + n3 * 4;
        P.ez = fe.dev + 2 * n3 * 4;
        P.scale = static_cast<float>(in.scale);
        P.norm = static_cast<float>(in.norm);
        P.hx = static_cast<float>(1.0 / (in.dx + in.dx));
        P.hy = static_cast<float>(1.0 / (in.dy + in.dy));
        P.hz = static_cast<float>(1.0 / (in.dz + in.dz));
        D_->batch_begin();  // the ten passes: one submission
        for (int pass = 0; pass <= 9; ++pass) {
            P.pass = pass;
            if (!D_->fft_lines(&g, &P, pass)) D_->fft(&g, &P, static_cast<long long>(n3));
        }
        D_->batch_end();
        fieldN3_ = n3;
        ++nSolve_;
        return true;
    }

    // lpboundary / longlimits on the device (phase 5), relative to the synchronous particle zs
    bool lp_fold(double zs, double full, double half) override {
        if (!auth_) return false;
        ZArgsH A = zargs(kZLp);
        A.off[4] = static_cast<float>(ref_.post[4] - zs);
        A.wx = static_cast<float>(full);
        A.wy = static_cast<float>(half);
        D_->zrow(&A, &pf_, n_);
        hostCurrent_ = false;
        ++nLp_;
        return true;
    }
    bool long_loss(double zs, double ws, double dW, double dzMax) override {
        if (!auth_) return false;
        ZArgsH A = zargs(kZLong);
        A.off[4] = static_cast<float>(ref_.post[4] - zs);
        for (int j = 0; j < 3; ++j) A.cen[2 * j + 1] = static_cast<float>(ref_.post[2 * j + 1]);
        A.t = static_cast<float>(ws);
        A.wx = static_cast<float>(dW);
        A.wy = static_cast<float>(dzMax);
        D_->zrow(&A, &pf_, n_);
        hostCurrent_ = false;
        ++nLp_;
        return true;
    }

    // the field of the last solve_mesh on the host (space-charge records, the interpolation of fp64 arrays)
    void fields(std::vector<double>& ex, std::vector<double>& ey, std::vector<double>& ez) override {
        const size_t n3 = fieldN3_;
        D_->download(bfield_, 0, 3 * n3 * 4);
        const float* f = static_cast<const float*>(bfield_.host);
        ex.assign(f, f + n3);
        ey.assign(f + n3, f + 2 * n3);
        ez.assign(f + 2 * n3, f + 3 * n3);
    }

    void interp(const kern::MeshV& M, const double* ex, const double* ey, const double* ez, int n, const double* x,
                const double* y, const double* z, const int* lost, double* scx, double* scy, double* scz) override {
        if (n <= 0) return;
        {
            const size_t n3 = static_cast<size_t>(M.nx) * M.ny * M.nz;
            flu(bx_, n, x);
            flu(by_, n, y);
            flu(bz_, n, z);
            std::memcpy(buf(blost_, n * 4).host, lost, static_cast<size_t>(n) * 4);
            D_->upload(blost_, 0, static_cast<size_t>(n) * 4);
            F32Buf& fe = field_up(ex, ey, ez, n3);
            F32Buf& sc = buf(bsc_, static_cast<size_t>(n) * 12);
            MeshF F = mesh(M);
            D_->interp(&F, SL(bx_, 0), SL(by_, 0), SL(bz_, 0), SL(blost_, 0), SL(fe, 0), SL(fe, n3 * 4),
                                     SL(fe, 2 * n3 * 4), SL(sc, 0), n);
            D_->download(sc, 0, static_cast<size_t>(n) * 12);
            const float* s = static_cast<const float*>(sc.host);
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
        hostCurrent_ = false;
        rowsValid_ = true;  // rows and records start empty
        recStale_ = false;
        rowsT0Set_ = false;
        T0_ = 0.0;
        pend_ = 0.0f;
        n_ = n;
        stepOk_ = true;
        why_.clear();
        {
            set_elems(L);
            // field maps in float (the same conversion as the CUDA fp32 tier)
            size_t tot = 0;
            for (const MapView<double>& m : L.maps) tot += static_cast<size_t>(m.nx + 1) * (m.ny + 1) * (m.nz + 1) * m.ncomp;
            F32Buf& md = buf(bmapd_, std::max<size_t>(tot, 1) * 4);
            float* f = static_cast<float*>(md.host);
            MapView<float, float>* mv =
                static_cast<MapView<float, float>*>(buf(bmaps_, std::max<size_t>(L.maps.size(), 1) * sizeof(MapView<float, float>)).host);
            size_t o = 0;
            for (size_t q = 0; q < L.maps.size(); ++q) {
                const MapView<double>& m = L.maps[q];
                const size_t cnt = static_cast<size_t>(m.nx + 1) * (m.ny + 1) * (m.nz + 1) * m.ncomp;
                for (size_t e = 0; e < cnt; ++e) f[o + e] = static_cast<float>(m.data[e]);
                MapView<float, float> g{};
                g.data = reinterpret_cast<const float*>(md.dev + o * 4);
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
            D_->tables(bmapd_);
            D_->tables(bmaps_);
            D_->upload(bmapd_, 0, tot * 4);
            D_->upload(bmaps_, 0, L.maps.size() * sizeof(MapView<float, float>));
            latZ_ = &L;
            // particle state: one shared buffer, slices
            // column length n (a length padded to whole reduction groups puts the ~80 columns of a particle
            // 16 KiB apart, in the same cache sets: the step kernel was 25 % slower on the M2)
            m_ = static_cast<size_t>(std::max(n, 1));
            const size_t m = m_;
            const size_t floats = 26 * m + 21 * m + 8 * m + 6 * m + 8 * m + m + m;
            buf(bstate_, floats * 4);
            buf(bints_, 8 * m * 4);
            D_->particles(bstate_, m * 4, n);
            D_->particles(bints_, m * 4, n);
            shared(bcross_, (3 * m + 4) * 4);
            const std::uint64_t fa = bstate_.dev, ia = bints_.dev, ca = bcross_.dev;
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
            P.oldS = fs(26 + 21 + 8);
            for (int j = 0; j < 8; ++j) P.row[j] = fs(kRow + j);
            P.qm = fs(kRow + 8);
            P.rm = fs(kRow + 9);
            // compact candidate entries and the host's results (capacity: every particle)
            shared(bcand_, m * kCand * 4);
            const std::uint64_t ka = bcand_.dev;
            P.cSnap = ka;
            P.cNp = ka + 12 * m * 4;
            P.sSnap = ka + 13 * m * 4;
            P.sElem = ka + 25 * m * 4;
            P.aIdx = ka + 26 * m * 4;  // 2 m entries: a step can record a plane and a superposeout exit
            P.aRow = ka + 28 * m * 4;
            P.bEnt = ka + 44 * m * 4;
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
    void invalidate_rows() override { rowsValid_ = false; }
    void rec_cleared(bool sp) override {
        if (n_ > 0 && bints_.host) {  // on the device: the flags (and the row flags) stay where they are
            ZArgsH A = zargs(kZClear);
            A.kind = sp ? 1 : 0;
            D_->zrow(&A, &pf_, n_);
        }
        if (!sp && n_ > 0 && bstate_.host) {
            recStale_ = false;
            rowsT0Set_ = false;
            pend_ = 0.0f;
        }
    }

    // element tables (also when the superposeout beam frame changes rotMode)
    void set_elems(const LatHost& L) override {
        {
            const size_t nE = L.elems.size();
            ElemVF* ev = static_cast<ElemVF*>(buf(belems_, std::max<size_t>(nE, 1) * sizeof(ElemVF)).host);
            for (size_t k = 0; k < nE; ++k) ev[k] = elemf(L.elems[k]);
            ElemVF* fv = static_cast<ElemVF*>(buf(beff_, std::max<size_t>(L.eff.size(), 1) * sizeof(ElemVF)).host);
            for (size_t k = 0; k < L.eff.size(); ++k) fv[k] = elemf(L.eff[k]);
            SubVF* sv = static_cast<SubVF*>(buf(bsubs_, std::max<size_t>(L.subs.size(), 1) * sizeof(SubVF)).host);
            for (size_t q = 0; q < L.subs.size(); ++q) sv[q] = subf(L.subs[q]);
            hasEff_ = !L.eff.empty();
            errOn_ = L.errOn;
            GeomFH* gv = static_cast<GeomFH*>(buf(bgeom_, std::max<size_t>(L.geoms.size(), 1) * sizeof(GeomFH)).host);
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
            float* sg = static_cast<float*>(buf(bseg_, std::max<size_t>(2 * ns, 1) * 4).host);
            for (size_t q = 0; q < ns; ++q) {
                sg[q] = static_cast<float>(L.segStart[q]);
                sg[ns + q] = static_cast<float>(L.segEnd[q]);
            }
            const size_t nb = L.bndR1.size();
            float* bz = static_cast<float*>(buf(bbndz_, std::max<size_t>(L.bndZ.size(), 1) * 4).host);
            for (size_t q = 0; q < L.bndZ.size(); ++q) bz[q] = static_cast<float>(L.bndZ[q]);
            int* br = static_cast<int*>(buf(bbndr_, std::max<size_t>(2 * nb, 1) * 4).host);
            for (size_t q = 0; q < nb; ++q) {
                br[q] = L.bndR1[q];
                br[nb + q] = L.bndR2[q];
            }
            for (F32Buf* b : {&belems_, &beff_, &bsubs_, &bgeom_, &bseg_, &bbndz_, &bbndr_}) {
                D_->tables(*b);
                D_->upload(*b, 0, b->len);
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
        ensure_rows(P, n);
        if (auth_) {
            // the float state is relative to the synchronous particle of the last step (or of zTotcode)
            bool same = true;
            for (int j = 0; j < 6; ++j) same = same && ref_.post[j] == a.syncPre[j];
            if (!same) {
                to_host(P, n);
                auth_ = false;
                ++nRebase_;
            }
        }
        hostCurrent_ = false;
        {
            const SyncH S = sync_step(a);
            if (!auth_) {
                const double tc = now();
                from64(P, n, S, a.dt);
                D_->upload(bstate_, 0, bstate_.len);
                D_->upload(bints_, 0, bints_.len);
                tConv_ += now() - tc;
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
            std::memcpy(buf(boff_, o.size() * 4).host, o.data(), o.size() * 4);
            D_->tables(boff_);
            D_->upload(boff_, 0, o.size() * 4);
            float* rt = static_cast<float*>(buf(brf_, std::max<size_t>(rf.size(), 1) * 4).host);
            for (size_t q = 0; q < rf.size(); ++q) rt[q] = static_cast<float>(rf[q]);
            D_->tables(brf_);
            D_->upload(brf_, 0, rf.size() * 4);
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
            U.stride = static_cast<int>(m_);
            LatVF LV{};
            LV.elems = belems_.dev;
            LV.eff = hasEff_ ? beff_.dev : belems_.dev;
            LV.subs = bsubs_.dev;
            LV.mapsF = bmaps_.dev;
            LV.nElem = static_cast<int>(nE);
            LV.errOn = errOn_ ? 1 : 0;
            LV.spOutCur = ls.spOutCur;
            LV.spOutCount = ls.spOutCount;
            LV.spOutSize = ls.spOutSize;
            LV.totalLength = static_cast<float>(ls.totalLength);
            LV.spExit = static_cast<float>(ls.spExit);
            for (int c = 0; c < 4; ++c) static_cast<std::uint32_t*>(bcross_.host)[c] = 0;
            const int ng = red_groups(n);
            F32Buf& gs = shared(bgs_, static_cast<size_t>(ng) * 21 * 8);
            F32Buf& ge = shared(bge_, static_cast<size_t>(ng) * 21 * 4);
            F32Buf& gm = shared(bgm_, static_cast<size_t>(ng) * 8 * 4);
            D_->batch_begin();  // the push and its group sums: one submission (the host reads only the sums)
            D_->step(&LV, &U, &pf_, SL(boff_, 0), SL(brf_, 0), sizes, n);
            t1 = now();
            // statistics per reduction group (fixed point on the GPU, merged in double here)
            D_->group(SL(bstate_, 26 * m_ * 4), SL(bstate_, (26 + 21) * m_ * 4), SL(gs, 0), SL(ge, 0), SL(gm, 0), n, static_cast<int>(m_),
                      21, 8, 6);
            D_->batch_end();
            t2 = now();
            const std::int64_t* sv = static_cast<const std::int64_t*>(gs.host);
            const std::int32_t* ex = static_cast<const std::int32_t*>(ge.host);
            const float* mx = static_cast<const float*>(gm.host);
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
            const std::uint32_t nc = *static_cast<const std::uint32_t*>(bcross_.host);
            const std::uint32_t* idx = static_cast<const std::uint32_t*>(bcross_.host) + 4;
            char* cb = static_cast<char*>(bcand_.host);
            const float* cSnap = reinterpret_cast<const float*>(cb);
            const int* cNp = reinterpret_cast<const int*>(cb + 12 * m_ * 4);
            const float* sSnap = reinterpret_cast<const float*>(cb + 13 * m_ * 4);
            const int* sElem = reinterpret_cast<const int*>(cb + 25 * m_ * 4);
            int* aIdx = reinterpret_cast<int*>(cb + 26 * m_ * 4);
            float* aRow = reinterpret_cast<float*>(cb + 28 * m_ * 4);
            // pass 1 (independent candidates, in parallel when there are many): the record in double
            std::vector<unsigned char>& ok = okBuf_;
            ok.assign(nc, 0);
            auto complete = [&](long long c) {
                const int i = static_cast<int>(idx[c]);
                const float* sn = cSnap + 12 * static_cast<size_t>(c);
                double cur[6], old[6];
                for (int j = 0; j < 3; ++j) {
                    cur[2 * j] = S.post[2 * j] + static_cast<double>(sn[6 + j]);
                    cur[2 * j + 1] = S.post[2 * j + 1] + static_cast<double>(sn[9 + j]);
                    old[2 * j] = S.pre[2 * j] + static_cast<double>(sn[j]);
                    old[2 * j + 1] = S.pre[2 * j + 1] + static_cast<double>(sn[3 + j]);
                }
                const double pl = events[static_cast<size_t>(cNp[c])];
                if (!(pl <= cur[4])) return;
                const double r = (pl - old[4]) / (cur[4] - old[4]);
                double* rec = P.rec + 8 * static_cast<long long>(i);
                for (int j = 0; j < 6; ++j) rec[j] = (cur[j] - old[j]) * r + old[j];
                kern::interp_momentum_norm(rec, old, cur, r);
                rec[6] = (r - 1.0) * a.dt + a.tEnd;
                ok[c] = rec[7] != 1.0 ? 2 : 1;  // 2: a new record
                rec[7] = 1.0;
            };
            if (nc >= 16384) par(nc, complete);
            else for (std::uint32_t c = 0; c < nc; ++c) complete(c);
            // record flag, next plane and the z-code row in float (time relative to T0, the end of the first
            // recording step) are written on the device (kZApply); slot c, -1 where nothing is recorded
            bool any = false;
            for (std::uint32_t c = 0; c < nc; ++c) {
                if (!ok[c]) continue;
                any = true;
                if (ok[c] == 2) out[static_cast<size_t>(idx[c] / kRedGroup)].rec += 1.0;
            }
            if (any && !rowsT0Set_) {
                T0_ = a.tEnd;
                rowsT0Set_ = true;
            }
            auto row = [&](long long c) {
                const int i = static_cast<int>(idx[c]);
                if (!ok[c]) {
                    aIdx[c] = -1;
                    return;
                }
                const double* rec = P.rec + 8 * static_cast<long long>(i);
                aIdx[c] = 4 * i;
                for (int j = 0; j < 8; ++j) aRow[8 * c + j] = static_cast<float>(j == 6 ? rec[6] - T0_ : rec[j]);
            };
            if (nc >= 16384) par(nc, row);
            else for (std::uint32_t c = 0; c < nc; ++c) row(c);
            int nApply = any ? static_cast<int>(nc) : 0;
            // superposeout exits: the candidates of the GPU decided and recorded in double
            const std::uint32_t ns = static_cast<const std::uint32_t*>(bcross_.host)[1];
            const std::uint32_t* sidx = static_cast<const std::uint32_t*>(bcross_.host) + 4 + m_;
            for (std::uint32_t c = 0; c < ns; ++c) {
                const int i = static_cast<int>(sidx[c]);
                const float* sn = sSnap + 12 * static_cast<size_t>(c);
                double cur[6], old[6];
                for (int j = 0; j < 3; ++j) {
                    cur[2 * j] = S.post[2 * j] + static_cast<double>(sn[6 + j]);
                    cur[2 * j + 1] = S.post[2 * j + 1] + static_cast<double>(sn[9 + j]);
                    old[2 * j] = S.pre[2 * j] + static_cast<double>(sn[j]);
                    old[2 * j + 1] = S.pre[2 * j + 1] + static_cast<double>(sn[3 + j]);
                }
                if (!(cur[4] > P.spOutZ0[i]) || P.spRec[8 * static_cast<long long>(i) + 7] != 0.0) continue;
                const kern::SpOutV& row = L.spOut[static_cast<size_t>(ls.spOutCur)];
                const double zst = L.elems[static_cast<size_t>(sElem[c])].zStart;
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
                    aIdx[nApply++] = 4 * i + 1;
                    out[static_cast<size_t>(i / kRedGroup)].sp += 1.0;
                }
            }
            if (nApply > 0) {
                ZArgsH A = zargs(kZApply);
                A.n = nApply;
                D_->zrow(&A, &pf_, nApply);
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
        if (!auth_ || hostCurrent_) return;
        ++nTo_;
        struct T { double& acc; double t0; ~T() { acc += now() - t0; } } timer{tConv_, now()};
        D_->download(bstate_, 0, bstate_.len);
        D_->download(bints_, 0, bints_.len);
        const size_t m = m_;
        const float* f = static_cast<const float*>(bstate_.host);
        const int* iv = static_cast<const int*>(bints_.host);
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
            const float* os = f + (26 + 21 + 8) * m;
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < 6; ++j) P.old[6 * static_cast<long long>(i) + j] = oldPre_[j] + static_cast<double>(os[6 * i + j]);
        }
        hostCurrent_ = true;  // the float state stays the current one (invalidate() when the host changes P)
    }

    // ---------------- z-code rows (phase 4) ----------------
    // float rows -> fp64 records (after z-code work on the device)
    void rec_to_host(const kern::PartV& P, int n) override {
        if (!recStale_ || !P.rec) return;
        ++nRecTo_;
        D_->download(bstate_, kRow * m_ * 4, 8 * m_ * 4);
        const float* rw = static_cast<const float*>(bstate_.host) + kRow * m_;
        for (int i = 0; i < n; ++i) {
            double* r = P.rec + 8 * static_cast<long long>(i);
            for (int j = 0; j < 8; ++j) r[j] = static_cast<double>(rw[j * m_ + static_cast<size_t>(i)]);
            r[6] = T0_ + r[6];
        }
        recStale_ = false;
    }

    bool rows(const kern::RowOp& op, const kern::PartV& P, int n) override {
        if (!zOn_ || !auth_ || n != n_) return false;
        ensure_rows(P, n);
        ZArgsH A = zargs(kZHet);
        A.op = rowop(op);
        zcall(A);
        if (op.kind != kern::kEdge && op.uzs > 0.0) {
            // dt = len / vz of every row is computed with c^2 and the length rounded to float, the same
            // relative error b for every particle (about 4e-8, 1e-16 s per element, below the float
            // resolution of the row times): the time origin takes it instead (the synchronous row
            // is fp64 on the host).  b dt with the synchronous particle's dt (on axis).
            const double len = op.kind == kern::kBend ? op.bs.s : op.len;
            const double lf = static_cast<double>(static_cast<float>(len));
            const double c2f = static_cast<double>(static_cast<float>(k::c2));
            const double b = (lf / len) * std::sqrt(k::c2 / c2f) - 1.0;
            const double dts = len * std::sqrt(1.0 + op.uzs * op.uzs) / (k::c * op.uzs);
            if (len != 0.0) T0_ = T0_ - b * dts;
        }
        recStale_ = true;
        ++nRows_;
        return true;
    }

    bool rows_tref(bool average, const kern::PartV& P, int n, double& tRef) override {
        if (!zOn_ || !auth_ || n != n_) return false;
        ensure_rows(P, n);
        ZArgsH A = zargs(kZTref);
        zcall(A);
        double s[2];
        float mx[1];
        gsum(2, 1, 0, s, mx);
        if (average) tRef = T0_ + s[0] / s[1];
        else tRef = std::isinf(mx[0]) ? 1e10 : T0_ + static_cast<double>(-mx[0]);
        // the rows move to the reference time with the next transfer (exact: the times are close to it)
        const double sh = tRef - T0_;
        pend_ = std::isfinite(sh) && std::fabs(sh) < 1e-3 ? static_cast<float>(sh) : 0.0f;
        ++nRows_;
        return true;
    }

    bool rows_kick(int kind, double wx, double wy, double len, const kern::PartV& P, int n) override {
        if (!zOn_ || !auth_ || n != n_) return false;
        ensure_rows(P, n);
        ZArgsH A = zargs(kZKick);
        A.kind = kind;
        A.wx = static_cast<float>(wx);
        A.wy = static_cast<float>(wy);
        A.len = static_cast<float>(len);
        zcall(A);
        recStale_ = true;
        ++nRows_;
        return true;
    }

    bool hesc1(double tRef, const kern::PartV& P, int n, double* sums) override {
        if (!zOn_ || !auth_ || n != n_) return false;
        ensure_rows(P, n);
        ZArgsH A = zargs(kZHesc1);
        A.t = static_cast<float>(tRef - (T0_ + static_cast<double>(pend_)));
        zcall(A);
        gsum(7, 0, 0, sums, nullptr);
        hostCurrent_ = false;
        ++nRows_;
        return true;
    }

    bool hesc2(const double* cen, double gc, double* q3) override {
        if (!zOn_ || !auth_) return false;
        ZArgsH A = zargs(kZHesc2);
        for (int j = 0; j < 6; ++j) A.cen[j] = static_cast<float>(cen[j]);
        A.gc = static_cast<float>(gc);
        D_->zrow(&A, &pf_, n_);
        gsum(3, 0, 0, q3, nullptr);
        ++nRows_;
        return true;
    }

    // zTotcode on the device: the float state becomes relative to the new synchronous particle sync
    bool ztot(double zPlane, double tRef, const double* sync, const kern::PartV& P, int n) override {
        if (!zOn_ || !auth_ || n != n_ || !sync) return false;
        ensure_rows(P, n);
        ZArgsH A = zargs(kZTot);
        A.zpl = static_cast<float>(zPlane - sync[4]);
        A.t = static_cast<float>(tRef - (T0_ + static_cast<double>(pend_)));
        A.uzs = static_cast<float>(sync[5]);
        A.cen[4] = static_cast<float>((zPlane - sync[4]) - static_cast<double>(A.zpl));
        A.cen[5] = static_cast<float>(sync[5] - static_cast<double>(A.uzs));
        for (int j = 0; j < 6; ++j) A.off[j] = static_cast<float>(ref_.post[j] - sync[j]);
        zcall(A);
        for (int j = 0; j < 6; ++j) ref_.post[j] = sync[j];
        if (P.rec) std::memset(P.rec, 0, static_cast<size_t>(n) * 8 * sizeof(double));
        recStale_ = false;
        rowsValid_ = true;
        rowsT0Set_ = false;
        pend_ = 0.0f;
        hostCurrent_ = false;
        ++nZtot_;
        return true;
    }

    bool ubp1(const double* s6, double* sums) override {
        if (!zOn_ || !auth_) return false;
        ZArgsH A = zargs(kZUbp1);
        for (int j = 0; j < 3; ++j) {
            A.off[2 * j] = static_cast<float>(ref_.post[2 * j] - s6[2 * j]);
            A.cen[2 * j + 1] = static_cast<float>(ref_.post[2 * j + 1]);
        }
        D_->zrow(&A, &pf_, n_);
        gsum(9, 0, 0, sums, nullptr);
        hostCurrent_ = false;
        ++nUbp_;
        return true;
    }

    bool ubp2(const double* cen, double igz, double* q3, double& zmin, double& zmax) override {
        if (!zOn_ || !auth_) return false;
        ZArgsH A = zargs(kZUbp2);
        for (int j = 0; j < 6; ++j) A.cen[j] = static_cast<float>(cen[j]);
        A.gc = static_cast<float>(igz);
        D_->zrow(&A, &pf_, n_);
        float mx[2];
        gsum(3, 2, 0, q3, mx);
        zmax = ref_.post[4] + static_cast<double>(mx[0]);
        zmin = ref_.post[4] - static_cast<double>(mx[1]);
        return true;
    }

    // boundary.txt: float pre-selection on the GPU (k_bound), the candidates in double here with the
    // shared kern::boundary_particle (the same decisions and wall coordinates as the CPU engine).
    bool boundary(const LatScalars& ls, double t, double dt, std::vector<BoundaryHit>& hits) override {
        if (!auth_ || !latZ_ || latZ_->segEnd.empty()) return false;
        hostCurrent_ = false;
        {
            const LatHost& L = *latZ_;
            const size_t m = m_;
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
            static_cast<std::uint32_t*>(bcross_.host)[2] = 0;
            const size_t ns = L.segStart.size(), nb = L.bndR1.size();
            D_->bound(&U, &pf_, SL(bgeom_, 0), SL(bseg_, 0), SL(bseg_, ns * 4), SL(bbndz_, 0),
                                    SL(bbndr_, 0), SL(bbndr_, nb * 4), n_);
            const std::uint32_t nc = static_cast<const std::uint32_t*>(bcross_.host)[2];
            const std::uint32_t* bidx = static_cast<const std::uint32_t*>(bcross_.host) + 4 + 2 * m;
            std::vector<std::pair<std::uint32_t, std::uint32_t>> cand(nc);  // (particle, slot of its entry)
            for (std::uint32_t c = 0; c < nc; ++c) cand[c] = {bidx[c], c};
            std::sort(cand.begin(), cand.end());  // hits in particle order, as on the CPU
            const float* bEnt = reinterpret_cast<const float*>(static_cast<const char*>(bcand_.host) + 44 * m * 4);
            int* aIdx = reinterpret_cast<int*>(static_cast<char*>(bcand_.host) + 26 * m * 4);
            float* aRow = reinterpret_cast<float*>(static_cast<char*>(bcand_.host) + 28 * m * 4);
            int nApply = 0;
            kern::LatV LV = L.view(Lattice());
            LV.spOutCur = ls.spOutCur;
            LV.spOutCount = ls.spOutCount;
            LV.spOutSize = ls.spOutSize;
            LV.totalLength = ls.totalLength;
            LV.spExit = ls.spExit;
            hits.clear();
            const kern::PartV& PH = hostP_;
            for (const auto& cs : cand) {
                const size_t i = cs.first;
                const float* e = bEnt + 16 * static_cast<size_t>(cs.second);  // d, du, old, lost, spOutCount, geomIndex
                double x[6], old[6], spz0 = PH.spOutZ0[i];
                int lost = static_cast<int>(e[12]), spc = static_cast<int>(e[13]), gi = static_cast<int>(e[14]);
                for (int j = 0; j < 3; ++j) {
                    x[2 * j] = ref_.post[2 * j] + static_cast<double>(e[j]);
                    x[2 * j + 1] = ref_.post[2 * j + 1] + static_cast<double>(e[3 + j]);
                }
                const float* os = e + 6;
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
                // geometry index and loss back on the device (kZApply)
                aIdx[nApply] = 4 * static_cast<int>(i) + 2;
                aRow[8 * nApply] = static_cast<float>(gi);
                aRow[8 * nApply + 1] = static_cast<float>(code == 1 ? 1 : lost);
                ++nApply;
                if (code == 0) continue;
                h.i = static_cast<int>(i);
                h.code = code;
                hits.push_back(h);
            }
            if (nApply > 0) {
                ZArgsH A = zargs(kZApply);
                A.n = nApply;
                D_->zrow(&A, &pf_, nApply);
            }
            ++nBound_;
            nCand_ += nc;
        }
        return true;
    }

    void deposit_state(const kern::MeshV& M, int n, std::vector<long long>& acc) override {
        Timer timer{tDep_, now()};
        ++nDep_;
        {
            const size_t m = m_;
            const size_t n3 = static_cast<size_t>(M.nx) * M.ny * M.nz;
            if (qDirty_ || !bqs_.host) {  // the charges change only with the fp64 arrays (from64)
                float* q = static_cast<float*>(buf(bqs_, static_cast<size_t>(n) * 4).host);
                for (int i = 0; i < n; ++i) q[i] = charge_[static_cast<size_t>(i)];
                D_->upload(bqs_, 0, static_cast<size_t>(n) * 4);
                D_->particles(bqs_, static_cast<size_t>(n) * 4, n);
                qDirty_ = false;
            }
            F32Buf& lo = shared(blo_, n3 * 4);
            F32Buf& hi = shared(bhi_, n3 * 4);
            std::memset(lo.host, 0, n3 * 4);
            std::memset(hi.host, 0, n3 * 4);
            MeshF F = mesh(M);
            D_->deposit(&F, SL(bstate_, 15 * m * 4), SL(bstate_, 17 * m * 4), SL(bstate_, 19 * m * 4),
                                      SL(bqs_, 0), SL(bints_, 0), SL(lo, 0), SL(hi, 0), n);
            const std::uint32_t* l = static_cast<const std::uint32_t*>(lo.host);
            const std::uint32_t* h = static_cast<const std::uint32_t*>(hi.host);
            for (size_t e = 0; e < n3; ++e) {
                const std::uint64_t v = (static_cast<std::uint64_t>(h[e]) << 32) | l[e];
                acc[e] = static_cast<long long>(static_cast<std::uint64_t>(acc[e]) + v);
            }
        }
    }

    void batch(bool open) override {
        if (open) D_->batch_begin();
        else D_->batch_end();
    }

    void interp_state(const kern::MeshV& M, const double* ex, const double* ey, const double* ez, int n) override {
        Timer timer{tInterp_, now()};
        hostCurrent_ = false;
        {
            const size_t m = m_;
            const size_t n3 = static_cast<size_t>(M.nx) * M.ny * M.nz;
            F32Buf& fe = field_up(ex, ey, ez, n3);  // ex null: the field of solve_mesh, already on the device
            F32Buf& sc = buf(bsc_, static_cast<size_t>(n) * 12);
            if (scHinted_ != sc.dev) {  // per particle: on the GPU of the particle
                D_->particles(sc, static_cast<size_t>(n) * 12, n, 12);
                scHinted_ = sc.dev;
            }
            MeshF F = mesh(M);
            D_->batch_begin();  // interpolation and the write into the particles: one submission
            D_->interp(&F, SL(bstate_, 15 * m * 4), SL(bstate_, 17 * m * 4), SL(bstate_, 19 * m * 4),
                                     SL(bints_, 0), SL(fe, 0), SL(fe, n3 * 4), SL(fe, 2 * n3 * 4), SL(sc, 0), n);
            // into the state columns on the device (the host does not touch the particle pages)
            pf_.scTmp = sc.dev;
            ZArgsH A = zargs(kZSc);
            D_->zrow(&A, &pf_, n_);
            D_->batch_end();
        }
    }

    ~F32Accel() override {
        if (prof_ && nStep_ > 0)
            std::fprintf(stderr, "[fp32 profile] %lld steps: step kernel %.3f ms, group sums %.3f ms, host %.3f ms per step; "
                         "fp64->fp32 %lld, fp32->fp64 %lld conversions; boundary.txt %lld calls on the GPU, %lld candidates\n", nStep_,
                         1e3 * tStep_ / nStep_, 1e3 * tGroup_ / nStep_, 1e3 * tHost_ / nStep_, nFrom_, nTo_, nBound_, nCand_);
        if (prof_ && (nStep_ > 0 || nRows_ > 0))
            std::fprintf(stderr, "[fp32 profile] z-code on the GPU: %lld row operations, %lld zTotcode, %lld beam-parameter "
                         "passes; %lld row uploads (fp64 records -> float rows), %lld record downloads; %lld synchronous "
                         "rebases; %.3f s in row kernels and their sums, %.3f s in fp32<->fp64 conversions; %lld mesh solves, "
                         "%lld lpboundary / longlimits passes on the GPU\n", nRows_, nZtot_, nUbp_, nRowsConv_, nRecTo_, nRebase_, tZ_,
                         tConv_, nSolve_, nLp_);
        if (prof_ && nDep_ > 0)
            std::fprintf(stderr, "[fp32 profile] space charge: %lld deposits, %.3f ms per deposit, %.3f ms per mesh solve (charge and "
                         "correction list %.3f ms, Green table %.3f ms, zero potential %.3f ms, potential %.3f ms), %.3f ms per field interpolation\n",
                         nDep_, 1e3 * tDep_ / nDep_, nSolve_ ? 1e3 * tSolve_ / nSolve_ : 0.0, nSolve_ ? 1e3 * tCharge_ / nSolve_ : 0.0,
                         nSolve_ ? 1e3 * tGreen_ / nSolve_ : 0.0, nSolve_ ? 1e3 * tPhi0_ / nSolve_ : 0.0,
                         nSolve_ ? 1e3 * tKern_ / nSolve_ : 0.0, 1e3 * tInterp_ / nDep_);
    }

private:
    std::unique_ptr<F32Device> D_;
    bool prof_ = std::getenv("AVAS_ENGINE_PROFILE") != nullptr;
    // AVAS_ENGINE_F32_ZCODE=0: z-code rows and beam parameters on the host (as before phase 4; measurement only)
    bool zOn_ = !(std::getenv("AVAS_ENGINE_F32_ZCODE") && std::string(std::getenv("AVAS_ENGINE_F32_ZCODE")) == "0");
    template <class T>
    T* hp(F32Buf& b, size_t off = 0) const { return reinterpret_cast<T*>(static_cast<char*>(b.host) + off); }
    static F32Slice SL(const F32Buf& b, size_t off) { return F32Slice{&b, off}; }
    long long nStep_ = 0, nFrom_ = 0, nTo_ = 0;
    long long nRows_ = 0, nZtot_ = 0, nUbp_ = 0, nRowsConv_ = 0, nRecTo_ = 0, nRebase_ = 0;
    double tZ_ = 0, tConv_ = 0;
    static constexpr size_t kRow = 26 + 21 + 8 + 6;  // first z-code row column of the state
    bool hostCurrent_ = false;  // the fp64 arrays hold the float state (to_host since the last change)
    bool rowsValid_ = true;     // the float rows match the fp64 records (else converted before use)
    bool recStale_ = false;     // the float rows are newer than the fp64 records
    bool rowsT0Set_ = false;
    double T0_ = 0.0;           // time origin of the float rows
    float pend_ = 0.0f;         // shift of the row times to the reference time, applied by the next transfer
    double tStep_ = 0, tGroup_ = 0, tHost_ = 0;
    double tDep_ = 0, tSolve_ = 0, tInterp_ = 0, tCharge_ = 0, tGreen_ = 0, tPhi0_ = 0, tKern_ = 0;  // space charge (profile)
    long long nDep_ = 0;
    std::uint64_t scHinted_ = 0;
    struct Timer { double& acc; double t0; ~Timer() { acc += now() - t0; } };
    static double now() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
    bool auth_ = false, stepOk_ = false, warned_ = false, hasEff_ = false, errOn_ = false, oldValid_ = false;
    kern::PartV hostP_{};
    long long nBound_ = 0, nCand_ = 0;
    double oldPre_[6] = {0, 0, 0, 0, 0, 0};
    int n_ = 0;
    size_t m_ = 1;  // column length of the particle state
    std::string why_;
    const LatHost* latZ_ = nullptr;
    PartFH pf_{};
    SyncH ref_{};
    std::vector<float> charge_;
    std::vector<unsigned char> okBuf_;
    bool qDirty_ = true;
    F32Buf bqs_;  // charges of the float state (deposit_state)
    F32Buf beff_, bsubs_, bgeom_, bseg_, bbndz_, bbndr_;
    F32Buf belems_, bmaps_, bmapd_, bstate_, bints_, bcross_, bcand_, boff_, brf_, bgs_, bge_, bgm_;

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
        const size_t m = m_;
        float* f = static_cast<float*>(bstate_.host);
        int* iv = static_cast<int*>(bints_.host);
        const double* c[6] = {P.x, P.ux, P.y, P.uy, P.z, P.uz};
        charge_.assign(static_cast<size_t>(n), 0.0f);
        qDirty_ = true;
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
            f[(kRow + 8) * m + i] = static_cast<float>(P.qOverM[i]);
            f[(kRow + 9) * m + i] = static_cast<float>(P.restMass[i]);
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

    F32Buf sz_, bx_, by_, bz_, bq_, blost_, blo_, bhi_, bnodes_, brho_, brhod_, bG_, bphi_, bfield_, bmf_, bsc_, bcorr_, bdval_;
    size_t fieldN3_ = 0;
    long long nSolve_ = 0, nLp_ = 0;
    F32Buf bfft_, bwork_;
    int fftKey_[3] = {0, 0, 0};
    // compact candidate entries (words per particle): cSnap 12, cNp 1, sSnap 12, sElem 1, aIdx 2, aRow 16, bEnt 16
    static constexpr size_t kCand = 60;

    // device buffer (with its host copy) of at least `bytes`, kept for the next call
    F32Buf& buf(F32Buf& b, size_t bytes) {
        D_->ensure(b, bytes == 0 ? 4 : bytes);
        return b;
    }
    // memory the host and the kernels both use directly
    F32Buf& shared(F32Buf& b, size_t bytes) {
        D_->ensure_shared(b, bytes == 0 ? 4 : bytes);
        return b;
    }
    // double -> float into a buffer, uploaded
    void flu(F32Buf& b, size_t n, const double* v) {
        fl(b, n, v);
        D_->upload(b, 0, n * 4);
    }
    // the mesh field for an interpolation: given (double, converted and uploaded) or the device's own
    F32Buf& field_up(const double* ex, const double* ey, const double* ez, size_t n3) {
        if (!ex) return bfield_;
        F32Buf& fe = buf(bmf_, 3 * n3 * 4);
        float* f = static_cast<float*>(fe.host);
        for (size_t e = 0; e < n3; ++e) {
            f[e] = static_cast<float>(ex[e]);
            f[n3 + e] = static_cast<float>(ey[e]);
            f[2 * n3 + e] = static_cast<float>(ez[e]);
        }
        D_->upload(fe, 0, 3 * n3 * 4);
        return fe;
    }
    // The fp64 Green table on the host (the device back end's numbers: sck::green_corner / green_cell, every
    // entry on its own, so in parallel) as float.
    void host_green(const sck::SpGrid& g, const ScSolveIn& in, float* out) {
        const int ax = g.mx + 1, ay = g.my + 1, az = g.mz + 1;
        std::vector<double> F(static_cast<size_t>(ax) * ay * az);
        const size_t nG = static_cast<size_t>(2 * g.nx + 1) * (2 * g.ny + 1) * (2 * g.nz + 1);
        std::vector<double> G(nG, 0.0);
        const long long nF = static_cast<long long>(F.size()), nC = static_cast<long long>(g.mx) * g.my * g.mz;
        par(nF, [&](long long e) {
            F[static_cast<size_t>(e)] = sck::green_corner(static_cast<int>(e / (ay * az)), static_cast<int>((e / az) % ay),
                                                          static_cast<int>(e % az), in.dx, in.dy, in.dz);
        });
        par(nC, [&](long long e) {
            sck::green_cell(g, F.data(), in.vol, static_cast<int>(e / (g.my * g.mz)), static_cast<int>((e / g.mz) % g.my),
                            static_cast<int>(e % g.mz), G.data());
        });
        for (size_t e = 0; e < nG; ++e) out[e] = static_cast<float>(G[e]);
    }
    template <class Fn>
    static void par(long long n, Fn&& fn) {
        int nt = options().threads > 0 ? options().threads : static_cast<int>(std::thread::hardware_concurrency());
        nt = static_cast<int>(std::max<long long>(1, std::min<long long>(nt, n / 1024 + 1)));
        std::vector<std::thread> th;
        for (int t = 0; t < nt; ++t)
            th.emplace_back([&, t] {
                for (long long e = n * t / nt; e < n * (t + 1) / nt; ++e) fn(e);
            });
        for (auto& x : th) x.join();
    }
    // double -> float into a buffer
    float* fl(F32Buf& b, size_t n, const double* v) {
        float* f = static_cast<float*>(buf(b, n * 4).host);
        for (size_t i = 0; i < n; ++i) f[i] = static_cast<float>(v[i]);
        return f;
    }
    // a row kernel; the pending move of the row times to the reference time is applied first
    void zcall(ZArgsH& A) {
        A.shift = pend_;
        const double t0 = now();
        D_->zrow(&A, &pf_, n_);
        tZ_ += now() - t0;
        T0_ = T0_ + static_cast<double>(pend_);
        pend_ = 0.0f;
    }
    ZArgsH zargs(int code) const {
        ZArgsH A{};
        A.code = code;
        A.stride = static_cast<int>(m_);
        A.n = n_;
        return A;
    }
    RowOpH rowop(const kern::RowOp& o) const {
        RowOpH r{};
        r.kind = o.kind;
        r.scKick = o.scKick;
        r.err = o.err;
        r.hv = o.hv;
        r.strength = static_cast<float>(o.strength);
        r.len = static_cast<float>(o.len);
        r.uzs = static_cast<float>(o.uzs);
        r.k1 = static_cast<float>(o.k1);
        r.k2 = static_cast<float>(o.k2);
        const double* bs[9] = {&o.bs.s, &o.bs.h, &o.bs.kx, &o.bs.kx2, &o.bs.ky, &o.bs.sx, &o.bs.cx, &o.bs.sy, &o.bs.cy};
        float* bd[9] = {&r.bs.s, &r.bs.h, &r.bs.kx, &r.bs.kx2, &r.bs.ky, &r.bs.sx, &r.bs.cx, &r.bs.sy, &r.bs.cy};
        for (int j = 0; j < 9; ++j) *bd[j] = static_cast<float>(*bs[j]);
        r.bs.hv = o.bs.hv;
        r.e = elemf(o.e);
        return r;
    }
    // fp64 records -> float rows (after the host changed them); T0 = the earliest recorded time
    void ensure_rows(const kern::PartV& P, int n) {
        if (rowsValid_) return;
        ++nRowsConv_;
        struct Up { F32Device& d; F32Buf& b; size_t off, len; ~Up() { d.upload(b, off, len); } } up{*D_, bstate_, kRow * m_ * 4, 8 * m_ * 4};
        float* rw = static_cast<float*>(bstate_.host) + kRow * m_;
        rowsT0Set_ = false;
        T0_ = 0.0;
        for (int i = 0; i < n; ++i) {
            const double* r = P.rec + 8 * static_cast<long long>(i);
            if (r[7] == 1.0 && (!rowsT0Set_ || r[6] < T0_)) {
                T0_ = r[6];
                rowsT0Set_ = true;
            }
        }
        for (int i = 0; i < n; ++i) {
            const double* r = P.rec + 8 * static_cast<long long>(i);
            for (int j = 0; j < 8; ++j) rw[j * m_ + static_cast<size_t>(i)] = static_cast<float>(j == 6 ? r[6] - T0_ : r[j]);
        }
        rowsValid_ = true;
        recStale_ = false;
        pend_ = 0.0f;
    }
    // Fixed-point group sums of the C / Mx columns written by a zrow kernel: ncol sums merged in group
    // order (reduce.h), nmx maxima.
    void gsum(int ncol, int nmx, int nmx0, double* out, float* mxOut) {
        const int ng = red_groups(n_);
        const double t0 = now();
        F32Buf& gs = shared(bgs_, static_cast<size_t>(ng) * 21 * 8);
        F32Buf& ge = shared(bge_, static_cast<size_t>(ng) * 21 * 4);
        F32Buf& gm = shared(bgm_, static_cast<size_t>(ng) * 8 * 4);
        struct T { double& acc; double t0; ~T() { acc += now() - t0; } } timer{tZ_, t0};
        D_->group(SL(bstate_, 26 * m_ * 4), SL(bstate_, (26 + 21) * m_ * 4), SL(gs, 0), SL(ge, 0), SL(gm, 0), n_,
                  static_cast<int>(m_), ncol, nmx, nmx0);
        const std::int64_t* sv = static_cast<const std::int64_t*>(gs.host);
        const std::int32_t* ex = static_cast<const std::int32_t*>(ge.host);
        const float* mx = static_cast<const float*>(gm.host);
        for (int k = 0; k < ncol; ++k) out[k] = 0.0;
        for (int k = 0; k < nmx; ++k) mxOut[k] = -HUGE_VALF;
        for (int g = 0; g < ng; ++g) {
            for (int k = 0; k < ncol; ++k)
                out[k] = out[k] + std::ldexp(static_cast<double>(sv[ncol * g + k]), -ex[ncol * g + k]);
            for (int k = 0; k < nmx; ++k) mxOut[k] = std::max(mxOut[k], mx[nmx * g + k]);
        }
    }

    // MeshV in the Metal layout; the node coordinates go into bnodes_
    MeshF mesh(const kern::MeshV& M) {
        F32Buf& b = buf(bnodes_, static_cast<size_t>(M.nx + M.ny + M.nz) * 4);
        float* f = static_cast<float*>(b.host);
        for (int i = 0; i < M.nx; ++i) f[i] = static_cast<float>(M.xs[i]);
        for (int j = 0; j < M.ny; ++j) f[M.nx + j] = static_cast<float>(M.ys[j]);
        for (int k = 0; k < M.nz; ++k) f[M.nx + M.ny + k] = static_cast<float>(M.zs[k]);
        D_->upload(b, 0, static_cast<size_t>(M.nx + M.ny + M.nz) * 4);
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
        const std::uint64_t a = b.dev;
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
};

}  // namespace

std::unique_ptr<ScAccel> make_f32_accel(std::unique_ptr<F32Device> dev, std::string& why) {
    auto a = std::make_unique<F32Accel>(std::move(dev));
    if (!a->init(why)) return nullptr;
    return a;
}

std::unique_ptr<ScAccel> make_f32_cpu_accel(std::string& why) { return make_f32_accel(make_f32_cpu_device(), why); }

}  // namespace avas
