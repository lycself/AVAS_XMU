#include "core/tracker.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "core/device.h"
#include "core/dllmath.h"
#include "core/engine.h"
#include "core/errors.h"
#include "core/errorset.h"
#include "core/files.h"
#include "core/format.h"
#include "core/kernels.h"
#include "core/latview.h"
#include "core/options.h"
#include "core/physics.h"
#include "core/reduce.h"
#include "core/secondary.h"
#include "core/spacecharge.h"
#include "core/workers.h"

namespace avas {

namespace {

// Per-block statistics (summed in block order, so results do not depend on scheduling).
struct BlockAcc {
    double maxAbs[6], sum2[6], cross[3], sum1[6];
    double zmin, zmax, sumGz2, cnt0, cnt1, gsum;
    int spCnt;
    void reset() {
        for (int j = 0; j < 6; ++j) maxAbs[j] = sum2[j] = sum1[j] = 0.0;
        for (int j = 0; j < 3; ++j) cross[j] = 0.0;
        zmin = 1e10;
        zmax = -1e10;
        sumGz2 = cnt0 = cnt1 = gsum = 0.0;
        spCnt = 0;
    }
};

using Rec = std::array<double, 8>;  // crossing record: x, ux, y, uy, z, uz, t, flag

class Tracker {
public:
    Tracker(Lattice& lat, Beam& b, Params& p) : lat_(lat), b_(b), p_(p), compat_(compat_dll()) {
        if (p.spaceCharge == 1) sc_ = std::make_unique<SpaceCharge>(p, b.n);
    }

    void run() {
        if (b_.secondary) {
            run_secondary();
            return;
        }
        const auto t0 = std::chrono::steady_clock::now();
        const int n = b_.n;
        planeRec_.assign(static_cast<size_t>(n) + 1, Rec{});
        spRec_.assign(static_cast<size_t>(n) + 1, Rec{});
        dt_ = (1.0 / b_.frequency) / p_.stepPerCycle;
        say(Text().s("The total distance simulated is ").g(lat_.total_length()).s(" (m)").str());
        say(Text().s("Simulation time step is ").g(dt_).s(" (m)").str());
        say("Start simulation, start time " + time_stamp());
        const std::string& out = paths().output;
        for (const char* f : {"DataSet.txt", "synParticle.txt", "test.txt", "testP.txt", "collisionData.txt",
                              "LongitudinalBeamLoss.txt", "BeamSet.plt", "ExData.edt", "FullParticle.txt",
                              "ParticleTracking.txt"})
            write_file(out + f, "", false);
        if (p_.recScField == 1)
            for (const char* f : {"SpaceChargeMesh.txt", "SpaceChargeSet.plt"}) write_file(out + f, "", false);
        dataSet_.open(out + "DataSet.txt");
        synFile_.open(out + "synParticle.txt");
        setup_errors();
        if (p_.dumpPeriodicity != 0) {
            // BeamSet.plt header (truncating)
            std::string h;
            h.push_back(static_cast<char>(0x7D));
            h.push_back(static_cast<char>(0x64));
            const std::int32_t d32 = p_.dumpPeriodicity, n32 = n;
            const double cur = b_.current * 1000.0, fm = b_.frequency / 1000000.0, mc2 = b_.restMass[n];
            h.append(reinterpret_cast<const char*>(&d32), 4);
            h.append(reinterpret_cast<const char*>(&n32), 4);
            h.append(reinterpret_cast<const char*>(&cur), 8);
            h.append(reinterpret_cast<const char*>(&fm), 8);
            h.append(reinterpret_cast<const char*>(&mc2), 8);
            if (!write_file(out + "BeamSet.plt", h, false, true))
                throw EngineError("BeamSet.plt open failed, Check permissions");
            if (!compat_) beamset_estimate();
        }

        const std::vector<Segment> segs = segment_lattice();
        const long long cells = p_.spaceCharge == 1 ? static_cast<long long>(p_.numOfGrid[0]) * p_.numOfGrid[1] * p_.numOfGrid[2] : 0;
        dev_ = make_device_backend(n, p_.spaceCharge == 1, cells);
        if (dev_) dev_->begin_run(b_, latH_, lat_.spOutCount > 0);
        startClock_ = std::chrono::steady_clock::now();
        zStartRun_ = b_.z[n];
        lastProgressPct_ = 0.0;
        double t = b_.tMin;
        double zSeg = 0.0;
        if (p_.dumpPeriodicity != 0) {
            for (int i = 0; i < n; ++i) {
                b_.rel[0][i] = b_.x[i] - b_.x[n];
                b_.rel[1][i] = b_.ux[i];
                b_.rel[2][i] = b_.y[i] - b_.y[n];
                b_.rel[3][i] = b_.uy[i];
                b_.rel[4][i] = b_.z[i] - b_.z[n];
                b_.rel[5][i] = b_.uz[i];
            }
            beam_out(0, b_.tMin, b_.z[n], -1);
        }
        for (const Segment& sg : segs) {
            if (sg.zcode) zcode(sg, t, zSeg);
            else tcode(t, zSeg, sg.ev, sg.marks);
        }

        say("Simulation completed, completion time " + time_stamp() + std::string(77, ' '));
        say("Total number of cycles 0. Solving the number of space charge effects 0");
        const double total = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0).count();
        Text st;
        auto c = [&](int k) { return static_cast<long long>(p_.cores[k]); };
        st.g(total / 1e6, 5).s(" Exter:").i(c(0)).s(": ").g(pushTime_ / 1e6, 5).s(" ").i(c(1)).s(": ").g(0.0, 5)
            .s(" ").i(c(2)).s(": ").g(0.0, 5).s(" ").i(c(3)).s(": ").g(0.0, 5).s(" Inter:").i(c(4)).s(": ").g(0.0, 5)
            .s(" ").i(c(5)).s(": ").g(0.0, 5).s(" ").i(c(6)).s(": ").g(0.0, 5).s(" ").i(c(7)).s(": ").g(0.0, 5)
            .s(" ").i(c(5)).s(": ").g(0.0, 5).s(" ").i(c(8)).s(": ").g(0.0, 5).s(" ").i(c(9)).s(": ").g(0.0, 5).nl();
        write_file(out + "SimulationTime.txt", st.str(), true);
    }

private:
    // ---------------- secondary particles (MethodMP::SeRun, spec 70 9-10) ----------------
    struct Segment;
    int maxSec_ = 0;
    std::vector<double> bendRef_;  // reference gamma*beta_z of each bend (from the synchronous particle)
    std::vector<double> seSlotZ_;  // z of every output slot of the secondary particles

    // Output sink of the shared SeRun code (secondary.h): files written as the original does.
    struct CpuSeSink {
        Tracker* T;
        void plane(int slot, const double* S, bool isSync, int i) { T->se_plane_out(i, S, T->seSlotZ_[slot], isSync); }
        void sync_step(int i) { T->se_syn_out(i); }
        void sync_iter(int iter) {
            if (T->maxSec_ < iter) T->maxSec_ = iter;
        }
        void hit(int gi, int code, const double* P, double tHit, int i) {
            const Geometry& g = T->lat_.bnd.geoms[static_cast<size_t>(gi)];
            if (code < 0) (void)g.par.at(static_cast<size_t>(-code - 10));
            if (code == 1) T->collision_line(g.shape, P, tHit, i);
        }
    };

    // Host arrays of the shared SeRun code.
    void build_se(const std::vector<Segment>& segs, SeHost& se) {
        se.segs.clear();
        se.ev.clear(); se.evMark.clear(); se.evSlot.clear();
        se.el.clear(); se.elMark.clear(); se.elSlot.clear();
        seSlotZ_.clear();
        for (const Segment& sg : segs) {
            kern::SeSegV v{};
            v.zcode = sg.zcode ? 1 : 0;
            if (sg.zcode) {
                v.el0 = static_cast<int>(se.el.size());
                v.nel = static_cast<int>(sg.elems.size());
                for (size_t j = 0; j < sg.elems.size(); ++j) {
                    const int idx = sg.elems[j];
                    se.el.push_back(idx);
                    se.elMark.push_back(sg.marks[j]);
                    int slot = -1;
                    if (sg.marks[j] == -2) {
                        slot = static_cast<int>(seSlotZ_.size());
                        seSlotZ_.push_back(lat_.elemLength[static_cast<size_t>(idx)] + lat_.elemZStart[static_cast<size_t>(idx)]);
                    }
                    se.elSlot.push_back(slot);
                }
            } else {
                v.ev0 = static_cast<int>(se.ev.size());
                v.nev = static_cast<int>(sg.ev.size());
                for (size_t j = 0; j < sg.ev.size(); ++j) {
                    se.ev.push_back(sg.ev[j]);
                    se.evMark.push_back(sg.marks[j]);
                    int slot = -1;
                    if (sg.marks[j] == -1) {
                        slot = static_cast<int>(seSlotZ_.size());
                        seSlotZ_.push_back(sg.ev[j]);
                    }
                    se.evSlot.push_back(slot);
                }
            }
            se.segs.push_back(v);
        }
        se.elems.assign(lat_.elements.size(), kern::SeElemV{});
        for (size_t k = 0; k < lat_.elements.size(); ++k) {
            const Element& e = lat_.elements[k];
            kern::SeElemV& v = se.elems[k];
            v.kind = static_cast<int>(e.kind);
            v.hv = e.hv;
            v.strength = e.strength;
            v.radius = e.radius;
            v.z0 = lat_.elemZStart[k];
            v.L = lat_.elemLength[k];
            // SeZRun passes the entrance and exit z and SeHardEdgeTrans takes their difference as
            // the length, which can differ from the element length in the last bit
            v.len = compat_ ? (v.z0 + v.L) - v.z0 : v.L;
            if (e.kind == ElemKind::Edge) edge_k(e, v.k1, v.k2);
            if (e.kind == ElemKind::Bend) {
                double arc = e.arc;
                if (v.len < arc * 0.85) arc = arc * 0.5;  // AVAS.dll: a short element is half a bend
                v.arc = arc;
                v.h = e.h;
                v.kx = e.kx;
                v.ky = e.ky;
                dll_sincos(e.kx * arc, v.sx, v.cx);
                dll_sincos(e.ky * arc, v.sy, v.cy);
            }
        }
        se.nSlots = static_cast<int>(seSlotZ_.size());
    }

    kern::SeV se_view(const SeHost& se) const {
        kern::SeV v{};
        v.segs = se.segs.data();
        v.nSeg = static_cast<int>(se.segs.size());
        v.ev = se.ev.data();
        v.evMark = se.evMark.data();
        v.evSlot = se.evSlot.data();
        v.el = se.el.data();
        v.elMark = se.elMark.data();
        v.elSlot = se.elSlot.data();
        v.elems = se.elems.data();
        v.bendRef = bendRef_.data();
        v.maxSec = maxSec_;
        v.errOn = lat_.sample && (lat_.sample->flags[2] + lat_.sample->flags[3] != 0) ? 1 : 0;
        v.boundary = p_.boundary == 1 && lat_.hasBoundary ? 1 : 0;
        v.compat = compat_ ? 1 : 0;
        v.L = lat_.total_length();
        return v;
    }

    void run_secondary() {
        const int n = b_.n;
        say(Text().s("The total distance simulated is ").g(lat_.total_length()).s(" (m)").str());
        say("Start simulation, start time " + time_stamp());
        const std::string& out = paths().output;
        for (const char* f : {"DataSet.txt", "synParticle.txt", "SeSynParticle.txt", "testP.txt", "collisionData.txt"})
            write_file(out + f, "", false);
        setup_errors();
        if (p_.dumpPeriodicity != 0) {
            std::string h;
            h.push_back(static_cast<char>(0x7D));
            h.push_back(static_cast<char>(0x64));
            const std::int32_t d32 = p_.dumpPeriodicity, n32 = n;
            const double cur = b_.current * 1000.0, fm = b_.frequency / 1000000.0, mc2 = b_.restMass[n];
            h.append(reinterpret_cast<const char*>(&d32), 4);
            h.append(reinterpret_cast<const char*>(&n32), 4);
            h.append(reinterpret_cast<const char*>(&cur), 8);
            h.append(reinterpret_cast<const char*>(&fm), 8);
            h.append(reinterpret_cast<const char*>(&mc2), 8);
            if (!write_file(out + "BeamSet.plt", h, false, true)) throw EngineError("BeamSet.plt open failed, Check permissions");
        }
        std::vector<Segment> segs = segment_lattice();
        for (Segment& sg : segs) {
            if (sg.zcode) continue;
            // spacechargecomp events do not exist in SeRun
            std::vector<double> ev;
            std::vector<int> mk;
            for (size_t j = 0; j < sg.ev.size(); ++j)
                if (sg.marks[j] != -4) { ev.push_back(sg.ev[j]); mk.push_back(sg.marks[j]); }
            sg.ev = ev;
            sg.marks = mk;
        }
        bendRef_.assign(lat_.elements.size(), 0.0);
        oldCoord_.assign(static_cast<size_t>(n) + 1, std::array<double, 6>{});
        maxSec_ = 0;
        SeHost se;
        build_se(segs, se);
        latV_ = latH_.view(lat_);
        CpuSeSink sink{this};
        {
            const kern::SeV V = se_view(se);
            const kern::PartV P = pview();
            if (compat_) kern::se_track<DllMath>(latV_, P, V, n, true, sink, bendRef_.data());
            else kern::se_track<PortableMath>(latV_, P, V, n, true, sink, bendRef_.data());
        }
        say("Synchronized particles have completed simulation.");
        maxSec_ *= 10;
        dev_ = make_device_backend(n, false);
        if (dev_) {
            // every particle alone through the lattice on the device; outputs written in particle order
            dev_->begin_run(b_, latH_, false);
            SeOut res;
            dev_->secondary(lat_scalars(), se, se_view(se), res);
            dev_->download(b_, nullptr, nullptr, nullptr, kDevCoord | kDevLost);
            dev_->download_times(b_);
            std::sort(res.hits.begin(), res.hits.end(), [](const BoundaryHit& x, const BoundaryHit& y) { return x.i < y.i; });
            size_t h = 0;
            for (int i = 0; i < n; ++i) {
                for (int q = 0; q < se.nSlots; ++q) {
                    const double* S = res.planes.data() + (static_cast<size_t>(i) * se.nSlots + q) * 8;
                    if (S[7] == 1.0) se_plane_out(i, S, seSlotZ_[static_cast<size_t>(q)], false);
                }
                for (; h < res.hits.size() && res.hits[h].i == i; ++h)
                    sink.hit(res.hits[h].geom, res.hits[h].code, res.hits[h].P, res.hits[h].tHit, i);
            }
        } else {
            const kern::SeV V = se_view(se);
            const kern::PartV P = pview();
            for (int i = 0; i < n; ++i) {
                if (stop_requested()) throw EngineError("Programs terminated by users.                       ");
                if (compat_) kern::se_track<DllMath>(latV_, P, V, i, false, sink, nullptr);
                else kern::se_track<PortableMath>(latV_, P, V, i, false, sink, nullptr);
                char line[160];
                std::snprintf(line, sizeof line, "Simulating the %d particle.  total number of particles %d.                          \r", i, n);
                say_raw(line);
            }
        }
        int alive = 0;
        for (int i = 0; i < n; ++i)
            if (b_.lost[i] != 1) ++alive;
        say("Simulation completed, completion time " + time_stamp() + "   Number of particles not lost: " +
            std::to_string(alive) + std::string(77, ' '));
    }

    void se_syn_out(int i) {
        const Beam& b = b_;
        Text t;
        for (double v : {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i], b.bx[i], b.by[i], b.bz[i]})
            t.fixed(v, 12).s("  ");
        t.i(b.lost[i]).nl();
        write_file(paths().output + "SeSynParticle.txt", t.str(), true);
    }

    void se_plane_out(int i, const double* S, double z, bool isSync) {
        char zn[64];
        std::snprintf(zn, sizeof zn, "%f", z);
        const std::string f = paths().output + "outData_" + zn + ".txt";
        if (isSync) {
            write_file(f, "", false);
            return;
        }
        Text t;
        t.i(b_.id[i]).s(" ");
        for (int j : {0, 1, 2, 3, 6}) t.fixed(S[j], 15).s("  ");
        t.fixed(S[5], 15).nl();
        write_file(f, t.str(), true);
    }

    Lattice& lat_;
    Beam& b_;
    Params& p_;
    bool compat_;
    double dt_ = 0.0;
    int step_ = 0;
    double pushTime_ = 0.0;
    double zStartRun_ = 0.0;
    std::chrono::steady_clock::time_point startClock_;
    double lastProgressPct_ = 0.0;   // default mode: percentage of the last progress line
    std::vector<Rec> planeRec_;
    std::vector<Rec> spRec_;  // superposeout exit crossings
    double segEnd_ = 1e300;   // end of the current time-step segment
    std::unique_ptr<SpaceCharge> sc_;
    std::vector<std::array<double, 6>> oldCoord_;
    AppendFile dataSet_, synFile_;

    // ---- shared particle kernels (kernels.h): lattice view, RF table, particle view ----
    LatHost latH_;
    kern::LatV latV_{};
    std::vector<double> rfTab_;

    kern::PartV pview() {
        kern::PartV v = beam_view(b_);
        v.old = oldCoord_.empty() ? nullptr : oldCoord_[0].data();
        v.rec = planeRec_.empty() ? nullptr : planeRec_[0].data();
        v.spRec = spRec_.empty() ? nullptr : spRec_[0].data();
        return v;
    }

    // Lattice view (superposeout state, element rotMode) and the RF factors of a step at time t.
    void prepare(double t) {
        if (latH_.sync_dynamic(lat_) && dev_) dev_->set_elems(latH_);
        latV_ = latH_.view(lat_);
        latH_.rf_table(t, rfTab_);
    }

    // ---- device back end (device.h): null on the CPU ----
    std::unique_ptr<DeviceBackend> dev_;
    long long devRecCount_ = 0;
    bool detach_device_rows_ = false;  // a host-side restart works on downloaded rows

    void dl(unsigned m) {
        if (dev_) dev_->download(b_, oldCoord_.empty() ? nullptr : oldCoord_[0].data(), planeRec_[0].data(),
                                 spRec_[0].data(), m);
    }
    void ul(unsigned m) {
        if (dev_) dev_->upload(b_, oldCoord_.empty() ? nullptr : oldCoord_[0].data(), planeRec_[0].data(),
                               spRec_[0].data(), m);
    }
    LatScalars lat_scalars() const {
        return LatScalars{lat_.spOutCur, lat_.spOutCount, static_cast<int>(lat_.spOut.size()), lat_.total_length(),
                          lat_.sp_exit()};
    }

    // One space-charge solve (t-code step or z-code element middle).
    void sc_solve(const double* sync) {
        Beam& b = b_;
        SpaceCharge& sc = *sc_;
        if (!dev_) {
            if (sc.importing()) sc.import_scf(b);
            else sc.solve(b, b.scComp, sync);
            return;
        }
        if (sc.importing()) {
            // importSCF reads the recorded fields on the host
            dl(kDevCoord | kDevLost);
            sc.import_scf(b);
            ul(kDevSc);
            return;
        }
        if (!sc.begin(b, sync)) return;
        sc.mesh(b);
        sc.prepare_fix(b);
        kern::MeshV M = sc.mesh_view(b);
        std::vector<long long> acc;
        dev_->sc_deposit(M, acc);
        sc.set_rho(acc, b.scComp);
        ScSolveIn in{};
        in.method = sc.method();
        in.nx = sc.nx();
        in.ny = sc.ny();
        in.nz = sc.nz();
        in.mx = sc.mx();
        in.my = sc.my();
        in.mz = sc.mz();
        in.rho = sc.rho().data();
        in.dx = M.dx;
        in.dy = M.dy;
        in.dz = M.dz;
        std::vector<double> lx, ly, lz;
        if (in.method == 1 || in.method == 2) {
            in.vol = sc.cell_volume();
            in.thr = in.method == 1 ? sc.spicnic_threshold() : 0.0;
        } else {
            sc.fft_tables(lx, ly, lz, in.scale, in.norm);
            in.lx = lx.data();
            in.ly = ly.data();
            in.lz = lz.data();
        }
        dev_->sc_solve(in);
        const double Q = sc.total_charge();
        M.Q = Q;
        dev_->sc_interp(M);
        if (sc.recording()) {
            dev_->sc_fields(sc.ex(), sc.ey(), sc.ez());
            sc.record_after(b, Q);
        }
    }

    // HESCFieldPre on the device (corrected engine).
    void dev_hesc(double tRef) {
        Beam& b = b_;
        double s[kern::kHescSums], q[3];
        dev_->hesc1(tRef, s);
        const double cnt = s[6];
        for (int k = 0; k < 6; ++k) b.centroid[k] = s[k] / cnt;
        const double* c = b.centroid.data();
        const double gc = 1.0 / std::sqrt(1.0 - std::pow(c[5] / std::sqrt(c[1] * c[1] + 1.0 + c[3] * c[3] + c[5] * c[5]), 2.0));
        dev_->hesc2(c, gc, q);
        for (int k = 0; k < 3; ++k) b.rms[k] = std::sqrt(q[k] / cnt);
    }

    // Field of particle i in element k at the particle's own time (secondary particles).
    void element_field_at(int i, double t, int k, bool err) {
        latH_.sync_dynamic(lat_);
        latV_ = latH_.view(lat_);
        const kern::RfSrc rf{nullptr, t};
        const kern::PartV P = pview();
        if (compat_) {
            if (err) kern::element_err_field<DllMath>(latV_, P, i, k, rf);
            else kern::element_field<DllMath>(latV_, P, i, k, rf);
        } else {
            if (err) kern::element_err_field<PortableMath>(latV_, P, i, k, rf);
            else kern::element_field<PortableMath>(latV_, P, i, k, rf);
        }
    }

    // ---- error samples (spec 70 6, 7) ----
    std::vector<char> errUse_;  // t-code: element uses GetErrField
    std::vector<char> hetErr_;  // z-code: entrance / exit transform of the misalignment
    std::vector<Element> spEff_;  // corrected engine: superpose blocks with the sub-field errors applied

    void setup_errors() {
        setup_error_flags();
        latH_.build(lat_, errUse_, hetErr_, spEff_);
    }

    void setup_error_flags() {
        errUse_.clear();
        hetErr_.clear();
        const ErrorSample* es = lat_.sample;
        if (!es) return;
        // MethodMP::Run: errSampleOut, then SetErrInEle
        write_error_sample(*es);
        if (es->flags[2] + es->flags[3] == 0) return;
        set_errors_in_elements(lat_, *es);
        const size_t ne = lat_.elements.size();
        auto marked = [](const std::vector<std::vector<double>>& rows, size_t k) {
            return rows.size() > k + 1 && !rows[k + 1].empty() && rows[k + 1][0] == 1.0;
        };
        errUse_.assign(ne, 0);
        hetErr_.assign(ne, 0);
        spEff_.assign(ne, Element{});
        for (size_t k = 0; k < ne; ++k) {
            const Element& e = lat_.elements[k];
            if (e.kind == ElemKind::Field && e.fieldType == 4 && !e.subErr.empty()) {
                // corrected engine: the error of a sub-field moves and scales that sub-field
                // (displacement added to its placement, angles composed by addition)
                Element f = e;
                bool any = false;
                for (size_t q = 0; q < e.subErr.size(); ++q) {
                    const auto& se = e.subErr[q];
                    if (se[0] != 1.0) continue;
                    any = true;
                    f.subZStart[q] += se[3];
                    f.subPlace[q][0] += se[3];
                    f.subPlace[q][1] += se[1];
                    f.subPlace[q][2] += se[2];
                    f.subPlace[q][3] += se[6];
                    f.subPlace[q][4] += se[4];
                    f.subPlace[q][5] += se[5];
                    f.subKb[q] = se[7];
                    f.subKe[q] = se[8];
                    if (e.subType[q] == 2) f.phaseErr = se[9];
                }
                spEff_[k] = f;
                if (!compat_ && any) errUse_[k] = 1;
            }
        }
        for (size_t k = 0; k < ne; ++k) {
            if (!compat_ && lat_.elements[k].fieldType == 4 && lat_.elements[k].kind == ElemKind::Field) continue;
            if (compat_) {
                // AVAS.dll looks at the static row when static element errors are on (a field map
                // with only a dynamic error then uses GetField), and the z-code at the dynamic row
                errUse_[k] = es->flags[2] ? marked(es->stat, k) : marked(es->dyn, k);
                hetErr_[k] = marked(es->dyn, k);
            } else {
                errUse_[k] = hetErr_[k] = marked(es->stat, k) || marked(es->dyn, k);
            }
        }
    }

    // ---- segmentation of the lattice into time-step and hard-edge segments (MethodMP::Run) ----
    struct Segment {
        bool zcode = false;
        std::vector<double> ev;  // t segment: event positions
        std::vector<int> elems;  // z segment: element indices
        std::vector<int> marks;
    };

    // The original segments by type name: an element left with the default name "drift" after
    // "parameter not enough" (compat=dll) is a time-step element.
    static bool is_z(const Element& e) {
        return e.typeName == "quad" || e.typeName == "solenoid" || e.typeName == "bend" || e.typeName == "edge";
    }

    std::vector<Segment> segment_lattice() const {
        std::vector<Segment> segs;
        const auto& P = p_.outputPlanes;
        const int ne = static_cast<int>(lat_.elements.size());
        auto zS = [&](int i) { return lat_.elemZStart[i]; };
        auto zE = [&](int i) { return lat_.elemZStart[i] + lat_.elemLength[i]; };
        auto isSteerer = [&](int i) { return lat_.elements[i].typeName == "steerer"; };
        int i = 0;
        bool flag = false;
        while (i < ne) {
            Segment sg;
            if (is_z(lat_.elements[i])) {
                sg.zcode = true;
                while (true) {
                    if (!is_z(lat_.elements[i])) break;
                    sg.elems.push_back(i);
                    if (flag) {
                        sg.marks.push_back(i - 1);
                        flag = false;
                    } else {
                        for (double pl : P) {
                            if (pl == zE(i)) { sg.marks.push_back(-2); break; }
                            if (zS(i) < pl && pl < zE(i)) { sg.marks.push_back(-1); break; }
                        }
                    }
                    if (sg.marks.size() < sg.elems.size()) sg.marks.push_back(-3);
                    i += 1;
                    if (i >= ne) break;
                    if (isSteerer(i)) {
                        i += 1;
                        flag = true;
                    }
                }
            } else {
                std::vector<std::pair<double, int>> ev;
                while (true) {
                    if (i >= ne || is_z(lat_.elements[i])) break;
                    if (flag) {
                        ev.push_back({zS(i) + 0.5 * lat_.elemLength[i], i - 1});
                        flag = false;
                    }
                    for (double pl : P)
                        if (zS(i) < pl && pl <= zE(i)) ev.push_back({pl, -1});
                    for (const auto& row : lat_.spaceChargeComp)
                        if (zS(i) <= row[0] && row[0] < zE(i)) ev.push_back({zS(i), -4});
                    i += 1;
                    if (i >= ne) break;
                    if (isSteerer(i)) {
                        i += 1;
                        flag = true;
                    }
                }
                std::stable_sort(ev.begin(), ev.end(),
                                 [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
                                     return a.first < b.first;
                                 });
                for (const auto& e : ev) {
                    sg.ev.push_back(e.first);
                    sg.marks.push_back(e.second);
                }
                const int last = std::min(i, ne) - 1;
                const double zEnd = zE(last);
                if (sg.ev.empty() || zEnd != sg.ev.back()) {
                    sg.ev.push_back(zEnd);
                    sg.marks.push_back(-2);
                }
            }
            segs.push_back(std::move(sg));
        }
        if (segs.empty() || segs.front().zcode) {
            throw EngineError("Do not allow the first element to be a hard-edged model element; very short drift "
                              "can be added at the beginning");
        }
        return segs;
    }

    // Steerer kick on z-code row r of particle i; lq0 = length of the kicked element.
    void kick(double* r, const Element& st, double len, int i) const {
        // AVAS.dll uses the charge-to-mass ratio of particle 0 for every particle
        const double qm = compat_ ? b_.qOverM[0] : b_.qOverM[i];
        kern::kick_row(r, st.steerKind, st.steerX, st.steerY, len * qm, compat_);
    }

    // ---- hard-edge transfer of the z-code rows (MethodMP::HardEdgeTrans) ----
    BendStep<double> bend_step(const Element& e, double zFrom, double zTo, bool half) const {
        BendStep<double> s{};
        s.s = e.arc;
        // AVAS.dll recognises a half step by its length, so a bend written with L = 0 (or any
        // L < 0.85 arc) only travels half of its arc; the corrected engine is told explicitly.
        if (compat_ ? (zTo - zFrom) < e.arc * 0.85 : half) s.s = e.arc * 0.5;
        s.h = e.h;
        s.kx = e.kx;
        s.kx2 = e.kx * e.kx;
        s.ky = e.ky;
        s.hv = e.hv == 0 ? 0 : 1;
        dll_sincos(s.s * e.kx, s.sx, s.cx);
        dll_sincos(s.s * e.ky, s.sy, s.cy);
        return s;
    }

    void edge_k(const Element& e, double& k1, double& k2) const {
        k1 = std::tan(e.poleAngle) / e.rho;
        k2 = -(std::tan(e.poleAngle - e.psi) / e.rho);
    }

    // Transfer description of element idx for the rows (kind < 0: no transfer).
    kern::RowOp row_op(int idx, double zFrom, double zTo, bool half) const {
        const Element& e = lat_.elements[idx];
        kern::RowOp op{};
        op.kind = -1;
        op.len = zTo - zFrom;
        op.err = !hetErr_.empty() && hetErr_[static_cast<size_t>(idx)] ? 1 : 0;
        op.e = latH_.elems[static_cast<size_t>(idx)];
        op.uzs = planeRec_[static_cast<size_t>(b_.n)][5];  // synchronous gamma*beta_z (bend; time scale of the fp32 rows)
        if (e.kind == ElemKind::Edge) {
            op.kind = kern::kEdge;
            edge_k(e, op.k1, op.k2);
            op.hv = e.hv == 0 ? 0 : 1;
        } else if (e.kind == ElemKind::Quad) {
            op.kind = kern::kQuad;
            op.strength = e.strength;
        } else if (e.kind == ElemKind::Solenoid) {
            op.kind = kern::kSolenoid;
            op.strength = e.strength;
        } else if (e.kind == ElemKind::Bend) {
            op.kind = kern::kBend;
            op.bs = bend_step(e, zFrom, zTo, half);
            op.uzs = planeRec_[static_cast<size_t>(b_.n)][5];
        }
        return op;
    }

    void het(WorkerPool& pool, int idx, double zFrom, double zTo, bool half, int& mode, double& tRef) {
        const int n = b_.n;
        auto& rec = planeRec_;
        kern::RowOp op = row_op(idx, zFrom, zTo, half);
        if (op.kind < 0) return;
        auto apply = [&](const kern::RowOp& o) {
            if (dev_) {
                dev_->rows(o);
                return;
            }
            for_rows(pool, [&](int i) {
                if (compat_) kern::het_row<DllMath>(o, rec[i].data(), b_.qOverM[i], b_.scx[i], b_.scy[i], b_.scz[i]);
                else kern::het_row<PortableMath>(o, rec[i].data(), b_.qOverM[i], b_.scx[i], b_.scy[i], b_.scz[i]);
            });
        };
        if (op.kind == kern::kEdge) {
            apply(op);
            return;
        }
        op.scKick = mode == 1 ? 1 : 0;
        const bool average = mode == 2;
        mode = 0;
        apply(op);
        if (dev_) {
            dev_->rows_tref(average, tRef);
        } else if (average) {
            if (compat_) {
                // AVAS.dll sums with atomics in thread completion order (last bits not reproducible);
                // the value is always replaced by the second half-step that follows.
                double sT = 0.0, sC = 0.0;
                for (int i = 0; i < n; ++i)
                    if (rec[i][7] == 1.0) { sT = sT + rec[i][6]; sC = sC + 1.0; }
                tRef = sT / sC;
            } else {
                double s2[2];
                fixed_sum<2>(n, serial_run, [&](int i, double* c) {
                    const bool on = rec[i][7] == 1.0;
                    c[0] = on ? rec[i][6] : 0.0;
                    c[1] = on ? 1.0 : 0.0;
                }, s2);
                tRef = s2[0] / s2[1];
            }
        } else {
            tRef = 1e10;
            for (int i = 0; i < n; ++i)
                if (rec[i][7] == 1.0 && rec[i][6] < tRef) tRef = rec[i][6];
        }
    }

    static void serial_run(int ng, const std::function<void(int)>& fn) {
        for (int g = 0; g < ng; ++g) fn(g);
    }

    template <class F>
    void for_rows(WorkerPool& pool, F&& f) {
        const int n = b_.n;
        const int nb = std::max(1, std::min(n, 64));
        const int chunk = (n + nb - 1) / nb;
        pool.run(nb, [&](int kb) {
            const int i1 = std::min(n, (kb + 1) * chunk);
            for (int i = kb * chunk; i < i1; ++i)
                if (planeRec_[i][7] == 1.0) f(i);
        });
    }

    // Synchronous row through the element (whole length, or the first half when half is set).
    void sync_het(const Element& e, double* r, bool half) const {
        const double qm = b_.qOverM[b_.n];
        auto sc = [](double a, double& s, double& c) { dll_sincos(a, s, c); };
        const double len = half ? e.length * 0.5 : e.length;
        const double p2 = zrow_p2(r);
        sync_het_raw(e, r, half, qm, len, sc);
        if (!compat_) zrow_keep_momentum(r, p2);
    }

    template <class SC>
    void sync_het_raw(const Element& e, double* r, bool half, double qm, double len, SC sc) const {
        switch (e.kind) {
            case ElemKind::Quad: quad_sync_row(r, qm, e.strength, len, sc); break;
            case ElemKind::Solenoid: solenoid_row(r, qm, e.strength, len, sc); break;
            case ElemKind::Bend: {
                BendStep<double> bs = bend_step(e, 0.0, e.arc, false);
                if (half) {
                    bs.s = e.arc * 0.5;
                    dll_sincos(bs.s * e.kx, bs.sx, bs.cx);
                    dll_sincos(bs.s * e.ky, bs.sy, bs.cy);
                }
                bend_sync_row(r, bs);
                break;
            }
            case ElemKind::Edge: {
                double k1, k2;
                edge_k(e, k1, k2);
                if (!half) edge_row(r, k1, k2, e.hv == 0 ? 0 : 1);
                break;
            }
            default: break;
        }
    }

    // zTotcode: rows at plane zPlane back to particles at time tRef; returns tRef.
    double ztot(double zPlane, double tRef) {
        Beam& b = b_;
        const int n = b.n;
        const bool onDev = dev_ && !detach_device_rows_;
        Rec& r = planeRec_[n];
        double x, ux, y, uy, z, uz;
        ztot_row(r.data(), zPlane, tRef, !compat_, x, ux, y, uy, z, uz);
        if (onDev) {
            const double sync[6] = {0.0, 0.0, 0.0, 0.0, z, r[5]};  // the synchronous particle set below
            dev_->ztot(zPlane, tRef, sync);
        }
        for (int i = 0; i < (onDev ? 0 : n); ++i) {
            b.nextPlane[i] = 0;
            Rec& q = planeRec_[i];
            if (q[7] == 1.0)
                ztot_row(q.data(), zPlane, tRef, !compat_, b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]);
            q.fill(0.0);
        }
        b.z[n] = z;
        b.x[n] = b.ux[n] = b.y[n] = b.uy[n] = 0.0;
        b.uz[n] = r[5];
        b.nextPlane[n] = 0;
        r.fill(0.0);
        return tRef;
    }

    // ---- a hard-edge segment (MethodMP::zcodeRun) ----
    void zcode(const Segment& sg, double& t, double& zSeg) {
        const int n = b_.n;
        int threads = options().threads > 0 ? options().threads : static_cast<int>(std::thread::hardware_concurrency());
        threads = std::max(1, std::min(threads, 64));
        WorkerPool pool(threads > 1 ? threads : 0);
        double tRef = 0.0;
        double zEnd = 0.0;
        for (size_t j = 0; j < sg.elems.size(); ++j) {
            const int idx = sg.elems[j];
            const Element& e = lat_.elements[idx];
            const double z0 = lat_.elemZStart[idx];
            const double L = lat_.elemLength[idx];
            const int mark = sg.marks[j];
            int mode = 0;
            const bool split = (p_.spaceCharge == 1 || mark > -1.5) && e.kind != ElemKind::Edge;
            if (!split) {
                het(pool, idx, z0, z0 + L, false, mode, tRef);
            } else {
                mode = 2;
                const double zMid = L * 0.5 + z0;
                het(pool, idx, z0, zMid, true, mode, tRef);
                step_ += 1;
                if (sc_) {
                    if (dev_) dev_hesc(tRef);
                    else hesc_field_pre(b_, planeRec_, tRef, p_.cores[2]);
                    const double syncRow[6] = {planeRec_[n][0], planeRec_[n][1], planeRec_[n][2], planeRec_[n][3], 0.0, planeRec_[n][5]};
                    // recordSCF centre: AVAS.dll keeps the synchronous coordinates of the last
                    // t-code step (only the SpaceChargeMesh.txt line uses them)
                    sc_solve(compat_ ? oldCoord_[n].data() : syncRow);
                    mode = 1;
                }
                if (mark == -1) {
                    dl(kDevRec);
                    if (compat_) {
                        dst_out(zMid, planeRec_);
                    } else {
                        // the reference time of the dst file is the synchronous particle at the
                        // middle of the element (AVAS.dll uses its entrance time)
                        std::vector<Rec> copy = planeRec_;
                        sync_het(e, copy[n].data(), true);
                        dst_out(zMid, copy);
                    }
                } else if (mark >= 0) {
                    if (dev_) {
                        const Element& st = lat_.elements[mark];
                        dev_->rows_kick(st.steerKind, st.steerX, st.steerY, e.length);
                    }
                    for (int i = 0; i < (dev_ ? 0 : n); ++i)
                        if (planeRec_[i][7] == 1.0) kick(planeRec_[i].data(), lat_.elements[mark], e.length, i);
                }
                het(pool, idx, zMid, L + z0, true, mode, tRef);
            }
            sync_het(e, planeRec_[n].data(), false);
            zEnd = L + z0;
            if (p_.dumpPeriodicity != 0) {
                dl(kDevRec);
                beam_out(1, tRef, zEnd, step_);
            }
            step_ += 1;
            if (mark == -2) {
                dl(kDevRec);
                dst_out(zEnd, planeRec_);
            }
        }
        t = ztot(zEnd, tRef);
        zSeg = zEnd;
    }

    // ---- one time step of particles [i0, i1) (TcodeTrans); prepare(tEnd) must have been called ----
    kern::StepArgs step_args(double zField, const std::vector<double>& events, double tEnd, double gp,
                             const double sync[6]) const {
        kern::StepArgs a{};
        a.dt = dt_;
        a.tEnd = tEnd;
        a.zField = zField;
        a.zMinPrev = b_.zMin;
        a.gp = gp;
        a.segEnd = segEnd_;
        for (int j = 0; j < 6; ++j) {
            a.sync[j] = sync[j];
            a.cen[j] = b_.centroid[j];
        }
        a.events = events.data();
        a.nEvents = static_cast<int>(events.size());
        a.rotMode = b_.rotMode;
        a.rs = 0.0;
        a.rc = 1.0;
        if (b_.rotMode != 0) dll_sincos(b_.rotAngle, a.rs, a.rc);
        const int n = b_.n;
        if (!oldCoord_.empty())
            for (int j = 0; j < 6; ++j) a.syncPre[j] = oldCoord_[static_cast<size_t>(n)][j];
        for (int j = 0; j < 6; ++j) a.syncTot[j] = syncTot_[j];
        a.qmSync = b_.qOverM[n];
        a.writeOld = lat_.hasBoundary && p_.boundary == 1 ? 1 : 0;
        return a;
    }

    static void accumulate(BlockAcc& acc, const kern::Contrib& c) {
        if (c.hasA) {
            if (c.zmin < acc.zmin) acc.zmin = c.zmin;
            if (acc.zmax < c.zmax) acc.zmax = c.zmax;
            acc.sumGz2 = c.s[9] + acc.sumGz2;
            for (int j = 0; j < 6; ++j) {
                if (acc.maxAbs[j] < c.mx[j]) acc.maxAbs[j] = c.mx[j];
                acc.sum2[j] = c.s[j] + acc.sum2[j];
            }
            for (int j = 0; j < 3; ++j) acc.cross[j] = c.s[6 + j] + acc.cross[j];
        }
        acc.spCnt += c.sp;
        if (c.hasB) {
            acc.cnt0 = acc.cnt0 + 1.0;
            acc.cnt1 = c.s[11] + acc.cnt1;
            for (int j = 0; j < 6; ++j) acc.sum1[j] = c.s[12 + j] + acc.sum1[j];
            acc.gsum = c.s[18] + acc.gsum;
        }
    }

    // Field of the push of the synchronous particle (the fp32 GPU path follows the bunch relative to it).
    double syncTot_[6] = {0, 0, 0, 0, 0, 0};
    void capture_sync_tot(double zField) {
        const Beam& b = b_;
        const int n = b.n;
        if (b.zMin <= zField && !(zField < b.z[n])) {
            for (double& v : syncTot_) v = 0.0;
        } else {
            syncTot_[0] = b.ex[n] + b.scx[n]; syncTot_[1] = b.ey[n] + b.scy[n]; syncTot_[2] = b.ez[n] + b.scz[n];
            syncTot_[3] = b.bx[n] + 0.0; syncTot_[4] = b.by[n] + 0.0; syncTot_[5] = b.bz[n] + 0.0;
        }
    }

    void trans(int i0, int i1, double zField, const std::vector<double>& events, double tEnd, double gp,
               const double sync[6], BlockAcc& acc) {
        const kern::StepArgs a = step_args(zField, events, tEnd, gp, sync);
        const kern::PartV P = pview();
        const kern::RfSrc rf{rfTab_.data(), tEnd};
        for (int i = i0; i < i1; ++i) {
            kern::Contrib c;
            if (compat_) kern::trans_particle<DllMath>(latV_, P, i, a, rf, c);
            else kern::trans_particle<PortableMath>(latV_, P, i, a, rf, c);
            accumulate(acc, c);
        }
    }

    // Corrected engine: the step of the particles of group g (4096 particles) with the statistics
    // summed in the fixed order of reduce.h; the group result goes to acc (one block per group).
    void trans_group(int g, double zField, const std::vector<double>& events, double tEnd, double gp,
                     const double sync[6], BlockAcc& acc) {
        const kern::StepArgs a = step_args(zField, events, tEnd, gp, sync);
        const kern::PartV P = pview();
        const kern::RfSrc rf{rfTab_.data(), tEnd};
        acc.reset();
        double s[kern::kStepSums];
        group_sum<kern::kStepSums>(g * kRedGroup, b_.n, [&](int i, double* out) {
            kern::Contrib c;
            kern::trans_particle<PortableMath>(latV_, P, i, a, rf, c);
            kern::contrib_sums(c, out);
            if (c.hasA) {
                if (c.zmin < acc.zmin) acc.zmin = c.zmin;
                if (acc.zmax < c.zmax) acc.zmax = c.zmax;
                for (int j = 0; j < 6; ++j)
                    if (acc.maxAbs[j] < c.mx[j]) acc.maxAbs[j] = c.mx[j];
            }
            acc.spCnt += c.sp;
        }, s);
        set_block_sums(acc, s);
    }

    static void set_block_sums(BlockAcc& acc, const double* s) {
        for (int j = 0; j < 6; ++j) {
            acc.sum2[j] = s[j];
            acc.sum1[j] = s[12 + j];
        }
        for (int j = 0; j < 3; ++j) acc.cross[j] = s[6 + j];
        acc.sumGz2 = s[9];
        acc.cnt0 = s[10];
        acc.cnt1 = s[11];
        acc.gsum = s[18];
    }


    void ubp_finish(const double* t1) {
        Beam& b = b_;
        b.nAlive = static_cast<int>(t1[8]);
        b.totalWeight = t1[0];
        for (int j = 0; j < 6; ++j) b.centroid[j] = t1[1 + j] / b.totalWeight;
        b.gamma = t1[7] / b.totalWeight;
        const double bz = b.centroid[5] / b.gamma;
        b.betaGammaLambda = (k::c / b.frequency) * bz * (1.0 / std::sqrt(1.0 - bz * bz));
    }

    // Centroid, mean gamma, zMin/zMax at the start of a time-step segment (UpdateBeamParameters).
    void update_beam_parameters() {
        Beam& b = b_;
        const int n = b.n;
        if (dev_) {
            const double s6[6] = {b.x[n], b.ux[n], b.y[n], b.uy[n], b.z[n], b.uz[n]};
            double t1[kern::kUbpSums], q[3];
            dev_->ubp1(s6, t1);
            ubp_finish(t1);
            const double bz = b.centroid[5] / b.gamma;
            const double igz = 1.0 / std::sqrt(1.0 - bz * bz);
            dev_->ubp2(b.centroid.data(), igz, q, b.zMin, b.zMax);
            for (int j = 0; j < 3; ++j) b.rms[j] = std::sqrt(q[j] / b.totalWeight);
            return;
        }
        if (!compat_) {
            // corrected engine: sums in the fixed order of reduce.h
            const kern::PartV P = pview();
            const double s6[6] = {b.x[n], b.ux[n], b.y[n], b.uy[n], b.z[n], b.uz[n]};
            double t1[kern::kUbpSums];
            fixed_sum<kern::kUbpSums>(n, serial_run, [&](int i, double* c) { kern::ubp_pass1(P, i, s6, c); }, t1);
            ubp_finish(t1);
            double q[3];
            const double bz = b.centroid[5] / b.gamma;
            const double igz = 1.0 / std::sqrt(1.0 - bz * bz);
            fixed_sum<3>(n, serial_run, [&](int i, double* c) { kern::ubp_pass2(P, i, b.centroid.data(), igz, c); }, q);
            b.zMin = 1e10;
            b.zMax = -1e10;
            for (int i = 0; i < n; ++i) {
                if (b.z[i] < b.zMin) b.zMin = b.z[i];
                if (b.zMax < b.z[i]) b.zMax = b.z[i];
            }
            for (int j = 0; j < 3; ++j) b.rms[j] = std::sqrt(q[j] / b.totalWeight);
            return;
        }

        b.nAlive = 0;
        b.zMin = 1e10;
        b.zMax = -1e10;
        b.totalWeight = 0.0;
        double s[6] = {0, 0, 0, 0, 0, 0};
        double gsum = 0.0;
        for (int i = 0; i < n; ++i) {
            if (b.lost[i] == 1) continue;
            b.nAlive += 1;
            b.totalWeight = b.weight[i] + b.totalWeight;
            b.rel[0][i] = b.x[i] - b.x[n];
            b.rel[2][i] = b.y[i] - b.y[n];
            b.rel[4][i] = b.z[i] - b.z[n];
            b.rel[1][i] = b.ux[i];
            b.rel[3][i] = b.uy[i];
            b.rel[5][i] = b.uz[i];
            const double w = b.weight[i];
            for (int j = 0; j < 6; ++j) s[j] = s[j] + w * b.rel[j][i];
            if (compat_) {
                gsum = gsum + std::sqrt(b.uz[i] * b.uz[i] + 1.0);
            } else {
                gsum = gsum + std::sqrt(b.ux[i] * b.ux[i] + b.uy[i] * b.uy[i] + b.uz[i] * b.uz[i] + 1.0) * w;
            }
        }
        for (int j = 0; j < 6; ++j) b.centroid[j] = s[j] / b.totalWeight;
        b.gamma = gsum / b.totalWeight;
        const double bz = b.centroid[5] / b.gamma;
        b.betaGammaLambda = (k::c / b.frequency) * bz * (1.0 / std::sqrt(1.0 - bz * bz));
        double q[3] = {0, 0, 0};
        const double igz = 1.0 / std::sqrt(1.0 - bz * bz);
        for (int i = 0; i < n; ++i) {
            if (b.z[i] < b.zMin) b.zMin = b.z[i];
            if (b.zMax < b.z[i]) b.zMax = b.z[i];
            // rms for the first space-charge mesh.  AVAS.dll subtracts the relative centroid from
            // the absolute coordinates (and counts lost particles); corrected: relative ones
            if (!compat_ && b.lost[i] == 1) continue;
            const double w = b.weight[i];
            const double ax = (compat_ ? b.x[i] : b.rel[0][i]) - b.centroid[0];
            const double ay = (compat_ ? b.y[i] : b.rel[2][i]) - b.centroid[2];
            const double az = ((compat_ ? b.z[i] : b.rel[4][i]) - b.centroid[4]) * igz;
            q[0] = w * ax * ax + q[0];
            q[1] = w * ay * ay + q[1];
            q[2] = w * az * az + q[2];
        }
        for (int j = 0; j < 3; ++j) b.rms[j] = std::sqrt(q[j] / b.totalWeight);
    }

    // Beam frame inside a superposeout block (tcodeRun 4.2.15): turned to the synchronous momentum.
    void rot_mode(double sync[6]) {
        Beam& b = b_;
        const int n = b.n;
        Element& e = lat_.elements[b.elemIndex[n]];
        const int rm = e.inSuperposeOut;
        for (int j = 0; j < 6; ++j) sync[j] = 0.0;
        sync[0] = b.x[n]; sync[1] = b.ux[n]; sync[2] = b.y[n]; sync[3] = b.uy[n]; sync[4] = b.z[n]; sync[5] = b.uz[n];
        if (rm == 0) {
            b.rotMode = 0;
            return;
        }
        const double gz = b.uz[n];
        const double gt = rm == 1 ? b.uy[n] : b.ux[n];
        double th, sn, cs;
        if (std::fabs(gz) <= 0.0003) {
            th = gt > 0.0 ? k::half_pi : -k::half_pi;
            cs = 6.123233995736766e-17;
            sn = gt > 0.0 ? 1.0 : -1.0;
        } else {
            th = std::atan(gt / std::fabs(gz));
            dll_sincos(th, sn, cs);
        }
        b.rotAngle = th;
        if (rm == 1) {
            b.rotMode = 1;
            sync[3] = gt * cs + gz * sn;
            sync[5] = cs * gz - gt * sn;
            e.rotMode = 1;
        } else {
            b.rotMode = 2;
            sync[1] = gt * cs - gz * sn;
            sync[5] = sn * gt + gz * cs;
            e.rotMode = 2;
        }
    }

    void syn_out() {
        const Beam& b = b_;
        const int s = b.n;
        const double g = std::sqrt(b.uz[s] * b.uz[s] + 1.0);
        Text t;
        t.s(time_stamp()).fixed(b.z[s], 12).s("  ").fixed((g - 1.0) * b.restMass[s], 12).s("  ").fixed(b.x[s], 12)
            .s("  ").fixed(b.y[s], 12).s("  ").fixed(b.ux[s], 12).s("  ").fixed(b.uy[s], 12).s("  ")
            .fixed(b.uz[s], 12).s("  ").i(b.rotMode).s("  ").fixed(b.rotAngle, 12).s(" ").fixed(b.ex[s], 12)
            .s("  ").fixed(b.ey[s], 12).s("  ").fixed(b.ez[s], 12).s("  ").fixed(b.bx[s], 12).s("  ")
            .fixed(b.by[s], 12).s("  ").fixed(b.bz[s], 12).s("  ").i(b.lost[s]).nl();
        synFile_.write(t.str());
    }

    void single_out(double t) {
        const Beam& b = b_;
        double g, mc2;
        if (compat_) {
            g = std::sqrt(b.uz[0] * b.uz[0] + 1.0);
            mc2 = k::proton_mc2;  // AVAS.dll: proton mass hard-coded, transverse momentum ignored
        } else {
            g = std::sqrt(b.ux[0] * b.ux[0] + b.uy[0] * b.uy[0] + b.uz[0] * b.uz[0] + 1.0);
            mc2 = b.restMass[0];
        }
        Text s;
        s.fixed(b.x[0], 12).s("  ").fixed(b.y[0], 12).s("  ").fixed(b.z[0], 12).s("  ").fixed(b.ux[0], 12).s("  ")
            .fixed(b.uy[0], 12).s("  ").fixed(b.uz[0], 12).s("  ").fixed((g - 1.0) * mc2, 12).s(" ").fixed(t, 12).nl();
        write_file(paths().output + "SingleParticle.txt", s.str(), true);
    }

    void full_particle_out() {
        dl(kDevCoord | kDevLost);
        const Beam& b = b_;
        Text s;
        for (int i = 0; i < b.n; ++i) {
            s.i(b.lost[i]).s(" ").g(b.charge[i]).s(" ").g(b.x[i]).s(" ").g(b.ux[i]).s(" ").g(b.y[i]).s(" ").g(b.uy[i])
                .s(" ").g(b.z[i]).s(" ").g(b.uz[i]).nl();
        }
        write_file(paths().output + "FullParticle.txt", s.str(), true);
    }

    void dst_out(double z, const std::vector<Rec>& rec) {
        const Beam& b = b_;
        const int s = b.n;
        const double t0 = rec[s][6];
        const double mc2 = b.restMass[s];
        char zname[64];
        std::snprintf(zname, sizeof zname, "%f", z);
        if (b.fromDstFile) {
            // .edst: every selected particle (synchronous one included) with charge, mass, weight
            std::vector<double> rows;
            for (int i = 0; i <= s; ++i) {
                const Rec& r = rec[i];
                if (r[7] != 1.0) continue;
                const double u2 = compat_ ? r[5] * r[5] : r[1] * r[1] + r[3] * r[3] + r[5] * r[5];
                const double w = b.weight[i];
                const double v[9] = {r[0] * 100.0, r[1] / r[5], r[2] * 100.0, r[3] / r[5],
                                     (r[6] - t0) * k::two_pi * b.frequency, (std::sqrt(u2 + 1.0) - 1.0) * b.restMass[i],
                                     b.charge[i] / w / k::e_charge, b.mass[i] / w / k::m_proton * k::proton_mc2, w};
                rows.insert(rows.end(), v, v + 9);
            }
            const std::int32_t cnt = static_cast<std::int32_t>(rows.size() / 9) - 1;
            const double cur = b.current * 1000.0;
            const double fm = b.frequency * 1e-06;
            std::string out;
            out.push_back(static_cast<char>(0x7D));
            out.push_back(static_cast<char>(0x64));
            out.append(reinterpret_cast<const char*>(&cnt), 4);
            out.append(reinterpret_cast<const char*>(&cur), 8);
            out.append(reinterpret_cast<const char*>(&fm), 8);
            out.push_back(static_cast<char>(0x7D));
            out.append(reinterpret_cast<const char*>(rows.data()), rows.size() * 8);
            out.append(reinterpret_cast<const char*>(&mc2), 8);
            if (!write_file(paths().output + "outData_" + zname + ".edst", out, false, true)) say("output error");
            return;
        }
        std::vector<double> rows;
        double sumW = 0.0;
        for (int i = 0; i < s; ++i) {
            const Rec& r = rec[i];
            if (r[7] != 1.0) continue;
            const double u2 = compat_ ? r[5] * r[5] : r[1] * r[1] + r[3] * r[3] + r[5] * r[5];
            rows.push_back(r[0] * 100.0);
            rows.push_back(r[1] / r[5]);
            rows.push_back(r[2] * 100.0);
            rows.push_back(r[3] / r[5]);
            rows.push_back((r[6] - t0) * k::two_pi * b.frequency);
            rows.push_back((std::sqrt(u2 + 1.0) - 1.0) * mc2);
            sumW = sumW + b.weight[i];
        }
        double cur = 0.0;
        if (p_.spaceCharge == 1) cur = std::fabs(b.charge[s] * b.realPerMacro * b.frequency * 1000.0 * sumW);
        const std::int32_t cnt = static_cast<std::int32_t>(rows.size() / 6);
        std::string out;
        out.push_back(static_cast<char>(0x7D));
        out.push_back(static_cast<char>(0x64));
        out.append(reinterpret_cast<const char*>(&cnt), 4);
        out.append(reinterpret_cast<const char*>(&cur), 8);
        const double fm = b.frequency * 1e-06;
        out.append(reinterpret_cast<const char*>(&fm), 8);
        out.push_back(static_cast<char>(0x7D));
        out.append(reinterpret_cast<const char*>(rows.data()), rows.size() * 8);
        out.append(reinterpret_cast<const char*>(&mc2), 8);
        if (!write_file(paths().output + "outData_" + zname + ".dst", out, false, true)) say("output error");
        if (b.useWeightFile) {
            std::string wb;
            wb.append(reinterpret_cast<const char*>(&cnt), 4);
            for (int i = 0; i < s; ++i)
                if (rec[i][7] == 1.0) wb.append(reinterpret_cast<const char*>(&b.weight[i]), 8);
            if (!write_file(paths().output + "outData_" + zname + ".dstb", wb, false, true)) say("output error");
        }
    }

    // Corrected engine: estimated size of BeamSet.plt (header 34 B, a record 25 + 56 n B every
    // dumpperiodicity steps; steps = lattice length / (beta c dt) + bunch duration / dt, with the
    // entrance beta of the synchronous particle -- too many steps for accelerating lines).  Only a
    // warning (libPIC.so refuses to run above 50 GB, spec 65 1.2).
    void beamset_estimate() {
        const int n = b_.n;
        const double u = b_.uz[n], beta = u / std::sqrt(1.0 + u * u);
        if (!(beta > 0.0) || !(dt_ > 0.0)) return;
        const double steps = std::ceil(lat_.total_length() / (beta * k::c * dt_) + b_.bunchTime / dt_);
        const double records = std::floor(steps / p_.dumpPeriodicity) + 1.0;
        const double bytes = 34.0 + records * (25.0 + 56.0 * static_cast<double>(n));
        const double gb = bytes / 1073741824.0;
        if (gb > options().beamSetWarnGB)
            say(Text().s("Warning: BeamSet.plt is estimated at ").g(gb).s(" GB (dumpperiodicity ").i(p_.dumpPeriodicity)
                    .s(", about ").g(steps).s(" steps, ").i(n).s(" particles); check the free disk space").str());
    }

    void beam_out(int mode, double t, double zs, int step) {
        const Beam& b = b_;
        std::string out;
        out.push_back(static_cast<char>(0x7D));
        const std::int32_t m32 = mode, s32 = step;
        out.append(reinterpret_cast<const char*>(&m32), 4);
        out.append(reinterpret_cast<const char*>(&s32), 4);
        out.append(reinterpret_cast<const char*>(&t), 8);
        out.append(reinterpret_cast<const char*>(&zs), 8);
        for (int i = 0; i < b.n; ++i) {
            if (mode == 0) {
                for (int j = 0; j < 6; ++j) out.append(reinterpret_cast<const char*>(&b.rel[j][i]), 8);
                const std::int32_t alive = b.lost[i] == 1 ? 0 : 1, id = i;
                out.append(reinterpret_cast<const char*>(&alive), 4);
                out.append(reinterpret_cast<const char*>(&id), 4);
            } else {
                const Rec& r = planeRec_[i];
                for (int j : {0, 1, 2, 3, 6, 5}) out.append(reinterpret_cast<const char*>(&r[j]), 8);
                const std::int32_t fl = static_cast<std::int32_t>(std::lround(r[7])), id = i;
                out.append(reinterpret_cast<const char*>(&fl), 4);
                out.append(reinterpret_cast<const char*>(&id), 4);
            }
        }
        if (!write_file(paths().output + "BeamSet.plt", out, true, true)) say("BeamSet.plt cannot be output");
    }

    double progress_pct() const {
        const Beam& b = b_;
        const double zs = b.z[b.n];
        return (zs - zStartRun_) / ((p_.outputPlanes.back() - zStartRun_) + zs - b.zMin) * 100.0;
    }

    // A progress line is due every 3 s (AVAS.dll; kept with compat=dll).  The default mode writes one
    // every second or every 5 % of the run, whichever comes first, so that short GPU runs show their
    // progress (at most 20 lines more than one a second).
    bool progress_due(std::chrono::steady_clock::time_point now, std::chrono::steady_clock::time_point last) const {
        const double since = std::chrono::duration<double>(now - last).count();
        if (compat_) return since > 3.0;
        return since >= 1.0 || progress_pct() >= lastProgressPct_ + 5.0;
    }

    void progress(const std::vector<double>& events) {
        const Beam& b = b_;
        const double zs = b.z[b.n];
        const double pct = progress_pct();
        lastProgressPct_ = pct;
        const double T = std::chrono::duration<double>(std::chrono::steady_clock::now() - startClock_).count();
        const double R = (T * 100.0 / pct - T) * 0.5;
        char tb[64], rb[64];
        if (T <= 60) std::snprintf(tb, sizeof tb, "%.0fs", T);
        else if (T <= 3600) std::snprintf(tb, sizeof tb, "%.2fmin", T / 60);
        else std::snprintf(tb, sizeof tb, "%.2fh", T / 3600);
        if (R > 3600) std::snprintf(rb, sizeof rb, "%.2fh", R / 3600);
        else if (R > 60) std::snprintf(rb, sizeof rb, "%.2fmin", R / 60);
        else std::snprintf(rb, sizeof rb, "%.0fs", R);
        char line[256];
        std::snprintf(line, sizeof line, "Simulate progress %.2f%%. Estimated remaining time %s. Run time %s.  %d  %.2fm%30s\r",
                      pct, rb, tb, b.nAlive, zs, "");
        say_raw(line);
        (void)events;
    }

    // ---- the time-step loop of one segment (tcodeRun) ----
    void tcode(double& t, double& zSeg, const std::vector<double>& events, const std::vector<int>& marks) {
        // the last element of this segment ends here (the lattice end for the last segment)
        segEnd_ = events.back() < lat_.total_length() ? events.back() : 1e300;
        Beam& b = b_;
        const int n = b.n;
        if (events[0] <= b.zMax) {
            char a[64], c[64];
            std::snprintf(a, sizeof a, "%f", b.zMax);
            std::snprintf(c, sizeof c, "%f", events[0]);
            throw EngineError(std::string("Tcode won't start, there are particles located behind the output plane. "
                                          "headParticleCoordinates: ") + a + " line: " + c);
        }
        // particle blocks (the original's thread chunks); the partition fixes the summation order
        std::vector<int> bounds{0};
        const int nCh = p_.cores[0];
        if (nCh >= 1) {
            const int chunk = static_cast<int>(std::ceil(static_cast<double>(n) / static_cast<double>(nCh)));
            for (int i = 0; i < nCh; ++i) {
                const int next = bounds.back() + chunk;
                if (n <= next) {
                    bounds.push_back(n);
                    break;
                }
                bounds.push_back(next);
            }
        }
        p_.cores[0] = static_cast<int>(bounds.size()) - 1;
        // AVAS.dll: blocks of the cores table summed in block order; corrected engine: fixed groups
        // of 4096 particles (reduce.h), independent of the machine and of the device
        // (no block at all -- multithreading 0 / 2 / 3 -- pushes nothing in both modes, as before)
        const int nBlocks = compat_ || p_.cores[0] < 1 ? p_.cores[0] : red_groups(n);
        std::vector<BlockAcc> acc(static_cast<size_t>(std::max(nBlocks, 1)));
        BlockAcc syncAcc;
        syncAcc.reset();
        oldCoord_.assign(static_cast<size_t>(n) + 1, std::array<double, 6>{});
        int threads = options().threads > 0 ? options().threads : static_cast<int>(std::thread::hardware_concurrency());
        threads = std::max(1, std::min(threads, nBlocks));
        WorkerPool pool(threads > 1 ? threads : 0);

        update_beam_parameters();
        syn_out();
        double prev[18];
        for (int j = 0; j < 6; ++j) prev[j] = b.centroid[j];
        const double s0[6] = {b.x[n], b.ux[n], b.y[n], b.uy[n], b.z[n], b.uz[n]};
        for (int j = 0; j < 6; ++j) prev[6 + j] = s0[j];
        prev[12] = b.gamma;
        prev[13] = b.totalWeight;
        prev[14] = prev[15] = prev[16] = prev[17] = 0.0;
        double sync[6] = {0, 0, 0, 0, 0, 0};
        t = dt_ + t;
        prepare(t);
        capture_sync_tot(zSeg);
        trans(n, n + 1, zSeg, events, t, 0.0, sync, syncAcc);
        for (int j = 0; j < 6; ++j) sync[j] = 0.0;
        sync[0] = b.x[n]; sync[1] = b.ux[n]; sync[2] = b.y[n]; sync[3] = b.uy[n]; sync[4] = b.z[n]; sync[5] = b.uz[n];
        double gp = b.gamma;
        double bp = std::sqrt(1.0 - 1.0 / (gp * gp));
        for (auto& a : acc) a.reset();
        size_t iEv = 0;
        int stall = 0, lastCross = 0;
        int nAlivePrev = b.nAlive;
        auto lastProgress = std::chrono::steady_clock::now();

        while (true) {
            if (stop_requested()) throw EngineError("Programs terminated by users.                       ");
            b.time = t;
            const auto tp0 = std::chrono::steady_clock::now();
            if (sc_) {
                b.time = t;
                sc_solve(oldCoord_[n].data());
            }
            prepare(t);
            if (dev_ && nBlocks < 1) {
                // no particle block (multithreading 0 / 2 / 3): nothing is pushed, as on the CPU
            } else if (dev_) {
                std::vector<StepGroup> grp;
                long long spc = 0;
                dev_->step(lat_scalars(), step_args(zSeg, events, t, gp, sync), rfTab_, events, grp, spc, devRecCount_);
                for (size_t g = 0; g < acc.size(); ++g) {
                    BlockAcc& a = acc[g];
                    a.reset();
                    if (g >= grp.size()) continue;
                    set_block_sums(a, grp[g].s);
                    for (int j = 0; j < 6; ++j) a.maxAbs[j] = grp[g].mx[j];
                    a.zmin = grp[g].zmin;
                    a.zmax = grp[g].zmax;
                }
                acc[0].spCnt = static_cast<int>(spc);
            } else if (compat_) {
                pool.run(nBlocks, [&](int k) {
                    trans(bounds[k], bounds[k + 1], zSeg, events, t, gp, sync, acc[k]);
                });
            } else {
                pool.run(nBlocks, [&](int g) { trans_group(g, zSeg, events, t, gp, sync, acc[static_cast<size_t>(g)]); });
            }
            pushTime_ += std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - tp0).count();
            if (n == 1) {
                dl(kDevCoord);
                single_out(t);
            }
            if (lat_.hasBoundary && p_.boundary == 1) boundary_check(t);

            // block reduction, block order
            b.zMin = 1e10;
            b.zMax = -1e10;
            b.nAlive = 0;
            b.totalWeight = 0.0;
            double sx[3] = {0, 0, 0}, s1[6] = {0, 0, 0, 0, 0, 0}, s2[6] = {0, 0, 0, 0, 0, 0}, mx[6] = {0, 0, 0, 0, 0, 0};
            double sg = 0.0, gsum = 0.0;
            for (int k = 0; k < nBlocks; ++k) {
                const BlockAcc& a = acc[k];
                for (int j = 0; j < 3; ++j) sx[j] = sx[j] + a.cross[j];
                b.nAlive = static_cast<int>(std::round(a.cnt0) + static_cast<double>(b.nAlive));
                b.totalWeight = a.cnt1 + b.totalWeight;
                lat_.spCrossCount += a.spCnt;
                sg = sg + a.sumGz2;
                gsum = gsum + a.gsum;
                for (int j = 0; j < 6; ++j) {
                    s1[j] = s1[j] + a.sum1[j];
                    s2[j] = s2[j] + a.sum2[j];
                    if (mx[j] < a.maxAbs[j]) mx[j] = a.maxAbs[j];
                }
                if (a.zmin < b.zMin) b.zMin = a.zmin;
                if (b.zMax < a.zmax) b.zMax = a.zmax;
            }
            if (b.nAlive == 0 || (s2[0] == 0.0 && s2[2] == 0.0 && s2[4] == 0.0 && n != 1)) {
                full_particle_out();  // AVAS.dll writes FullParticle.txt before it stops
                throw EngineError("End of simulation, all particles lost.                     ");
            }
            // last event passed by every live particle: final output, no DataSet row
            {
                const double zev = events[iEv];
                if (zev < b.zMin && zev <= lat_.sp_exit() && events.size() <= iEv + 1) {
                    if (marks[iEv] == -1) final_output(zev, t);
                    return;
                }
            }
            // statistics and the DataSet row (describes the state before this step)
            const double W = prev[13];
            const double qx = s2[0] / W, qxp = s2[1] / W, qy = s2[2] / W, qyp = s2[3] / W, qz = s2[4] / W,
                         qd = s2[5] / W;
            const double iw = 1.0 / W;
            const double mxx = sx[0] * iw, myy = sx[1] * iw, mzz = sx[2] * iw;
            const double ex = std::sqrt(qxp * qx - mxx * mxx);
            const double ey = std::sqrt(qyp * qy - myy * myy);
            const double ez = std::sqrt(qd * qz - mzz * mzz);
            const double iex = 1.0 / ex, iey = 1.0 / ey, iez = 1.0 / ez;
            const double ax = -(iex * mxx), ay = -(iey * myy), az = -(iez * mzz);
            const double btx = iex * qx, bty = iey * qy, btz = iez * qz;
            const double enx = bp * ex * gp, eny = bp * ey * gp, enz = bp * ez * gp * gp * gp;
            for (int j = 0; j < 6; ++j) b.centroid[j] = s1[j] / b.totalWeight;
            const double gnew = gsum / b.totalWeight;
            const double ig = 1.0 / gnew;
            const double bpNew = std::sqrt(1.0 - ig * ig);
            b.rms = {std::sqrt(qx), std::sqrt(qy), std::sqrt(sg / W)};
            b.gamma = gnew;
            b.betaGammaLambda = (k::c / b.frequency) * b.centroid[5];
            if (p_.lpBoundary == 1) lp_fold(ig);
            if (!p_.track.empty()) single_track();

            Text row;
            row.g((prev[12] - 1.0) * b.restMass[n]);
            for (int j = 0; j < 6; ++j) row.s(" ").g(prev[j]);
            for (double v : {ax, ay, az, btx, bty, btz, enx, eny, enz, std::sqrt(qx), std::sqrt(qxp), std::sqrt(qy),
                             std::sqrt(qyp), std::sqrt(qz), std::sqrt(qd)})
                row.s(" ").g(v);
            for (int j = 0; j < 6; ++j) row.s(" ").g(mx[j]);
            row.s(" ").i(nAlivePrev);
            for (int j = 6; j < 12; ++j) row.s(" ").g(prev[j]);
            for (int j = 14; j < 18; ++j) row.s(" ").g(prev[j]);
            row.s(" ").i(step_ - 1).s(" ").g(t).nl();
            // statoutputinterval N (libPIC.so, spec 65 5.1): only the rows whose step number is a
            // multiple of N (the first row always, no extra last row); 0: no rows
            const int every = p_.statOutputInterval;
            if (every == 1 || (every > 1 && (step_ - 1) % every == 0)) dataSet_.write(row.str());
            gp = gnew;
            bp = bpNew;

            for (auto& a : acc) a.reset();
            syn_out();
            for (int j = 0; j < 6; ++j) prev[j] = b.centroid[j];
            const double sc[6] = {b.x[n], b.ux[n], b.y[n], b.uy[n], b.z[n], b.uz[n]};
            for (int j = 0; j < 6; ++j) prev[6 + j] = sc[j];
            prev[12] = b.gamma;
            nAlivePrev = b.nAlive;
            prev[13] = b.totalWeight;
            // bend / displacement columns (rotMode is always 0 in M1)
            const auto& so = oldCoord_[n];
            const int rot = b.rotMode;
            if (rot == 0) {
                if (prev[14] == 2.0) {
                    if (b.z[n] < zSeg) { prev[14] = 2; prev[15] = prev[16] = prev[17] = 0; }
                    else { prev[14] = prev[15] = prev[16] = 0; prev[17] = b.z[n] - zSeg; }
                } else {
                    prev[14] = prev[15] = prev[16] = 0;
                    prev[17] = b.z[n] - so[4];
                }
            } else if (prev[14] == 2.0) {
                prev[14] = 2; prev[15] = prev[16] = prev[17] = 0;
            } else {
                const double dT = rot == 1 ? b.y[n] - so[2] : b.x[n] - so[0];
                const double dZ = b.z[n] - so[4];
                if (spRec_[n][7] == 1.0) {
                    const double f = 1.0 - (t - spRec_[n][6]) / dt_;
                    prev[14] = 2; prev[15] = rot; prev[16] = dT * f; prev[17] = dZ * f;
                } else {
                    prev[14] = 1; prev[15] = rot; prev[16] = dT; prev[17] = dZ;
                }
            }
            if (b.nAlive == 0) {
                full_particle_out();
                throw EngineError("End of simulation, all particles lost.                     ");
            }

            // every live particle crossed a superposeout exit: restart in the exit frame
            if (b.nAlive <= lat_.spCrossCount && lat_.spOutCur < lat_.spOutCount) {
                zSeg = lat_.spOut[lat_.spOutCur][6];
                lat_.spCrossCount = 0;
                dl(kDevSpRec | kDevCoord | kDevLost | kDevSpz | kDevIdx);
                if (zSeg == lat_.total_length()) {
                    dst_out(zSeg, spRec_);
                    return;
                }
                double tmin = 1e10;
                for (int i = 0; i < n; ++i)
                    if (spRec_[i][7] == 1.0 && spRec_[i][6] < tmin) tmin = spRec_[i][6];
                for (int i = 0; i <= n; ++i) {
                    const Rec& r = spRec_[i];
                    if (r[7] != 1.0) {
                        b.lost[i] = 1;
                        continue;
                    }
                    // same back-propagation as zTotcode (AVAS.dll: gamma from gamma*beta_z only)
                    ztot_row(r.data(), zSeg, tmin, !compat_, b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i], true);
                    b.spOutZ1[i] = lat_.spOut[lat_.spOutCur + 1][6];
                    b.spOutCount[i] += 1;
                    b.spOutZ0[i] = zSeg;
                }
                b.rotMode = 0;
                b.rotAngle = 0.0;
                t = tmin - dt_;
                lat_.spOutCur += 1;
                lat_.elements[b.elemIndex[n]].rotMode = 0;
                for (auto& r : spRec_) r.fill(0.0);
                ul(kDevCoord | kDevLost | kDevSpz | kDevIdx | kDevSpRec);
            }

            // events: output planes passed by every live particle
            {
                const double zev = events[iEv];
                if (zev < b.zMin && zev <= lat_.sp_exit()) {
                    if (iEv + 2 <= events.size() && events[iEv + 1] <= b.zMax) {
                        throw EngineError("The output plane interval is less than the bunch length. Increase the "
                                          "output plane interval and restart the program");
                    }
                    if (events.size() <= iEv + 1) {
                        if (marks[iEv] == -1) final_output(zev, t);
                        return;
                    }
                    iEv += 1;
                    const int m = marks[iEv - 1];
                    if (m == -4) {
                        // spacechargecomp: the record at this element entrance
                        for (const auto& row : lat_.spaceChargeComp)
                            if (std::fabs(row[0] - zev) < 1e-9 && std::fabs(row[1] - 1.0) < 1e-9)
                                b.scComp = compat_ ? row[2] : 1.0 - row[2];
                    }
                    // AVAS.dll restarts the time-step loop at a spacechargecomp event as at a
                    // steerer (particles moved back along straight lines); corrected: no restart
                    if (m > 0 || (m == -4 && compat_)) {
                        // steerer: kick at the middle of the following element, then restart the
                        // time-step loop from the crossing records (MethodMP::tcodeRun)
                        dl(kDevRec | kDevCoord | kDevIdx);
                        detach_device_rows_ = true;
                        if (m > 0) {
                            const double lq = lat_.elements[m + 1].length;
                            for (int i = 0; i < n; ++i) {
                                Rec& r = planeRec_[i];
                                if (r[7] != 1.0) continue;
                                r[4] = 0.0;
                                kick(r.data(), lat_.elements[m], lq, i);
                            }
                        }
                        double tmin = 1e10;
                        for (int i = 0; i < n; ++i)
                            if (planeRec_[i][7] == 1.0 && planeRec_[i][6] < tmin) tmin = planeRec_[i][6];
                        t = ztot(zev, tmin);
                        detach_device_rows_ = false;
                        ul(kDevRec | kDevCoord | kDevIdx);
                        zSeg = zev;
                        const std::vector<double> ev2(events.begin() + static_cast<long>(iEv), events.end());
                        const std::vector<int> mk2(marks.begin() + static_cast<long>(iEv), marks.end());
                        tcode(t, zSeg, ev2, mk2);
                        return;
                    }
                    if (m != -4) {  // -4: spacechargecomp change (no output)
                        dl(kDevRec);
                        dst_out(zev, planeRec_);
                        for (auto& r : planeRec_) r.fill(0.0);
                        if (dev_) {
                            dev_->zero_rec(false);
                            devRecCount_ = 0;
                        }
                    }
                }
            }
            const auto now = std::chrono::steady_clock::now();
            if (progress_due(now, lastProgress)) {
                lastProgress = now;
                progress(events);
            }
            t = dt_ + t;
            prepare(t);
            capture_sync_tot(zSeg);
            trans(n, n + 1, zSeg, events, t, b.gamma, sync, syncAcc);
            syncAcc.reset();
            rot_mode(sync);
            if (p_.dumpPeriodicity != 0 && step_ % p_.dumpPeriodicity == 0) {
                dl(kDevRel | kDevLost);
                beam_out(0, t - dt_, b.z[n], step_);
            }
            if (p_.pcHistogram[0] == 1.0) {
                dl(kDevCoord | kDevLost);
                ex_output();
            }
            step_ += 1;
            // particles that stop crossing the next plane are declared lost
            int nc = 0;
            if (dev_) nc = static_cast<int>(devRecCount_);
            for (int i = 0; i < (dev_ ? 0 : n); ++i)
                if (planeRec_[i][7] == 1.0) ++nc;
            if (nc == lastCross && nc != 0) stall += 1;
            else stall = 0;
            lastCross = nc;
            if (static_cast<double>(stall) > 0.8 * static_cast<double>(step_)) {
                dl(kDevRec | kDevCoord | kDevLost);
                Text lt;
                for (int i = 0; i < n; ++i) {
                    if (planeRec_[i][7] == 0.0) {
                        b.lost[i] = 1;
                        lt.i(i).fixed(b.x[i], 12).s("  ").fixed(b.ux[i], 12).s("  ").fixed(b.y[i], 12).s("  ")
                            .fixed(b.uy[i], 12).s("  ").fixed(b.z[i], 12).s("  ").fixed(b.uz[i], 12).nl();
                    }
                }
                write_file(paths().output + "LongitudinalBeamLoss.txt", lt.str(), true);
                ul(kDevLost);
            }
            if (p_.longLimits[0] != 0.0) longitudinal_loss();
        }
    }

    // lpboundary 1 (tcodeRun 4.2.7): the beam gamma is that of the synchronous particle and the
    // particles are folded into one RF period around it.  AVAS.dll folds with lambda * gbz_s / mean
    // gamma; the corrected engine with the period beta_s * lambda of the synchronous particle.
    void lp_fold(double igMean) {
        Beam& b = b_;
        const int n = b.n;
        const double gs = std::sqrt(b.uz[n] * b.uz[n] + b.uy[n] * b.uy[n] + b.ux[n] * b.ux[n] + 1.0);
        b.gamma = gs;
        const double lam = k::c / b.frequency;
        b.betaGammaLambda = lam * b.uz[n];
        const double full = compat_ ? lam * b.uz[n] * igMean : lam * (b.uz[n] / gs);
        const double half = full * 0.5;
        if (dev_) {
            dev_->lp_fold(b.z[n], full, half);
            return;
        }
        const kern::PartV P = pview();
        for (int i = 0; i < n; ++i) kern::lp_fold_particle(P, i, b.z[n], full, half);
    }

    // track: one line per listed particle (SingleTrack -> ParticleTracking.txt)
    void single_track() {
        dl(kDevCoord | kDevLost | kDevSc | kDevField);
        const Beam& b = b_;
        Text s;
        for (int id : p_.track) {
            if (id < 0 || id > b.n) {
                if (compat_) continue;  // AVAS.dll reads out of range
                throw EngineError("input.txt track: particle number out of range");
            }
            s.i(step_).s(" ").i(id).s(" ").i(b.lost[id]).s(" ").g(b.charge[id]).s(" ").g(b.restMass[id]);
            for (double v : {b.x[id], b.ux[id], b.y[id], b.uy[id], b.z[id], b.uz[id], b.scx[id], b.scy[id], b.scz[id],
                             b.ex[id], b.ey[id], b.ez[id], b.bx[id], b.by[id], b.bz[id]})
                s.s(" ").g(v);
            s.nl();
        }
        write_file(paths().output + "ParticleTracking.txt", s.str(), true);
    }

    // pchistogram 1: histograms of x, y, r, z of the live particles (exOutput -> PCHistogram.dat)
    void ex_output() {
        const Beam& b = b_;
        const int n = b.n;
        double xmin = 1e9, xmax = -1e9, ymin = 1e9, ymax = -1e9, zmin = 1e9, zmax = -1e9, rmax = -1e9;
        const double rmin = 0.0;
        std::vector<double> r(static_cast<size_t>(n), 0.0);
        auto upd = [&](double v, double& lo, double& hi) {
            // AVAS.dll: else-if (a value that lowers the minimum is not compared with the maximum)
            if (compat_) {
                if (lo <= v) { if (hi < v) hi = v; }
                else lo = v;
            } else {
                if (v < lo) lo = v;
                if (hi < v) hi = v;
            }
        };
        for (int i = 0; i < n; ++i) {
            if (b.lost[i] == 1) continue;
            upd(b.x[i], xmin, xmax);
            upd(b.y[i], ymin, ymax);
            upd(b.z[i], zmin, zmax);
            r[i] = std::sqrt(b.y[i] * b.y[i] + b.x[i] * b.x[i]);
            if (rmax < r[i]) rmax = r[i];
        }
        const int nb = static_cast<int>(std::round(p_.pcHistogram[1]));
        const double dn = static_cast<double>(nb);
        std::vector<std::int32_t> hx(static_cast<size_t>(std::max(nb, 0)), 0), hy = hx, hr = hx, hz = hx;
        double sx = 0, sy = 0, sr = 0, sz = 0;
        int cnt = 0;
        auto bin = [&](double v, double lo, double span) {
            int k = static_cast<int>(std::floor(((v - lo) * dn) / span));
            if (k < 0) k = 0;
            if (nb - 1 < k) k = nb - 1;
            return k;
        };
        for (int i = 0; i < n; ++i) {
            if (b.lost[i] == 1) continue;
            if (nb > 0) {
                hx[bin(b.x[i], xmin, xmax - xmin)] += 1;
                hy[bin(b.y[i], ymin, ymax - ymin)] += 1;
                hr[bin(r[i], rmin, rmax - rmin)] += 1;
                hz[bin(b.z[i], zmin, zmax - zmin)] += 1;
            }
            cnt += 1;
            sx = sx + b.x[i];
            sy = sy + b.y[i];
            sr = sr + r[i];
            sz = sz + b.z[i];
        }
        const double ic = 1.0 / static_cast<double>(cnt);
        const double mxv = sx * ic, myv = sy * ic, mrv = sr * ic, mzv = sz * ic;
        std::string out;
        out.push_back(static_cast<char>(0x7D));
        const std::int32_t st = step_, n32 = nb;
        out.append(reinterpret_cast<const char*>(&st), 4);
        out.append(reinterpret_cast<const char*>(&n32), 4);
        for (const auto* h : {&hx, &hy, &hr, &hz})
            for (int i = 0; i < nb; ++i) out.append(reinterpret_cast<const char*>(&(*h)[static_cast<size_t>(i)]), 4);
        for (double v : {xmin, xmax, mxv, ymin, ymax, myv, rmin, rmax, mrv, zmin, zmax, mzv})
            out.append(reinterpret_cast<const char*>(&v), 8);
        if (!write_file(paths().output + "PCHistogram.dat", out, true, true)) say("PCHistogram.dat cannot be output");
    }

    // longlimits: particles outside the energy or phase window are lost (LongitudinalBunchLoss)
    void longitudinal_loss() {
        Beam& b = b_;
        const int n = b.n;
        const double gs = compat_ ? std::sqrt(b.uz[n] * b.uz[n] + 1.0)
                                  : std::sqrt(b.ux[n] * b.ux[n] + b.uy[n] * b.uy[n] + b.uz[n] * b.uz[n] + 1.0);
        const double bs = std::sqrt(1.0 - (1.0 / gs) / gs);
        const double ws = (gs - 1.0) * b.restMass[n];
        const double dzMax = (p_.longLimits[1] / 360.0) / b.frequency * bs * k::c;
        if (dev_) {
            dev_->long_loss(b.z[n], ws, p_.longLimits[2], dzMax);
            return;
        }
        const kern::PartV P = pview();
        for (int i = 0; i < n; ++i)
            kern::long_loss_particle(P, i, b.restMass.data(), b.z[n], ws, p_.longLimits[2], dzMax, compat_);
    }

    // ---- boundary.txt (tcodeRun 4.2.4, Geometry::Boundary) ----
    void boundary_check(double t) {
        Beam& b = b_;
        const Boundary& bd = lat_.bnd;
        if (dev_) {
            std::vector<BoundaryHit> hits;
            dev_->boundary(lat_scalars(), t, dt_, hits);
            std::sort(hits.begin(), hits.end(), [](const BoundaryHit& x, const BoundaryHit& y) { return x.i < y.i; });
            for (const BoundaryHit& h : hits) {
                if (h.code < 0) (void)bd.geoms[static_cast<size_t>(h.geom)].par.at(static_cast<size_t>(-h.code - 10));
                if (h.code == 1) collision_line(bd.geoms[static_cast<size_t>(h.geom)].shape, h.P, h.tHit, h.i);
            }
            return;
        }
        latV_ = latH_.view(lat_);
        const kern::PartV P = pview();
        for (int i = 0; i < b.n; ++i) {
            int gi = 0;
            double Pp[6], tHit = t;
            const int code = compat_ ? kern::boundary_particle<DllMath>(latV_, P, i, t, dt_, true, gi, Pp, tHit)
                                     : kern::boundary_particle<PortableMath>(latV_, P, i, t, dt_, false, gi, Pp, tHit);
            if (code < 0) (void)bd.geoms[static_cast<size_t>(gi)].par.at(static_cast<size_t>(-code - 10));
            if (code != 1) continue;
            collision_line(bd.geoms[static_cast<size_t>(gi)].shape, Pp, tHit, i);
            b.lost[i] = 1;
        }
    }

    // Geometry::Boundary of particle i with geometry gi (collisionData.txt line on a hit).
    void geometry_hit(int gi, double t, int i) {
        Beam& b = b_;
        const kern::PartV P = pview();
        const kern::GeomV& gv = latH_.geoms[static_cast<size_t>(gi)];
        double Pp[6], tHit = t;
        const int code = compat_ ? kern::geometry_hit<DllMath>(gv, P, i, t, dt_, Pp, tHit)
                                 : kern::geometry_hit<PortableMath>(gv, P, i, t, dt_, Pp, tHit);
        if (code < 0) (void)lat_.bnd.geoms[static_cast<size_t>(gi)].par.at(static_cast<size_t>(-code - 10));
        if (code != 1) return;
        collision_line(gv.shape, Pp, tHit, i);
        b.lost[i] = 1;
    }

    void collision_line(int shape, const double* P, double tHit, int i) {
        const Beam& b = b_;
        const double gm = shape == 1 ? std::sqrt(P[5] * P[5] + P[3] * P[3] + P[1] * P[1] + 1.0)
                                     : std::sqrt(P[3] * P[3] + P[5] * P[5] + P[1] * P[1] + 1.0);
        const double ig = 1.0 / gm;
        Text s;
        s.g(P[0]).s(" ").g(P[2]).s("  ").g(P[4]).s("  ").g(P[1] * k::c * ig).s("  ").g(P[3] * k::c * ig).s("  ")
            .g(P[5] * k::c * ig).s("  ").g(b.charge[i] * 6.241509479607718e+18).s("  ").g(b.restMass[i]).s("  ")
            .g(b.weight[i]).s("  ").g(tHit).s("  ").s(b.kind.empty() ? p_.kind : b.kind[static_cast<size_t>(i)]).nl();
        write_file(paths().output + "collisionData.txt", s.str(), true);
    }

    void final_output(double zev, double t) {
        full_particle_out();
        dl(kDevRec | kDevRel);
        dst_out(zev, planeRec_);
        beam_out(0, t, b_.z[b_.n], step_);
    }
};

}  // namespace

void run_tracking(Lattice& lat, Beam& beam, Params& p) {
    Tracker(lat, beam, p).run();
}

}  // namespace avas
