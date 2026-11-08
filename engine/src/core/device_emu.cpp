// Test device "emu" (AVAS_ENGINE_DEVICE=emu): the device back end of device.h running on the CPU.
//
// It keeps its own copy of the particles, divided into whole reduction groups over the "GPUs" of
// AVAS_ENGINE_GPUS (default one, "0,1" two) exactly like the CUDA back end, and runs the shared kernels
// of kernels.h on them.  The tracker's host copies are refreshed only through download() / upload(),
// so a missing transfer shows up as a difference to the plain CPU run; and the results must be
// byte-identical to the CPU engine in the corrected mode (fp64, no FMA).  Not meant for production.
#include <algorithm>
#include <cmath>
#include <sstream>
#include <thread>

#include "core/device.h"
#include "core/errors.h"
#include "core/options.h"
#include "core/reduce.h"
#include "core/sc_kernels.h"
#include "core/secondary.h"

namespace avas {

namespace {

struct EmuShard {
    int lo = 0, hi = 0;
    int n() const { return hi - lo; }
    std::vector<double> x, ux, y, uy, z, uz, ex, ey, ez, bx, by, bz, scx, scy, scz, rel[6];
    std::vector<double> weight, qOverM, charge, spOutZ0, spOutZ1, old, rec, spRec, restMass, t, dt;
    std::vector<int> elemIndex, lost, nextPlane, boundaryMode, geomIndex, spOutCount;
    kern::PartV P{};
    std::vector<double> xs, ys, zs, ex3, ey3, ez3;
};

// Runs f(0..n-1) on the CPU threads (options threads, default all cores).  Every index writes its own
// output (a reduction group), so the result does not depend on the thread count.
template <class F>
void par_for(int n, F&& f) {
    int nt = options().threads > 0 ? options().threads : static_cast<int>(std::thread::hardware_concurrency());
    nt = std::max(1, std::min(nt, n));
    if (nt <= 1) {
        for (int i = 0; i < n; ++i) f(i);
        return;
    }
    std::vector<std::thread> th;
    for (int t = 0; t < nt; ++t)
        th.emplace_back([&, t] {
            for (int i = t; i < n; i += nt) f(i);
        });
    for (auto& x : th) x.join();
}

template <class V>
void cp_in(V& dst, const V& src, int lo, int n, int per = 1) {
    dst.assign(src.begin() + static_cast<long>(lo) * per, src.begin() + static_cast<long>(lo + n) * per);
}

class EmuBackend : public DeviceBackend {
public:
    EmuBackend(int nDev, std::unique_ptr<ScAccel> accel) : nDev_(nDev), accel_(std::move(accel)) {}
    std::string describe() const override {
        if (accel_) return accel_->describe();
        return "emu (device back end on the CPU, " + std::to_string(nDev_) + " shard" + (nDev_ > 1 ? "s" : "") + ")";
    }
    bool fp32() const override { return false; }

    void begin_run(Beam& b, const LatHost& L, bool spRec) override {
        n_ = b.n;
        L_ = &L;
        lat_ = L.view(Lattice());  // pointers into the host copy; scalars set per call
        const int ng = red_groups(n_);
        sh_.assign(static_cast<size_t>(nDev_), EmuShard{});
        for (int k = 0; k < nDev_; ++k) {
            EmuShard& s = sh_[static_cast<size_t>(k)];
            s.lo = std::min(n_, static_cast<int>(static_cast<long long>(ng) * k / nDev_) * kRedGroup);
            s.hi = std::min(n_, static_cast<int>(static_cast<long long>(ng) * (k + 1) / nDev_) * kRedGroup);
            const int m = s.n();
            s.old.assign(static_cast<size_t>(6 * m), 0.0);
            s.rec.assign(static_cast<size_t>(8 * m), 0.0);
            s.spRec.assign(static_cast<size_t>(8 * m), 0.0);
            cp_in(s.weight, b.weight, s.lo, m);
            cp_in(s.qOverM, b.qOverM, s.lo, m);
            cp_in(s.charge, b.charge, s.lo, m);
            cp_in(s.restMass, b.restMass, s.lo, m);
            cp_in(s.t, b.t, s.lo, m);
            cp_in(s.dt, b.dt, s.lo, m);
        }
        (void)spRec;
        upload(b, nullptr, nullptr, nullptr, kDevAll & ~(kDevOld | kDevRec | kDevSpRec));
        if (accel_) accel_->begin_run(L, n_);
    }

    // The accelerator's float state back into the fp64 arrays before any work on them.
    void host() {
        if (accel_ && accel_->authoritative())
            for (EmuShard& s : sh_) accel_->to_host(s.P, s.n());
    }

    void set_elems(const LatHost& L) override {  // the view points at the host table
        if (accel_) accel_->set_elems(L);
    }

    void download(Beam& b, double* old, double* rec, double* spRec, unsigned m) override {
        host();
        for (EmuShard& s : sh_) {
            const int o = s.lo, c = s.n();
            auto put = [&](const std::vector<double>& v, std::vector<double>& h) { std::copy(v.begin(), v.end(), h.begin() + o); };
            auto puti = [&](const std::vector<int>& v, std::vector<int>& h) { std::copy(v.begin(), v.end(), h.begin() + o); };
            if (m & kDevCoord) { put(s.x, b.x); put(s.ux, b.ux); put(s.y, b.y); put(s.uy, b.uy); put(s.z, b.z); put(s.uz, b.uz); }
            if (m & kDevField) { put(s.ex, b.ex); put(s.ey, b.ey); put(s.ez, b.ez); put(s.bx, b.bx); put(s.by, b.by); put(s.bz, b.bz); }
            if (m & kDevSc) { put(s.scx, b.scx); put(s.scy, b.scy); put(s.scz, b.scz); }
            if (m & kDevRel) for (int j = 0; j < 6; ++j) put(s.rel[j], b.rel[j]);
            if (m & kDevLost) puti(s.lost, b.lost);
            if (m & kDevIdx) {
                puti(s.elemIndex, b.elemIndex); puti(s.nextPlane, b.nextPlane); puti(s.boundaryMode, b.boundaryMode);
                puti(s.geomIndex, b.geomIndex); puti(s.spOutCount, b.spOutCount);
            }
            if (m & kDevSpz) { put(s.spOutZ0, b.spOutZ0); put(s.spOutZ1, b.spOutZ1); }
            if ((m & kDevOld) && old) std::copy(s.old.begin(), s.old.end(), old + 6 * o);
            if ((m & kDevRec) && rec) std::copy(s.rec.begin(), s.rec.end(), rec + 8 * o);
            if ((m & kDevSpRec) && spRec) std::copy(s.spRec.begin(), s.spRec.end(), spRec + 8 * o);
            (void)c;
        }
    }

    void upload(const Beam& b, const double* old, const double* rec, const double* spRec, unsigned m) override {
        host();
        if (accel_) accel_->invalidate();
        for (EmuShard& s : sh_) {
            const int o = s.lo, c = s.n();
            if (m & kDevCoord) { cp_in(s.x, b.x, o, c); cp_in(s.ux, b.ux, o, c); cp_in(s.y, b.y, o, c); cp_in(s.uy, b.uy, o, c); cp_in(s.z, b.z, o, c); cp_in(s.uz, b.uz, o, c); }
            if (m & kDevField) { cp_in(s.ex, b.ex, o, c); cp_in(s.ey, b.ey, o, c); cp_in(s.ez, b.ez, o, c); cp_in(s.bx, b.bx, o, c); cp_in(s.by, b.by, o, c); cp_in(s.bz, b.bz, o, c); }
            if (m & kDevSc) { cp_in(s.scx, b.scx, o, c); cp_in(s.scy, b.scy, o, c); cp_in(s.scz, b.scz, o, c); }
            if (m & kDevRel) for (int j = 0; j < 6; ++j) cp_in(s.rel[j], b.rel[j], o, c);
            if (m & kDevLost) cp_in(s.lost, b.lost, o, c);
            if (m & kDevIdx) {
                cp_in(s.elemIndex, b.elemIndex, o, c); cp_in(s.nextPlane, b.nextPlane, o, c); cp_in(s.boundaryMode, b.boundaryMode, o, c);
                cp_in(s.geomIndex, b.geomIndex, o, c); cp_in(s.spOutCount, b.spOutCount, o, c);
            }
            if (m & kDevSpz) { cp_in(s.spOutZ0, b.spOutZ0, o, c); cp_in(s.spOutZ1, b.spOutZ1, o, c); }
            if ((m & kDevOld) && old) s.old.assign(old + 6 * o, old + 6 * (o + c));
            if ((m & kDevRec) && rec) s.rec.assign(rec + 8 * o, rec + 8 * (o + c));
            if ((m & kDevSpRec) && spRec) s.spRec.assign(spRec + 8 * o, spRec + 8 * (o + c));
            view(s);
        }
    }

    void zero_rec(bool sp) override {
        if (accel_) accel_->rec_cleared(sp);
        for (EmuShard& s : sh_) std::fill((sp ? s.spRec : s.rec).begin(), (sp ? s.spRec : s.rec).end(), 0.0);
    }

    void step(const LatScalars& ls, const kern::StepArgs& a0, const std::vector<double>& rf,
              const std::vector<double>& events, std::vector<StepGroup>& out, long long& spCount,
              long long& recCount) override {
        const int ng = red_groups(n_);
        out.assign(static_cast<size_t>(ng), StepGroup{});
        spCount = recCount = 0;
        set_scalars(ls);
        if (accel_ && sh_.size() == 1 && accel_->step(ls, a0, rf, events, sh_[0].P, sh_[0].n(), out, spCount, recCount)) return;
        host();
        kern::StepArgs a = a0;
        a.events = events.data();
        const kern::RfSrc src{rf.data(), a.tEnd};
        std::vector<std::pair<int, int>> work;  // (shard, group)
        for (int k = 0; k < static_cast<int>(sh_.size()); ++k)
            for (int g = 0; g < red_groups(sh_[static_cast<size_t>(k)].n()); ++g) work.push_back({k, g});
        par_for(static_cast<int>(work.size()), [&](int w) {
            {
                EmuShard& s = sh_[static_cast<size_t>(work[static_cast<size_t>(w)].first)];
                const int g = work[static_cast<size_t>(w)].second;
                StepGroup& o = out[static_cast<size_t>(s.lo / kRedGroup + g)];
                for (int j = 0; j < 6; ++j) o.mx[j] = 0.0;
                o.zmin = 1e10;
                o.zmax = -1e10;
                group_sum<kern::kStepSums>(g * kRedGroup, s.n(), [&](int i, double* c) {
                    kern::Contrib ct;
                    kern::trans_particle<PortableMath>(lat_, s.P, i, a, src, ct);
                    kern::contrib_sums(ct, c);
                    if (ct.hasA) {
                        for (int j = 0; j < 6; ++j)
                            if (o.mx[j] < ct.mx[j]) o.mx[j] = ct.mx[j];
                        if (ct.zmin < o.zmin) o.zmin = ct.zmin;
                        if (o.zmax < ct.zmax) o.zmax = ct.zmax;
                    }
                    o.sp += ct.sp;
                    if (s.rec[8 * static_cast<size_t>(i) + 7] == 1.0) o.rec += 1.0;
                }, o.s);
            }
        });
        for (const StepGroup& o : out) {
            spCount += static_cast<long long>(o.sp);
            recCount += static_cast<long long>(o.rec);
        }
    }

    void ubp1(const double* s6, double* sums) override {
        host();
        reduce<kern::kUbpSums>([&](EmuShard& s, int i, double* c) { kern::ubp_pass1(s.P, i, s6, c); }, sums);
    }
    void ubp2(const double* cen, double igz, double* q3, double& zmin, double& zmax) override {
        host();
        reduce<3>([&](EmuShard& s, int i, double* c) { kern::ubp_pass2(s.P, i, cen, igz, c); }, q3);
        zmin = 1e10;
        zmax = -1e10;
        for (EmuShard& s : sh_)
            for (double v : s.z) {
                if (v < zmin) zmin = v;
                if (zmax < v) zmax = v;
            }
    }
    void rows(const kern::RowOp& op) override {
        host();
        each([&](EmuShard& s, int i) {
            double* r = s.rec.data() + 8 * static_cast<size_t>(i);
            if (r[7] == 1.0) kern::het_row<PortableMath>(op, r, s.qOverM[i], s.scx[i], s.scy[i], s.scz[i]);
        });
    }
    void rows_tref(bool average, double& tRef) override {
        host();
        if (average) {
            double s2[2];
            reduce<2>([&](EmuShard& s, int i, double* c) {
                const double* r = s.rec.data() + 8 * static_cast<size_t>(i);
                c[0] = r[7] == 1.0 ? r[6] : 0.0;
                c[1] = r[7] == 1.0 ? 1.0 : 0.0;
            }, s2);
            tRef = s2[0] / s2[1];
        } else {
            rec_tmin(false, tRef);
        }
    }
    void rows_kick(int kind, double wx, double wy, double len) override {
        host();
        each([&](EmuShard& s, int i) {
            double* r = s.rec.data() + 8 * static_cast<size_t>(i);
            if (r[7] == 1.0) kern::kick_row(r, kind, wx, wy, len * s.qOverM[i], false);
        });
    }
    void ztot(double zPlane, double tRef) override {
        host();
        each([&](EmuShard& s, int i) {
            s.nextPlane[i] = 0;
            double* r = s.rec.data() + 8 * static_cast<size_t>(i);
            if (r[7] == 1.0) ztot_row(r, zPlane, tRef, true, s.x[i], s.ux[i], s.y[i], s.uy[i], s.z[i], s.uz[i]);
            for (int j = 0; j < 8; ++j) r[j] = 0.0;
        });
    }
    void rec_tmin(bool sp, double& tmin) override {
        host();
        tmin = 1e10;
        each([&](EmuShard& s, int i) {
            const double* r = (sp ? s.spRec : s.rec).data() + 8 * static_cast<size_t>(i);
            if (r[7] == 1.0 && r[6] < tmin) tmin = r[6];
        });
    }
    void hesc1(double tRef, double* sums) override {
        host();
        reduce<kern::kHescSums>([&](EmuShard& s, int i, double* c) { kern::hesc_pass1(s.P, i, tRef, c); }, sums);
    }
    void hesc2(const double* cen, double gc, double* q3) override {
        host();
        reduce<3>([&](EmuShard& s, int i, double* c) { kern::hesc_pass2(s.P, i, cen, gc, c); }, q3);
    }
    void sc_deposit(const kern::MeshV& M0, std::vector<long long>& acc) override {
        acc.assign(static_cast<size_t>(M0.nx) * M0.ny * M0.nz, 0);
        if (accel_ && accel_->authoritative()) {
            accel_->deposit_state(M0, sh_[0].n(), acc);
            return;
        }
        for (EmuShard& s : sh_) {
            s.xs.assign(M0.xs, M0.xs + M0.nx);
            s.ys.assign(M0.ys, M0.ys + M0.ny);
            s.zs.assign(M0.zs, M0.zs + M0.nz);
            kern::MeshV M = M0;
            M.xs = s.xs.data();
            M.ys = s.ys.data();
            M.zs = s.zs.data();
            if (accel_) {
                accel_->deposit(M, s.n(), s.rel[0].data(), s.rel[2].data(), s.rel[4].data(), s.charge.data(), s.lost.data(), acc);
                continue;
            }
            for (int p = s.n() - 1; p >= 0; --p) {  // any order: integer sums
                if (s.lost[p] == 1) continue;
                int i, j, k;
                double w[8];
                if (!kern::mesh_cell(M, s.P, p, i, j, k, w)) continue;
                for (int q = 0; q < 8; ++q)
                    acc[static_cast<size_t>(kern::mesh_corner(M, i, j, k, q))] += kern::fix_charge(w[q] * s.charge[p], M.fixScaleExp);
            }
        }
    }
    void sc_solve(const ScSolveIn& in) override {
        const sck::SpGrid g{in.nx, in.ny, in.nz, in.mx, in.my, in.mz};
        const long long n3 = g.n3();
        phi_.assign(static_cast<size_t>(n3), 0.0);
        ex_.assign(static_cast<size_t>(n3), 0.0);
        ey_.assign(static_cast<size_t>(n3), 0.0);
        ez_.assign(static_cast<size_t>(n3), 0.0);
        const double hx = 1.0 / (in.dx + in.dx), hy = 1.0 / (in.dy + in.dy), hz = 1.0 / (in.dz + in.dz);
        int i, j, k;
        if (in.method == 1 || in.method == 2) {
            // Green table and correction list as on the GPU (in reverse order: independent entries)
            const int ax = g.mx + 1, ay = g.my + 1, az = g.mz + 1;
            std::vector<double> F(static_cast<size_t>(ax) * ay * az);
            for (long long e = static_cast<long long>(F.size()) - 1; e >= 0; --e)
                F[static_cast<size_t>(e)] = sck::green_corner(static_cast<int>(e / (ay * az)), static_cast<int>((e / az) % ay),
                                                              static_cast<int>(e % az), in.dx, in.dy, in.dz);
            G_.assign(static_cast<size_t>(2 * g.nx + 1) * (2 * g.ny + 1) * (2 * g.nz + 1), 0.0);
            for (long long e = static_cast<long long>(g.mx) * g.my * g.mz - 1; e >= 0; --e)
                sck::green_cell(g, F.data(), in.vol, static_cast<int>(e / (g.my * g.mz)), static_cast<int>((e / g.mz) % g.my),
                                static_cast<int>(e % g.mz), G_.data());
        }
        if (accel_ && (in.method == 1 || in.method == 2)) {
            // accelerator: the sums in fp32 there; table and correction list from the host (fp64)
            std::vector<int> corr;
            std::vector<double> dval;
            if (in.method == 1) corr_list(g, in, corr, dval);
            if (accel_->solve(in, G_.data(), corr.data(), dval.data(), static_cast<long long>(dval.size()), ex_, ey_, ez_)) return;
        }
        if (in.method == 2) {
            for (long long id = n3 - 1; id >= 0; --id) {
                g.ijk(id, i, j, k);
                phi_[static_cast<size_t>(id)] = sck::picnic_node<double>(g, in.rho, G_.data(), i, j, k);
            }
        }
        if (in.method == 1) {
            std::vector<int> corr;
            std::vector<double> dval;
            corr_list(g, in, corr, dval);
            const long long nt = g.n_targets();
            const long long nc = static_cast<long long>(dval.size());
            for (long long id = nt - 1; id >= 0; --id) {  // any order: independent targets
                g.target(id, i, j, k);
                sck::sym_target<double>(g, in.rho, G_.data(), in.thr, i, j, k, phi_.data());
            }
            if (nc > 0)
                for (long long id = 0; id < n3; ++id) {
                    g.ijk(id, i, j, k);
                    sck::corr_node<double>(g, G_.data(), corr.data(), dval.data(), nc, i, j, k, phi_.data());
                }
        }
        if (in.method == 1 || in.method == 2) {
            for (long long id = 0; id < n3; ++id) {
                g.ijk(id, i, j, k);
                sck::grad_spicnic_node(g, phi_.data(), hx, hy, hz, i, j, k, ex_.data(), ey_.data(), ez_.data());
            }
            return;
        }
        const sck::FftTables T = sck::fft_tables(in.nx, in.ny, in.nz);
        std::vector<double> A(static_cast<size_t>(n3)), B(A), C(A), D(A);
        for (long long e = 0; e < n3; ++e) A[static_cast<size_t>(e)] = in.rho[e] * in.scale;
        auto dst = [&](int axis, const std::vector<double>& src, std::vector<double>& dstv, const std::vector<double>& tab) {
            for (long long id = 0; id < n3; ++id) {
                g.ijk(id, i, j, k);
                dstv[static_cast<size_t>(id)] = sck::dst_node(g, axis, src.data(), tab.data(), i, j, k);
            }
        };
        auto dft = [&](const std::vector<double>& re, const std::vector<double>* im, std::vector<double>& ore,
                       std::vector<double>& oim, double sign) {
            for (long long id = 0; id < n3; ++id) {
                g.ijk(id, i, j, k);
                sck::dft_z_node(g, re.data(), im ? im->data() : nullptr, T.cz.data(), T.sz.data(), sign, i, j, k,
                                ore[static_cast<size_t>(id)], oim[static_cast<size_t>(id)]);
            }
        };
        dst(0, A, B, T.sx);
        dst(1, B, A, T.sy);
        dft(A, nullptr, C, D, -1.0);
        for (long long id = 0; id < n3; ++id) {
            g.ijk(id, i, j, k);
            const double f = sck::green_factor(in.lx, in.ly, in.lz, i, j, k);
            C[static_cast<size_t>(id)] = C[static_cast<size_t>(id)] * f;
            D[static_cast<size_t>(id)] = D[static_cast<size_t>(id)] * f;
        }
        dft(C, &D, A, B, 1.0);
        dst(0, A, C, T.sx);
        dst(1, C, D, T.sy);
        for (long long e = 0; e < n3; ++e) phi_[static_cast<size_t>(e)] = D[static_cast<size_t>(e)] * in.norm;
        for (long long id = 0; id < n3; ++id) {
            g.ijk(id, i, j, k);
            sck::grad_fft_node(g, phi_.data(), hx, hy, hz, i, j, k, ex_.data(), ey_.data(), ez_.data());
        }
    }
    // SPICNIC correction list in the order of the GPU (sck::corr_value).
    static void corr_list(const sck::SpGrid& g, const ScSolveIn& in, std::vector<int>& corr, std::vector<double>& dval) {
        const long long nt = g.n_targets();
        for (long long q = 0; q < 8 * nt; ++q) {
            int I, J, K;
            const double d = sck::corr_value(g, in.rho, in.thr, q / 8, static_cast<int>(q % 8), I, J, K);
            if (d != 0.0) {
                corr.push_back(I);
                corr.push_back(J);
                corr.push_back(K);
                dval.push_back(d);
            }
        }
    }
    void sc_fields(std::vector<double>& ex, std::vector<double>& ey, std::vector<double>& ez) override {
        ex = ex_;
        ey = ey_;
        ez = ez_;
    }
    void sc_interp(const kern::MeshV& M0) override {
        if (accel_ && accel_->authoritative()) {
            accel_->interp_state(M0, ex_.data(), ey_.data(), ez_.data(), sh_[0].n());
            return;
        }
        for (EmuShard& s : sh_) {
            kern::MeshV M = M0;
            M.xs = s.xs.data();
            M.ys = s.ys.data();
            M.zs = s.zs.data();
            if (accel_) {
                accel_->interp(M, ex_.data(), ey_.data(), ez_.data(), s.n(), s.rel[0].data(), s.rel[2].data(), s.rel[4].data(),
                               s.lost.data(), s.scx.data(), s.scy.data(), s.scz.data());
                continue;
            }
            for (int p = 0; p < s.n(); ++p) kern::mesh_interp(M, s.P, p, ex_.data(), ey_.data(), ez_.data(), sck::kE);
        }
    }
    void boundary(const LatScalars& ls, double t, double dt, std::vector<BoundaryHit>& hits) override {
        if (accel_ && accel_->authoritative() && accel_->boundary(ls, t, dt, hits)) return;
        host();
        set_scalars(ls);
        hits.clear();
        for (int k = static_cast<int>(sh_.size()) - 1; k >= 0; --k) {  // shards in reverse: the host sorts
            EmuShard& s = sh_[static_cast<size_t>(k)];
            for (int i = 0; i < s.n(); ++i) {
                BoundaryHit h{};
                h.tHit = t;
                const int code = kern::boundary_particle<PortableMath>(lat_, s.P, i, t, dt, false, h.geom, h.P, h.tHit);
                if (code == 0) continue;
                if (code == 1) s.lost[i] = 1;
                h.i = s.lo + i;
                h.code = code;
                hits.push_back(h);
            }
        }
    }
    void lp_fold(double zs, double full, double half) override {
        host();
        each([&](EmuShard& s, int i) { kern::lp_fold_particle(s.P, i, zs, full, half); });
    }
    void long_loss(double zs, double ws, double dW, double dzMax) override {
        host();
        each([&](EmuShard& s, int i) { kern::long_loss_particle(s.P, i, s.restMass.data(), zs, ws, dW, dzMax, false); });
    }
    void secondary(const LatScalars& ls, const SeHost& se, const kern::SeV& v, SeOut& out) override {
        host();
        const int nSlots = std::max(se.nSlots, 1);
        out.planes.assign(static_cast<size_t>(n_) * nSlots * 8, 0.0);
        out.hits.clear();
        struct Sink {
            double* planes;
            int nSlots, lo;
            std::vector<BoundaryHit>* hits;
            void plane(int slot, const double* S, bool, int i) {
                for (int j = 0; j < 8; ++j) planes[(static_cast<size_t>(i) * nSlots + slot) * 8 + j] = S[j];
            }
            void sync_step(int) {}
            void sync_iter(int) {}
            void hit(int gi, int code, const double* P, double tHit, int i) {
                BoundaryHit h{};
                h.i = lo + i;
                h.code = code;
                h.geom = gi;
                for (int j = 0; j < 6; ++j) h.P[j] = P[j];
                h.tHit = tHit;
                hits->push_back(h);
            }
        };
        set_scalars(ls);
        for (EmuShard& s : sh_) {
            Sink sink{out.planes.data() + static_cast<size_t>(s.lo) * nSlots * 8, nSlots, s.lo, &out.hits};
            for (int i = 0; i < s.n(); ++i) kern::se_track<PortableMath>(lat_, s.P, v, i, false, sink, nullptr);
        }
    }
    void download_times(Beam& b) override {
        host();
        for (EmuShard& s : sh_) std::copy(s.t.begin(), s.t.end(), b.t.begin() + s.lo);
    }

    void set_lattice_scalars(const LatScalars& ls) { sc_ = ls; }

private:
    int nDev_;
    int n_ = 0;
    const LatHost* L_ = nullptr;
    kern::LatV lat_{};
    LatScalars sc_{};
    std::vector<EmuShard> sh_;
    std::vector<double> G_, phi_, ex_, ey_, ez_;
    std::unique_ptr<ScAccel> accel_;

    void set_scalars(const LatScalars& ls) {
        lat_ = L_->view(Lattice());
        lat_.spOutCur = ls.spOutCur;
        lat_.spOutCount = ls.spOutCount;
        lat_.spOutSize = ls.spOutSize;
        lat_.totalLength = ls.totalLength;
        lat_.spExit = ls.spExit;
        sc_ = ls;
    }

    void view(EmuShard& s) {
        kern::PartV& P = s.P;
        P = kern::PartV{};
        P.x = s.x.data(); P.ux = s.ux.data(); P.y = s.y.data(); P.uy = s.uy.data(); P.z = s.z.data(); P.uz = s.uz.data();
        P.ex = s.ex.data(); P.ey = s.ey.data(); P.ez = s.ez.data(); P.bx = s.bx.data(); P.by = s.by.data(); P.bz = s.bz.data();
        P.scx = s.scx.data(); P.scy = s.scy.data(); P.scz = s.scz.data();
        for (int j = 0; j < 6; ++j) P.rel[j] = s.rel[j].data();
        P.elemIndex = s.elemIndex.data(); P.lost = s.lost.data(); P.nextPlane = s.nextPlane.data();
        P.boundaryMode = s.boundaryMode.data(); P.geomIndex = s.geomIndex.data(); P.spOutCount = s.spOutCount.data();
        P.weight = s.weight.data(); P.qOverM = s.qOverM.data(); P.charge = s.charge.data();
        P.spOutZ0 = s.spOutZ0.data(); P.spOutZ1 = s.spOutZ1.data();
        P.old = s.old.data(); P.rec = s.rec.data(); P.spRec = s.spRec.data(); P.t = s.t.data(); P.dt = s.dt.data();
        P.sync = -1;
    }

    template <class F>
    void each(F&& f) {
        for (EmuShard& s : sh_)
            for (int i = 0; i < s.n(); ++i) f(s, i);
    }

    template <int K, class F>
    void reduce(F&& f, double* out) {
        const int ng = red_groups(n_);
        std::vector<double> all(static_cast<size_t>(std::max(ng, 1)) * K, 0.0);
        std::vector<std::pair<int, int>> work;  // (shard, group), in parallel: one output slot each
        for (int k = 0; k < static_cast<int>(sh_.size()); ++k)
            for (int g = 0; g < red_groups(sh_[static_cast<size_t>(k)].n()); ++g) work.push_back({k, g});
        par_for(static_cast<int>(work.size()), [&](int w) {
            EmuShard& s = sh_[static_cast<size_t>(work[static_cast<size_t>(w)].first)];
            const int g = work[static_cast<size_t>(w)].second;
            group_sum<K>(g * kRedGroup, s.n(), [&](int i, double* c) { f(s, i, c); },
                         all.data() + static_cast<size_t>(s.lo / kRedGroup + g) * K);
        });
        sum_groups<K>(all.data(), ng, out);
    }
};

}  // namespace

std::unique_ptr<DeviceBackend> make_emu_backend(std::unique_ptr<ScAccel> accel) {
    const std::string& g = options().gpus;
    const int n = accel || g.empty() ? 1 : static_cast<int>(std::count(g.begin(), g.end(), ',')) + 1;
    return std::make_unique<EmuBackend>(n, std::move(accel));
}

}  // namespace avas
