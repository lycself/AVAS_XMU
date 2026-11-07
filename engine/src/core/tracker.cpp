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

#include "core/dllmath.h"
#include "core/engine.h"
#include "core/errors.h"
#include "core/errorset.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/physics.h"
#include "core/spacecharge.h"

namespace avas {

namespace {

// Minimal fork-join pool: run(n, fn) calls fn(0..n-1) on the workers and waits.
class WorkerPool {
public:
    explicit WorkerPool(int threads) {
        for (int i = 0; i < threads; ++i) workers_.emplace_back([this] { loop(); });
    }
    ~WorkerPool() {
        {
            std::lock_guard<std::mutex> g(m_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& t : workers_) t.join();
    }
    void run(int n, const std::function<void(int)>& fn) {
        if (workers_.empty() || n <= 1) {
            for (int i = 0; i < n; ++i) fn(i);
            return;
        }
        std::unique_lock<std::mutex> g(m_);
        fn_ = &fn;
        next_ = 0;
        total_ = n;
        pending_ = n;
        ++generation_;
        cv_.notify_all();
        done_.wait(g, [this] { return pending_ == 0; });
        fn_ = nullptr;
    }

private:
    void loop() {
        unsigned seen = 0;
        std::unique_lock<std::mutex> g(m_);
        while (true) {
            cv_.wait(g, [&] { return stop_ || (generation_ != seen && next_ < total_); });
            if (stop_) return;
            while (next_ < total_) {
                const int k = next_++;
                const std::function<void(int)>* fn = fn_;
                g.unlock();
                (*fn)(k);
                g.lock();
                if (--pending_ == 0) done_.notify_all();
            }
            seen = generation_;
        }
    }
    std::vector<std::thread> workers_;
    std::mutex m_;
    std::condition_variable cv_, done_;
    const std::function<void(int)>* fn_ = nullptr;
    int next_ = 0, total_ = 0, pending_ = 0;
    unsigned generation_ = 0;
    bool stop_ = false;
};

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
        }

        const std::vector<Segment> segs = segment_lattice();
        startClock_ = std::chrono::steady_clock::now();
        zStartRun_ = b_.z[n];
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
        maxSec_ = 0;
        auto track = [&](int i, bool isSync) {
            Rec S{};
            for (const Segment& sg : segs) {
                if (sg.zcode) se_zrun(sg, S, i, isSync);
                else se_trun(sg, S, i, isSync);
            }
        };
        track(n, true);
        say("Synchronized particles have completed simulation.");
        maxSec_ *= 10;
        for (int i = 0; i < n; ++i) {
            if (stop_requested()) throw EngineError("Programs terminated by users.                       ");
            track(i, false);
            char line[160];
            std::snprintf(line, sizeof line, "Simulating the %d particle.  total number of particles %d.                          \r", i, n);
            say_raw(line);
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

    void se_plane_out(int i, const Rec& S, double z, bool isSync) {
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

    // SeTRun: own-time steps of one particle through a time-step segment
    void se_trun(const Segment& sg, Rec& S, int i, bool isSync) {
        Beam& b = b_;
        if (b.lost[i] != 0) return;
        const std::vector<double>& P = sg.ev;
        const double L = lat_.total_length();
        const bool errOn = lat_.sample && (lat_.sample->flags[2] + lat_.sample->flags[3] != 0);
        int iter = 0;
        // AVAS.dll compares z with a register that holds 0 only in the first step and afterwards the
        // particle's own new z (the comparisons with 0 then always pass); the corrected engine uses 0
        double thr = 0.0;
        while (true) {
            ++iter;
            if (!isSync) {
                if (iter > maxSec_) {
                    b.lost[i] = 1;
                    return;
                }
            } else {
                se_syn_out(i);
            }
            double z = b.z[i];
            int& ei = b.elemIndex[i];
            const bool inSp = lat_.elements[static_cast<size_t>(ei)].inSuperposeOut != 0;
            double E[3] = {0, 0, 0}, B[3] = {0, 0, 0};
            if ((thr <= z && b.lost[i] == 0 && z <= L) || (inSp && b.lost[i] == 0)) {
                if (z >= thr) {
                    const int last = static_cast<int>(lat_.elements.size()) - 1;
                    if (z < lat_.elemZStart[static_cast<size_t>(ei)]) {
                        do { --ei; } while (ei > 0 && z < lat_.elemZStart[static_cast<size_t>(ei)]);
                    } else if (lat_.elemZEnd[static_cast<size_t>(ei)] <= z) {
                        do {
                            // AVAS.dll stops at a superposeout block when there are no element errors
                            if (!errOn && lat_.elements[static_cast<size_t>(ei)].inSuperposeOut != 0) break;
                            ++ei;
                        } while (ei < last && lat_.elemZEnd[static_cast<size_t>(ei)] <= z);
                    }
                    if (ei < 0) ei = 0;
                    if (ei > last) ei = last;
                    const Element& e = lat_.elements[static_cast<size_t>(ei)];
                    if (!errUse_.empty() && errUse_[static_cast<size_t>(ei)]) element_err_field(i, b.t[i], e);
                    else element_field(i, b.t[i], e);
                    E[0] = b.ex[i]; E[1] = b.ey[i]; E[2] = b.ez[i];
                    B[0] = b.bx[i]; B[1] = b.by[i]; B[2] = b.bz[i];
                }
            } else {
                if (L < z) b.lost[i] = 2;
                if (z < thr) b.lost[i] = 1;
            }
            const double old[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
            if (b.lost[i] != 0) {
                if (isSync && maxSec_ < iter) maxSec_ = iter;
                return;
            }
            const double qm = b.qOverM[i], dt = b.dt[i];
            double ux = b.ux[i], uy = b.uy[i], uz = b.uz[i], f;
            if (qm == 0.0) {
                const double g = std::sqrt((uy * uy + uz * uz) + (ux * ux + 1.0));
                f = (k::c / g) * dt;
            } else {
                const double hq = qm * k::inv_c;
                const double h = hq * dt * 0.5;
                ux = h * E[0] + ux;
                uy = h * E[1] + uy;
                uz = h * E[2] + uz;
                const double g = std::sqrt((uz * uz + uy * uy) + (ux * ux + 1.0));
                const double tau = (dt * qm) / g;
                const double a = tau * B[0], bb = tau * B[1], c = tau * B[2];
                const double bb2 = bb * bb, a2 = a * a, c2 = c * c;
                const double D = 1.0 / ((a2 + bb2 + c2) * 0.25 + 1.0);
                const double ab = bb * a * 0.5, ac = c * a * 0.5, bc = c * bb * 0.5;
                const double h2 = hq * dt * 0.5;
                const double nx = ((((a2 - bb2) - c2) * 0.25 + 1.0) * ux + (ab + c) * uy + (ac - bb) * uz) * D + E[0] * h2;
                const double ny = ((((bb2 - c2) - a2) * 0.25 + 1.0) * uy + (ab - c) * ux + (bc + a) * uz) * D + E[1] * h2;
                const double nz = E[2] * h2 + ((bc - a) * uy + (ac + bb) * ux + (((c2 - bb2) - a2) * 0.25 + 1.0) * uz) * D;
                ux = nx; uy = ny; uz = nz;
                const double g2 = std::sqrt((uz * uz + uy * uy) + (ux * ux + 1.0));
                f = (k::c / g2) * dt;
            }
            b.ux[i] = ux; b.uy[i] = uy; b.uz[i] = uz;
            b.z[i] = f * uz + b.z[i];
            b.x[i] = f * ux + b.x[i];
            b.y[i] = uy * f + b.y[i];
            b.t[i] = dt + b.t[i];
            z = b.z[i];
            if (compat_) thr = z;
            // output planes (the original outputs only at the last one of the segment)
            const size_t kp = static_cast<size_t>(b.nextPlane[i]);
            if (kp + 1 <= P.size() && P[kp] <= z && lat_.elements[static_cast<size_t>(ei)].inSuperposeOut == 0) {
                const double r = (P[kp] - old[4]) / (z - old[4]);
                const double cur[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
                for (int j = 0; j < 6; ++j) S[j] = (cur[j] - old[j]) * r + old[j];
                S[6] = (r - 1.0) * dt + b.t[i];
                S[7] = 1.0;
                b.nextPlane[i] += 1;
                if (P.size() <= kp + 1) {
                    if (sg.marks[kp] == -1) se_plane_out(i, S, P[kp], isSync);
                    if (isSync && maxSec_ < iter) maxSec_ = iter;
                    return;
                }
                if (!compat_ && sg.marks[kp] == -1) se_plane_out(i, S, P[kp], isSync);
            }
            // superposeout exit
            const int si = b.spOutCount[i];
            if (si < lat_.spOutCount && lat_.elements[static_cast<size_t>(ei)].inSuperposeOut != 0) {
                const auto& row = lat_.spOut[static_cast<size_t>(si)];
                const double zs = lat_.elements[static_cast<size_t>(ei)].zStart;
                const double cur[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
                double lc[6], lp[6];
                sp_local(row, zs, cur, lc);
                sp_local(row, zs, old, lp);
                if (0.0 < lc[4]) {
                    const double r = lp[4] / (lp[4] - lc[4]);
                    b.x[i] = (lc[0] - lp[0]) * r + lp[0];
                    b.y[i] = (lc[2] - lp[2]) * r + lp[2];
                    b.z[i] = row[6];
                    b.ux[i] = (lc[1] - lp[1]) * r + lp[1];
                    b.uy[i] = (lc[3] - lp[3]) * r + lp[3];
                    b.uz[i] = (lc[5] - lp[5]) * r + lp[5];
                    b.t[i] = (r - 1.0) * dt + b.t[i];
                    if (row[6] == L) {
                        S = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i], b.t[i], 1.0};
                        if (isSync && maxSec_ < iter) maxSec_ = iter;
                        return;
                    }
                    b.spOutZ1[i] = lat_.spOut[static_cast<size_t>(si) + 1][6];
                    b.spOutZ0[i] = row[6];
                    ei += 1;
                    b.spOutCount[i] = si + 1;
                }
            }
            if (p_.boundary == 1 && lat_.hasBoundary) {
                const double zz = b.z[i];
                if (zz < thr || P.back() < zz || b.lost[i] != 0) {
                    if (L < zz && lat_.spOutCur == static_cast<int>(lat_.spOut.size()) && b.lost[i] == 0) b.lost[i] = 2;
                } else {
                    const Boundary& bd = lat_.bnd;
                    int& gi = b.geomIndex[i];
                    if (zz < bd.segStart[static_cast<size_t>(gi)]) {
                        do { --gi; } while (gi > 0 && zz < bd.segStart[static_cast<size_t>(gi)]);
                    } else {
                        while (gi + 1 < static_cast<int>(bd.segEnd.size()) && bd.segEnd[static_cast<size_t>(gi)] <= zz) ++gi;
                    }
                    oldCoord_.resize(static_cast<size_t>(b.n) + 1);
                    oldCoord_[static_cast<size_t>(i)] = {old[0], old[1], old[2], old[3], old[4], old[5]};
                    const double dtSave = dt_;
                    dt_ = dt;
                    geometry_hit(bd.geoms[static_cast<size_t>(gi)], b.t[i], i);
                    dt_ = dtSave;
                }
            }
        }
    }

    // SeHardEdgeTrans kernels (their operation order differs from the main-beam HardEdgeTrans)
    static void se_quad(double* S, double qm, double G, double len) {
        const double uz = S[5], uy = S[3], ux = S[1];
        const double g2 = (uy * uy + uz * uz) + (ux * ux + 1.0);
        const double gb = std::sqrt((1.0 - 1.0 / g2) * g2);
        const double kq = std::sqrt(std::fabs((G * qm) / (gb * k::c)));
        const double iuz = 1.0 / uz;
        const double xa = iuz * ux, ya = iuz * uy;
        const double vz = std::sqrt((k::c2 - k::c2 / g2) / ((ya * ya + xa * xa) + 1.0));
        const double dt = len / vz;
        S[6] = dt + S[6];
        S[4] = dt;
        const double ph = len * kq, ik = 1.0 / kq;
        double sn, cs;
        dll_sincos(ph, sn, cs);
        S[1] = xa;
        const double x = S[0], y = S[2];
        double X, Xp, Y, Yp;
        if (G * qm <= 0.0) {
            const double ch = std::cosh(ph), sh = std::sinh(ph);
            X = sh * xa * ik + x * ch;
            Xp = kq * x * sh + ch * xa;
            const double yp = S[3] / S[5];
            S[3] = yp;
            Y = yp * sn * ik + cs * y;
            Yp = cs * yp - kq * y * sn;
        } else {
            X = xa * sn * ik + cs * x;
            Xp = cs * xa - kq * x * sn;
            const double yp = S[3] / S[5];
            S[3] = yp;
            const double ch = std::cosh(ph), sh = std::sinh(ph);
            Y = sh * yp * ik + ch * y;
            Yp = kq * y * sh + ch * yp;
        }
        S[0] = X;
        S[1] = S[5] * Xp;
        S[2] = Y;
        S[3] = S[5] * Yp;
    }

    static void se_solenoid(double* S, double qm, double B, double len) {
        const double uz = S[5], uy = S[3], ux = S[1];
        const double g2 = (uy * uy + uz * uz) + (ux * ux + 1.0);
        const double gb = std::sqrt((1.0 - 1.0 / g2) * g2);
        const double K = (k::half_inv_c / gb) * qm * B;
        const double iuz = 1.0 / uz;
        const double xa = iuz * ux, ya = iuz * uy;
        const double vz = std::sqrt((k::c2 - k::c2 / g2) / ((ya * ya + xa * xa) + 1.0));
        const double dt = len / vz;
        S[6] = dt + S[6];
        S[4] = dt;
        double sn, cs;
        dll_sincos(K * len, sn, cs);
        const double C2 = cs * cs, S2 = sn * sn, SC = cs * sn;
        S[1] = xa;
        S[3] = ya;
        const double iK = 1.0 / K;
        const double x = S[0], y = S[2];
        const double X = (S2 * ya + SC * xa) * iK + C2 * x + SC * y;
        const double Xp = ((C2 * xa - K * x * SC) - K * y * S2) + SC * ya;
        const double Y = (C2 * y - (S2 * xa * iK + SC * x)) + SC * ya * iK;
        const double Yp = ((K * x * S2 - SC * xa) - K * y * SC) + C2 * ya;
        S[0] = X;
        S[1] = S[5] * Xp;
        S[2] = Y;
        S[3] = S[5] * Yp;
    }

    void se_bend(double* S, const Element& e, double len, double ref) const {
        double arc = e.arc;
        if (len < arc * 0.85) arc = arc * 0.5;  // AVAS.dll: a short element is half a bend
        const double h = e.h, kx = e.kx, ky = e.ky;
        double sx, cx, sy, cy;
        dll_sincos(kx * arc, sx, cx);
        dll_sincos(ky * arc, sy, cy);
        const double uz = S[5], iuz = 1.0 / uz;
        const double ig2 = 1.0 / ((uz * uz + S[3] * S[3]) + (S[1] * S[1] + 1.0));
        const double b2 = 1.0 - ig2;
        const double xa = iuz * S[1], ya = iuz * S[3];
        const double vz = std::sqrt((b2 * k::c2) / ((xa * xa + ya * ya) + 1.0));
        const double dt = arc / vz;
        const double dl = (uz - ref) / ref;
        S[6] = dt + S[6];
        S[4] = dt;
        S[1] = xa;
        S[3] = ya;
        const double omc = 1.0 - cx, ikx = 1.0 / kx, ikx2 = 1.0 / (kx * kx);
        const double x = S[0], y = S[2];
        double X, Xp, Y, Yp;
        if (e.hv == 0) {
            X = sx * xa * ikx + cx * x + omc * h * dl * ikx2;
            Xp = (cx * xa - kx * x * sx) + h * dl * sx * ikx;
            if (ky == 0.0) {
                Y = ya * arc + y;
                Yp = ya;
            } else {
                Y = (ya * sy) / ky + cy * y;
                Yp = cy * ya - ky * y * sy;
            }
        } else {
            Y = sx * ya * ikx + cx * y + omc * h * dl * ikx2;
            Yp = (cx * ya - kx * y * sx) + h * dl * sx * ikx;
            if (ky == 0.0) {
                X = arc * xa + x;
                Xp = xa;
            } else {
                X = (xa * sy) / ky + cy * x;
                Xp = cy * xa - ky * x * sy;
            }
        }
        S[0] = X;
        S[1] = S[5] * Xp;
        S[2] = Y;
        S[3] = S[5] * Yp;
    }

    // SeZRun / SeHardEdgeTrans: transfer matrices of the hard-edge elements
    void se_zrun(const Segment& sg, Rec& S, int i, bool isSync) {
        Beam& b = b_;
        if (b.lost[i] != 0 || S[7] != 1.0) return;
        for (size_t j = 0; j < sg.elems.size(); ++j) {
            const int idx = sg.elems[j];
            const Element& e = lat_.elements[static_cast<size_t>(idx)];
            const double z0 = lat_.elemZStart[static_cast<size_t>(idx)], L = lat_.elemLength[static_cast<size_t>(idx)];
            const double qm = b.qOverM[i];
            // SeZRun passes the entrance and exit z and SeHardEdgeTrans takes their difference as
            // the length, which can differ from the element length in the last bit
            const double len = compat_ ? (z0 + L) - z0 : L;
            switch (e.kind) {
                case ElemKind::Quad: se_quad(S.data(), qm, e.strength, len); break;
                case ElemKind::Solenoid: se_solenoid(S.data(), qm, e.strength, len); break;
                case ElemKind::Edge: {
                    double k1, k2;
                    edge_k(e, k1, k2);
                    edge_row(S.data(), k1, k2, e.hv == 0 ? 0 : 1);
                    break;
                }
                case ElemKind::Bend: {
                    if (isSync) bendRef_[static_cast<size_t>(idx)] = S[5];
                    se_bend(S.data(), e, len, bendRef_[static_cast<size_t>(idx)]);
                    break;
                }
                default: break;
            }
            const double r = std::sqrt(S[2] * S[2] + S[0] * S[0]);
            if (e.radius <= r) {
                b.lost[i] = 1;
                return;
            }
            if (sg.marks[j] == -2) se_plane_out(i, S, L + z0, isSync);
        }
        const int lastIdx = sg.elems.back();
        const double zEnd = lat_.elemLength[static_cast<size_t>(lastIdx)] + lat_.elemZStart[static_cast<size_t>(lastIdx)];
        b.nextPlane[i] = 0;
        if (S[7] == 1.0) {
            b.t[i] = S[6];
            b.x[i] = S[0];
            b.y[i] = S[2];
            b.z[i] = zEnd;
            b.ux[i] = S[1];
            b.uy[i] = S[3];
            b.uz[i] = S[5];
        } else {
            b.lost[i] = 1;
        }
        S = Rec{};
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
    std::vector<Rec> planeRec_;
    std::vector<Rec> spRec_;  // superposeout exit crossings
    double segEnd_ = 1e300;   // end of the current time-step segment
    std::unique_ptr<SpaceCharge> sc_;
    std::vector<std::array<double, 6>> oldCoord_;
    AppendFile dataSet_, synFile_;

    // ---- external field of one macro particle (Lattice::GetExternalField + element fields) ----
    void external_field(int i, double t) {
        Beam& b = b_;
        const double z = b.z[i];
        if (z < 0.0 || lat_.sp_exit() < z || b.lost[i] != 0) {
            if (lat_.total_length() < z && lat_.spOutCur == static_cast<int>(lat_.spOut.size()) && b.lost[i] == 0)
                b.lost[i] = 2;
            b.ex[i] = b.ey[i] = b.ez[i] = b.bx[i] = b.by[i] = b.bz[i] = 0.0;
            return;
        }
        int k = b.elemIndex[i];
        const int last = static_cast<int>(lat_.elements.size()) - 1;
        if (z < lat_.elemZStart[k]) {
            do { --k; } while (k > 0 && z < lat_.elemZStart[k]);
        } else if (lat_.elemZEnd[k] <= z) {
            do { ++k; } while (k < last && lat_.elemZEnd[k] <= z);
        }
        if (k < 0) k = 0;
        if (k > last) k = last;
        b.elemIndex[i] = k;
        // Lattice::GetErrExternalField: elements marked in the error sample use GetErrField; the
        // synchronous particle always uses GetField
        if (!errUse_.empty() && i != b.n && errUse_[static_cast<size_t>(k)]) {
            element_err_field(i, t, lat_.elements[k]);
            return;
        }
        element_field(i, t, lat_.elements[k]);
    }

    // ---- error samples (spec 70 6, 7) ----
    std::vector<char> errUse_;  // t-code: element uses GetErrField
    std::vector<char> hetErr_;  // z-code: entrance / exit transform of the misalignment
    std::vector<Element> spEff_;  // corrected engine: superpose blocks with the sub-field errors applied

    // Field::GetErrField type 4 of AVAS.dll (spec 70 6.3): the sub-field placement of the block is
    // not used; an erroneous sub-field is displaced and rotated by its error only.
    void superpose_err_field_dll(int i, double t, const Element& e) {
        Beam& b = b_;
        double E[3] = {0, 0, 0}, B[3] = {0, 0, 0};
        auto store = [&] {
            b.ex[i] = E[0]; b.ey[i] = E[1]; b.ez[i] = E[2];
            b.bx[i] = B[0]; b.by[i] = B[1]; b.bz[i] = B[2];
        };
        const double x = b.x[i], y = b.y[i], z = b.z[i];
        for (size_t q = 0; q < e.subZStart.size(); ++q) {
            const auto& se = e.subErr.size() > q ? e.subErr[q] : std::array<double, 12>{};
            double xl = x, yl = y, zl = z - e.subZStart[q];
            const bool err = se[0] == 1.0;
            double sx = 0, cx = 1, sy = 0, cy = 1, sz = 0, cz = 1;
            if (err) {
                const double X0 = x - se[1], Y0 = y - se[2];
                dll_sincos(se[6], sz, cz);
                const double u = sz * Y0 + cz * X0;
                const double v = cz * Y0 - sz * X0;
                const double Z0 = (z - se[3]) - e.subZStart[q];
                dll_sincos(se[5], sy, cy);
                const double w = cy * Z0 + sy * u;
                xl = cy * u - sy * Z0;
                dll_sincos(se[4], sx, cx);
                yl = sx * w + cx * v;
                zl = cx * w - sx * v;
            }
            if (0.0 < zl && zl < e.subLength[q] && e.subRadius[q] <= std::sqrt(yl * yl + xl * xl)) {
                b.lost[i] = 1;
                store();
                return;
            }
            const int st = e.subType[q];
            if (st != 1 && st != 2 && st != 3) continue;
            const MapView<double> m = lat_.fm.maps[e.subMap[q]].view();
            if (!(m.xmin < xl && xl < m.xmax && m.ymin < yl && yl < m.ymax && 0.0 < zl && zl < m.zlen)) continue;
            double F[3] = {0, 0, 0}, G[3] = {0, 0, 0};
            if (st == 1) {
                trilinear(m, xl, yl, zl, [&](double wt, const double* f) {
                    for (int j = 0; j < 3; ++j) F[j] = f[j] * wt * se[7] + F[j];
                });
            } else if (st == 3) {
                const double ke6 = se[8] * 1000000.0;
                trilinear(m, xl, yl, zl, [&](double wt, const double* f) {
                    for (int j = 0; j < 3; ++j) F[j] = f[j] * wt * ke6 + F[j];
                });
            } else {
                const double ke6 = se[8] * 1000000.0;
                trilinear(m, xl, yl, zl, [&](double wt, const double* f) {
                    for (int j = 0; j < 3; ++j) {
                        F[j] = f[j] * wt * ke6 + F[j];
                        G[j] = f[3 + j] * wt * se[7] + G[j];
                    }
                });
                const int c = e.cavIndex;
                const double ph = (t - lat_.fm.cavTime[c]) * e.frequency * k::two_pi +
                                  (lat_.fm.cavPhase[c] + se[9]) * k::deg_to_rad;
                const double ce = dll_cos(ph), cb = dll_cos(ph - k::half_pi);
                for (int j = 0; j < 3; ++j) {
                    F[j] = ce * F[j];
                    G[j] = cb * G[j];
                }
            }
            auto turn = [&](double* V) {
                if (!err) return;
                const double v0 = V[0];
                const double a = sx * V[1] + cx * V[2];
                const double bb = sy * a + cy * v0;
                const double yy = cx * V[1] - sx * V[2];
                V[0] = cz * bb - sz * yy;
                V[1] = dll_sin(se[6]) * bb + cz * yy;
                V[2] = cy * a - sy * v0;
            };
            if (st == 1) {
                turn(F);
                for (int j = 0; j < 3; ++j) B[j] = (j == 0 ? B[j] + F[j] : F[j] + B[j]);
            } else if (st == 3) {
                turn(F);
                for (int j = 0; j < 3; ++j) E[j] = (j == 0 ? E[j] + F[j] : F[j] + E[j]);
            } else {
                turn(F);
                turn(G);
                for (int j = 0; j < 3; ++j) {
                    E[j] = (j == 0 ? E[j] + F[j] : F[j] + E[j]);
                    B[j] = (j == 0 ? B[j] + G[j] : G[j] + B[j]);
                }
            }
        }
        store();
    }

    void setup_errors() {
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

    // Field::GetErrField (spec 70 6.3): the particle in the frame of the displaced and rotated
    // field map, the field rotated back to the laboratory.  Other elements: as GetField.
    void element_err_field(int i, double t, const Element& e) {
        Beam& b = b_;
        if (e.kind == ElemKind::Field && e.fieldType == 4) {
            if (compat_) superpose_err_field_dll(i, t, e);
            else superpose_field(i, t, spEff_[static_cast<size_t>(b.elemIndex[i])]);
            return;
        }
        if (e.kind != ElemKind::Field || (e.fieldType != 1 && e.fieldType != 2 && e.fieldType != 3)) {
            element_field(i, t, e);
            return;
        }
        const double X0 = b.x[i] - e.errDx;
        const double Y0 = b.y[i] - e.errDy;
        double sz, cz, sy, cy, sx, cx;
        dll_sincos(e.errRotZ, sz, cz);
        const double X1 = X0 * cz + Y0 * sz;
        const double Y1 = Y0 * cz - X0 * sz;
        const double Z0 = (b.z[i] - e.errDz) - e.zStart;
        dll_sincos(e.errRotY, sy, cy);
        const double xl = cy * X1 - Z0 * sy;
        const double T = Z0 * cy + sy * X1;
        dll_sincos(e.errRotX, sx, cx);
        const double yl = sx * T + cx * Y1;
        const double zl = cx * T - sx * Y1;
        const double r = std::sqrt(yl * yl + xl * xl);
        // AVAS.dll checks the aperture without the boundary mode and the superposeout planes
        const bool inside = 0.0 < zl && zl < e.length;
        const bool lostHere = compat_ ? (inside && !(r < e.radius))
                                      : (inside && e.radius <= r && b.boundaryMode[i] == 0 &&
                                         e.zStart + 1e-06 < b.spOutZ1[i] && b.spOutZ0[i] < b.z[i]);
        if (lostHere) {
            b.lost[i] = 1;  // fields keep their previous values
            return;
        }
        const MapView<double> m = lat_.fm.maps[e.mapIndex].view();
        if (!(m.xmin < xl && xl < m.xmax && m.ymin < yl && yl < m.ymax && 0.0 < zl && zl < m.zlen)) {
            b.ex[i] = b.ey[i] = b.ez[i] = b.bx[i] = b.by[i] = b.bz[i] = 0.0;
            return;
        }
        double E[3] = {0, 0, 0}, B[3] = {0, 0, 0};
        if (e.fieldType == 1) {
            const double ke6 = e.keErr * 1000000.0;
            trilinear(m, xl, yl, zl, [&](double w, const double* f) {
                E[0] = f[0] * w * ke6 + E[0];
                E[1] = f[1] * w * ke6 + E[1];
                E[2] = f[2] * w * ke6 + E[2];
                B[0] = f[3] * w * e.kbErr + B[0];
                B[1] = f[4] * w * e.kbErr + B[1];
                B[2] = f[5] * w * e.kbErr + B[2];
            });
            const int c = e.cavIndex;
            const double phase = (t - lat_.fm.cavTime[c]) * e.frequency * k::two_pi +
                                 (lat_.fm.cavPhase[c] + e.phaseErr) * k::deg_to_rad;
            const double ce = dll_cos(phase);
            const double cb = dll_cos(phase - k::half_pi);
            for (int j = 0; j < 3; ++j) {
                E[j] = ce * E[j];
                B[j] = cb * B[j];
            }
        } else if (e.fieldType == 3) {
            trilinear(m, xl, yl, zl, [&](double w, const double* f) {
                for (int j = 0; j < 3; ++j) B[j] = f[j] * w * e.kbErr + B[j];
            });
        } else if (!compat_) {
            // static electric map: AVAS.dll returns zero here (never an error target there)
            const double ke6 = e.keErr * 1000000.0;
            trilinear(m, xl, yl, zl, [&](double w, const double* f) {
                for (int j = 0; j < 3; ++j) E[j] = f[j] * w * ke6 + E[j];
            });
        }
        auto back = [&](double* F) {
            const double a = F[2] * cx + F[1] * sx;   // z after the x rotation
            const double bb = a * sy + F[0] * cy;    // x after the y rotation
            const double yy = F[1] * cx - F[2] * sx;
            const double zz = a * cy - F[0] * sy;
            F[0] = bb * cz - yy * sz;
            F[1] = yy * cz + bb * sz;
            F[2] = zz;
        };
        back(E);
        back(B);
        b.ex[i] = E[0]; b.ey[i] = E[1]; b.ez[i] = E[2];
        b.bx[i] = B[0]; b.by[i] = B[1]; b.bz[i] = B[2];
    }

    // z-code misalignment (spec 30 2.8.3): entrance transform T_in into the element frame;
    // returns the local longitudinal position, which T_in drops.
    static double het_err_in(double* r, const Element& e) {
        const double X0 = r[0] - e.errDx, Y0 = r[2] - e.errDy, mdz = -e.errDz;
        double sz, cz, sy, cy, sx, cx;
        dll_sincos(e.errRotZ, sz, cz);
        const double u = sz * Y0 + cz * X0;
        const double v = cz * Y0 - sz * X0;
        dll_sincos(e.errRotY, sy, cy);
        dll_sincos(e.errRotX, sx, cx);
        const double px1 = cz * r[1] + sz * r[3];
        const double py1 = cz * r[3] - sz * r[1];
        const double pz1 = cy * r[5] + sy * px1;
        const double T = sy * u + cy * mdz;
        r[0] = cy * u - sy * mdz;
        r[1] = cy * px1 - sy * r[5];
        r[2] = v * cx + T * sx;
        r[3] = py1 * cx + sx * pz1;
        r[5] = pz1 * cx - sx * py1;
        return cx * T - sx * v;
    }

    // Exit transform T_out.  AVAS.dll adds the offsets before rotating back (and takes +dz as the
    // local z), so T_out is not the inverse of T_in when the element is rotated; the corrected
    // engine rotates back and then adds the offsets (exact inverse, zl from T_in).
    void het_err_out(double* r, const Element& e, double zl) const {
        double sz, cz, sy, cy, sx, cx;
        dll_sincos(e.errRotX, sx, cx);
        dll_sincos(e.errRotY, sy, cy);
        double q, w;
        if (compat_) {
            const double Y1 = e.errDy + r[2];
            q = (Y1 * sx + e.errDz * cx) * sy + (e.errDx + r[0]) * cy;
            w = Y1 * cx - e.errDz * sx;
        } else {
            w = cx * r[2] - sx * zl;
            const double T = sx * r[2] + cx * zl;
            q = cy * r[0] + sy * T;
        }
        dll_sincos(e.errRotZ, sz, cz);
        const double pxo = r[1];
        const double pz1 = r[5] * cx + r[3] * sx;
        const double px1 = pz1 * sy + pxo * cy;
        const double py1 = r[3] * cx - r[5] * sx;
        r[0] = q * cz - sz * w;
        r[1] = px1 * cz - py1 * sz;
        r[2] = q * sz + cz * w;
        r[3] = py1 * cz + px1 * sz;
        r[5] = pz1 * cy - pxo * sy;
        if (!compat_) {
            r[0] = r[0] + e.errDx;
            r[2] = r[2] + e.errDy;
        }
    }

    bool aperture_lost(int i, const Element& e) const {
        const Beam& b = b_;
        const double r = std::sqrt(b.x[i] * b.x[i] + b.y[i] * b.y[i]);
        return e.radius <= r && b.boundaryMode[i] == 0 && e.zStart + 1e-06 < b.spOutZ1[i] && b.spOutZ0[i] < b.z[i];
    }

    void element_field(int i, double t, const Element& e) {
        Beam& b = b_;
        if (e.kind == ElemKind::Drift) {
            if (aperture_lost(i, e)) b.lost[i] = 1;
            b.ex[i] = b.ey[i] = b.ez[i] = b.bx[i] = b.by[i] = b.bz[i] = 0.0;
            return;
        }
        if (e.kind != ElemKind::Field) {
            // t-code fields of the hard-edge elements: quad and solenoid simplified, others none
            b.ex[i] = b.ey[i] = b.ez[i] = b.bx[i] = b.by[i] = b.bz[i] = 0.0;
            if (e.kind == ElemKind::Quad) {
                b.bx[i] = b.y[i] * e.strength;
                b.by[i] = e.strength * b.x[i];
            } else if (e.kind == ElemKind::Solenoid) {
                b.bz[i] = e.strength;
            }
            return;
        }
        const int type = e.fieldType;
        if (type == 4) {
            superpose_field(i, t, e);
            return;
        }
        if (type != 1 && type != 2 && type != 3) return;
        if (aperture_lost(i, e)) {
            b.lost[i] = 1;  // fields keep their previous values (cleared on the next step)
            return;
        }
        const MapView<double> m = lat_.fm.maps[e.mapIndex].view();
        const double x = b.x[i], y = b.y[i];
        if (!(m.xmin < x && x < m.xmax && m.ymin < y && y < m.ymax && e.zStart < b.z[i] &&
              b.z[i] < e.zStart + m.zlen)) {
            b.ex[i] = b.ey[i] = b.ez[i] = b.bx[i] = b.by[i] = b.bz[i] = 0.0;
            return;
        }
        const double zl = b.z[i] - e.zStart;
        if (type == 1) {
            double se[3] = {0, 0, 0}, sb[3] = {0, 0, 0};
            const double ke6 = e.ke * 1000000.0;
            trilinear(m, x, y, zl, [&](double w, const double* f) {
                se[0] = w * f[0] * ke6 + se[0];
                se[1] = f[1] * w * ke6 + se[1];
                se[2] = f[2] * w * ke6 + se[2];
                sb[0] = f[3] * w * e.kb + sb[0];
                sb[1] = f[4] * w * e.kb + sb[1];
                sb[2] = f[5] * w * e.kb + sb[2];
            });
            const int c = e.cavIndex;
            const double phase =
                (t - lat_.fm.cavTime[c]) * e.frequency * k::two_pi + lat_.fm.cavPhase[c] * k::deg_to_rad;
            const double ce = dll_cos(phase);
            const double cb = dll_cos(phase - k::half_pi);
            b.ex[i] = ce * se[0];
            b.bx[i] = cb * sb[0];
            b.ey[i] = ce * se[1];
            b.by[i] = cb * sb[1];
            b.ez[i] = ce * se[2];
            b.bz[i] = cb * sb[2];
        } else if (type == 2) {
            double se[3] = {0, 0, 0};
            trilinear(m, x, y, zl, [&](double w, const double* f) {
                for (int j = 0; j < 3; ++j) se[j] = w * f[j] * (e.ke * 1000000.0) + se[j];
            });
            b.ex[i] = se[0]; b.ey[i] = se[1]; b.ez[i] = se[2];
            b.bx[i] = b.by[i] = b.bz[i] = 0.0;
        } else {
            double sb[3] = {0, 0, 0};
            trilinear(m, x, y, zl, [&](double w, const double* f) {
                for (int j = 0; j < 3; ++j) sb[j] = w * f[j] * e.kb + sb[j];
            });
            b.ex[i] = b.ey[i] = b.ez[i] = 0.0;
            b.bx[i] = sb[0]; b.by[i] = sb[1]; b.bz[i] = sb[2];
        }
    }

    // Field::GetField type 4: sum of the sub-fields of a superpose block (spec 30 2.4.2).
    void superpose_field(int i, double t, const Element& e) {
        Beam& b = b_;
        double E[3] = {0, 0, 0}, B[3] = {0, 0, 0};
        auto store = [&] {
            b.ex[i] = E[0]; b.ey[i] = E[1]; b.ez[i] = E[2];
            b.bx[i] = B[0]; b.by[i] = B[1]; b.bz[i] = B[2];
        };
        const double x = b.x[i], y = b.y[i], z = b.z[i];
        const size_t nsub = e.subZStart.size();
        for (size_t j = 0; j < nsub; ++j) {
            const auto& P = e.subPlace[j];
            const double dz = z - e.subZStart[j];
            const double X0 = x - P[1], Y0 = y - P[2];
            double s3, c3, s4, c4, s5, c5;
            dll_sincos(P[3], s3, c3);
            const double u = c3 * X0 + s3 * Y0;
            const double v = Y0 * c3 - X0 * s3;
            dll_sincos(P[5], s5, c5);
            const double xl = c5 * u - dz * s5;
            const double w = u * s5 + dz * c5;
            dll_sincos(P[4], s4, c4);
            const double zl = w * c4 - v * s4;
            const double yl = s4 * w + v * c4;
            if (0.0 < zl && zl < e.subLength[j] && e.zStart + 1e-06 < b.spOutZ1[i] && b.spOutZ0[i] < z) {
                const double r = e.rotMode == 0 ? std::sqrt(xl * xl + yl * yl)
                                                : std::sqrt(b.rel[0][i] * b.rel[0][i] + b.rel[2][i] * b.rel[2][i]);
                if (e.subRadius[j] <= r) {
                    b.lost[i] = 1;
                    store();  // the partial sum stays in the particle (as the original)
                    return;
                }
            }
            const int st = e.subType[j];
            if (st != 1 && st != 2 && st != 3) continue;
            const MapView<double> m = lat_.fm.maps[e.subMap[j]].view();
            if (!(m.xmin < xl && xl < m.xmax && m.ymin < yl && yl < m.ymax && 0.0 < zl && zl < m.zlen)) continue;
            double S[3] = {0, 0, 0}, SB[3] = {0, 0, 0};
            if (st == 1) {
                trilinear(m, xl, yl, zl, [&](double wt, const double* f) {
                    for (int q = 0; q < 3; ++q) S[q] = wt * f[q] * e.subKb[j] + S[q];
                });
            } else if (st == 3) {
                trilinear(m, xl, yl, zl, [&](double wt, const double* f) {
                    for (int q = 0; q < 3; ++q) S[q] = wt * f[q] * e.subKe[j] * 1000000.0 + S[q];
                });
            } else {
                const double ke6 = e.subKe[j] * 1000000.0;
                trilinear(m, xl, yl, zl, [&](double wt, const double* f) {
                    S[0] = wt * f[0] * ke6 + S[0];
                    S[1] = f[1] * wt * ke6 + S[1];
                    S[2] = f[2] * wt * ke6 + S[2];
                    SB[0] = f[3] * wt * e.subKb[j] + SB[0];
                    SB[1] = f[4] * wt * e.subKb[j] + SB[1];
                    SB[2] = f[5] * wt * e.subKb[j] + SB[2];
                });
            }
            dll_sincos(P[4], s4, c4);
            dll_sincos(P[5], s5, c5);
            dll_sincos(P[3], s3, c3);
            if (st == 1 || st == 3) {
                double* D = st == 1 ? B : E;
                double* O = st == 1 ? E : B;
                const double a = S[1] * c4 - s4 * S[2];
                const double bb = S[1] * s4 + c4 * S[2];
                const double cz = bb * c5;
                const double d = S[0] * c5 + s5 * bb;
                D[0] = (c3 * d - a * s3) + D[0];
                D[1] = (s3 * d + a * c3) + D[1];
                D[2] = (cz - s5 * S[0]) + D[2];
                for (int q = 0; q < 3; ++q) O[q] = O[q] + 0.0;
            } else {
                const int c = e.cavIndex;
                const double phase =
                    (t - lat_.fm.cavTime[c]) * e.frequency * k::two_pi + (lat_.fm.cavPhase[c] + e.phaseErr) * k::deg_to_rad;
                const double cE = dll_cos(phase);
                const double cB = dll_cos(phase - k::half_pi);
                const double a = cE * S[1] * c4 - cE * S[2] * s4;
                const double bb = s4 * cE * S[1] + cE * S[2] * c4;
                const double cz = bb * c5;
                const double d = S[0] * cE * c5 + s5 * bb;
                E[0] = (c3 * d - a * s3) + E[0];
                E[1] = (d * s3 + a * c3) + E[1];
                E[2] = (cz - s5 * S[0] * cE) + E[2];
                const double a2 = cB * SB[1] * c4 - cB * SB[2] * s4;
                const double b2 = cB * SB[1] * s4 + cB * SB[2] * c4;
                const double cz2 = b2 * c5;
                const double d2 = SB[0] * cB * c5 + s5 * b2;
                B[0] = (c3 * d2 - a2 * s3) + B[0];
                B[1] = (s3 * d2 + a2 * c3) + B[1];
                B[2] = (cz2 - s5 * SB[0] * cB) + B[2];
            }
        }
        store();
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
        const double p2 = zrow_p2(r);
        steerer_kick(r, st.steerKind, st.steerX, st.steerY, len * qm);
        // thin kick: the particle does not move across the field, so |p| is kept (AVAS.dll keeps uz)
        if (!compat_) zrow_keep_momentum(r, p2);
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

    void het(WorkerPool& pool, int idx, double zFrom, double zTo, bool half, int& mode, double& tRef) {
        const Element& e = lat_.elements[idx];
        const int n = b_.n;
        auto& rec = planeRec_;
        const double len = zTo - zFrom;
        auto sc = [](double a, double& s, double& c) { dll_sincos(a, s, c); };
        std::function<void(int)> one;
        if (e.kind == ElemKind::Edge) {
            double k1, k2;
            edge_k(e, k1, k2);
            const int hv = e.hv == 0 ? 0 : 1;
            const bool err = !hetErr_.empty() && hetErr_[static_cast<size_t>(idx)];
            for_rows(pool, [&](int i) {
                double* r = rec[i].data();
                const double p2 = zrow_p2(r);
                const double zl = err ? het_err_in(r, e) : 0.0;
                edge_row(r, k1, k2, hv);
                if (err) het_err_out(r, e, zl);
                if (!compat_) zrow_keep_momentum(r, p2);
            });
            return;
        }
        if (e.kind == ElemKind::Quad) {
            one = [&](int i) { quad_row(rec[i].data(), b_.qOverM[i], e.strength, len, sc); };
        } else if (e.kind == ElemKind::Solenoid) {
            one = [&](int i) { solenoid_row(rec[i].data(), b_.qOverM[i], e.strength, len, sc); };
        } else if (e.kind == ElemKind::Bend) {
            const BendStep<double> bs = bend_step(e, zFrom, zTo, half);
            const double uzs = rec[n][5];
            one = [&, bs, uzs](int i) { bend_row(rec[i].data(), uzs, bs); };
        } else {
            return;
        }
        if (mode == 1) {
            // space-charge kick of both half steps (dt of the first half twice), lab frame
            const auto inner = one;
            one = [&, inner](int i) {
                double* r = rec[i].data();
                double a = (b_.qOverM[i] / k::c) * r[4];
                a = a + a;
                r[1] = b_.scx[i] * a + r[1];
                r[3] = b_.scy[i] * a + r[3];
                r[5] = a * b_.scz[i] + r[5];
                inner(i);
            };
        }
        if (!hetErr_.empty() && hetErr_[static_cast<size_t>(idx)]) {
            // misaligned element: T_in before the space-charge kick, T_out after the transfer
            const auto inner = one;
            one = [&, inner](int i) {
                double* r = rec[i].data();
                const double zl = het_err_in(r, e);
                inner(i);
                het_err_out(r, e, zl);
            };
        }
        const bool average = mode == 2;
        mode = 0;
        if (compat_) {
            for_rows(pool, one);
        } else {
            // magnetic elements: |gamma*beta| of every particle is conserved
            for_rows(pool, [&](int i) {
                const double p2 = zrow_p2(rec[i].data());
                one(i);
                zrow_keep_momentum(rec[i].data(), p2);
            });
        }
        if (average) {
            // AVAS.dll sums with atomics in thread completion order (last bits not reproducible);
            // the value is always replaced by the second half-step that follows.
            double sT = 0.0, sC = 0.0;
            for (int i = 0; i < n; ++i)
                if (rec[i][7] == 1.0) { sT = sT + rec[i][6]; sC = sC + 1.0; }
            tRef = sT / sC;
        } else {
            tRef = 1e10;
            for (int i = 0; i < n; ++i)
                if (rec[i][7] == 1.0 && rec[i][6] < tRef) tRef = rec[i][6];
        }
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
        for (int i = 0; i < n; ++i) {
            b.nextPlane[i] = 0;
            Rec& r = planeRec_[i];
            if (r[7] == 1.0)
                ztot_row(r.data(), zPlane, tRef, !compat_, b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]);
            r.fill(0.0);
        }
        Rec& r = planeRec_[n];
        double x, ux, y, uy, z, uz;
        ztot_row(r.data(), zPlane, tRef, !compat_, x, ux, y, uy, z, uz);
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
                    hesc_field_pre(b_, planeRec_, tRef, p_.cores[2]);
                    const double syncRow[6] = {planeRec_[n][0], planeRec_[n][1], planeRec_[n][2], planeRec_[n][3], 0.0, planeRec_[n][5]};
                    // recordSCF centre: AVAS.dll keeps the synchronous coordinates of the last
                    // t-code step (only the SpaceChargeMesh.txt line uses them)
                    if (sc_->importing()) sc_->import_scf(b_);
                    else sc_->solve(b_, b_.scComp, compat_ ? oldCoord_[n].data() : syncRow);
                    mode = 1;
                }
                if (mark == -1) {
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
                    for (int i = 0; i < n; ++i)
                        if (planeRec_[i][7] == 1.0) kick(planeRec_[i].data(), lat_.elements[mark], e.length, i);
                }
                het(pool, idx, zMid, L + z0, true, mode, tRef);
            }
            sync_het(e, planeRec_[n].data(), false);
            zEnd = L + z0;
            if (p_.dumpPeriodicity != 0) beam_out(1, tRef, zEnd, step_);
            step_ += 1;
            if (mark == -2) dst_out(zEnd, planeRec_);
        }
        t = ztot(zEnd, tRef);
        zSeg = zEnd;
    }

    // A linear interpolation of the momentum vector within a step shortens it when a magnetic
    // field turns it (by about (turn angle)^2 / 8); the corrected engine interpolates the
    // direction linearly and the magnitude separately.
    static void interp_momentum_norm(double* rec, const double* a, const double* b, double r) {
        const double pa = std::sqrt(a[1] * a[1] + a[3] * a[3] + a[5] * a[5]);
        const double pb = std::sqrt(b[1] * b[1] + b[3] * b[3] + b[5] * b[5]);
        const double pr = std::sqrt(rec[1] * rec[1] + rec[3] * rec[3] + rec[5] * rec[5]);
        if (!(pr > 0.0)) return;
        const double f = ((pb - pa) * r + pa) / pr;
        rec[1] *= f;
        rec[3] *= f;
        rec[5] *= f;
    }

    // Coordinates {x, ux, y, uy, z, uz} in the frame of a superposeout exit (spec 20 2.6.6).
    static void sp_local(const std::array<double, 7>& row, double zStart, const double* c, double* o) {
        double s3, c3, s4, c4, s5, c5;
        dll_sincos(row[3], s3, c3);
        dll_sincos(row[5], s5, c5);
        dll_sincos(row[4], s4, c4);
        auto turn = [&](double X, double Y, double Z, double& xo, double& yo, double& zo) {
            const double X1 = c3 * X + s3 * Y;
            const double Y1 = c3 * Y - s3 * X;
            const double X2 = c5 * X1 - s5 * Z;
            const double Z2 = s5 * X1 + c5 * Z;
            zo = c4 * Z2 - s4 * Y1;
            yo = c4 * Y1 + s4 * Z2;
            xo = X2;
        };
        turn(c[0] - row[1], c[2] - row[2], (c[4] - row[0]) - zStart, o[0], o[2], o[4]);
        turn(c[1], c[3], c[5], o[1], o[3], o[5]);
    }

    // ---- one time step of particles [i0, i1) (TcodeTrans) ----
    void trans(int i0, int i1, double zField, const std::vector<double>& events, double tEnd, double gp,
               const double sync[6], BlockAcc& acc) {
        Beam& b = b_;
        const int n = b.n;
        const double* cen = b.centroid.data();
        for (int i = i0; i < i1; ++i) {
            if (b.lost[i] != 1 && i != n) {
                const double w = b.weight[i];
                const double d0 = b.rel[0][i] - cen[0];
                const double d2 = b.rel[2][i] - cen[2];
                const double d4 = b.rel[4][i] - cen[4];
                const double inv = 1.0 / b.rel[5][i];
                const double d1 = (b.rel[1][i] - cen[1]) * inv;
                const double d3 = (b.rel[3][i] - cen[3]) * inv;
                const double d5 = (b.rel[5][i] - cen[5]) * inv;
                if (b.z[i] < acc.zmin) acc.zmin = b.z[i];
                if (acc.zmax < b.z[i]) acc.zmax = b.z[i];
                acc.sumGz2 = gp * d4 * gp * d4 * w + acc.sumGz2;
                const double d[6] = {d0, d1, d2, d3, d4, d5};
                for (int j = 0; j < 6; ++j) {
                    const double a = std::fabs(d[j]);
                    if (acc.maxAbs[j] < a) acc.maxAbs[j] = a;
                    acc.sum2[j] = d[j] * d[j] * w + acc.sum2[j];
                }
                acc.cross[0] = d1 * d0 * w + acc.cross[0];
                acc.cross[1] = d3 * d2 * w + acc.cross[1];
                acc.cross[2] = d5 * d4 * w + acc.cross[2];
            }
            double tot[6];
            if (b.zMin <= zField && !(zField < b.z[i])) {
                for (double& v : tot) v = 0.0;
            } else {
                tot[0] = b.ex[i] + b.scx[i]; tot[1] = b.ey[i] + b.scy[i]; tot[2] = b.ez[i] + b.scz[i];
                tot[3] = b.bx[i] + 0.0; tot[4] = b.by[i] + 0.0; tot[5] = b.bz[i] + 0.0;
            }
            auto& old = oldCoord_[i];
            old = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
            if (b.lost[i] == 1) continue;
            boris_push(b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i], tot, tot + 3, b.qOverM[i], dt_);
            if (b.rotMode == 0) {
                b.rel[0][i] = b.x[i] - sync[0];
                b.rel[2][i] = b.y[i] - sync[2];
                b.rel[4][i] = b.z[i] - sync[4];
                b.rel[1][i] = b.ux[i];
                b.rel[3][i] = b.uy[i];
                b.rel[5][i] = b.uz[i];
            } else {
                // coordinates relative to the synchronous particle, turned by the beam angle
                double sn, cs;
                dll_sincos(b.rotAngle, sn, cs);
                const double dz = b.z[i] - sync[4];
                if (b.rotMode == 2) {
                    const double dx = b.x[i] - sync[0];
                    b.rel[0][i] = cs * dx - sn * dz;
                    b.rel[4][i] = sn * dx + cs * dz;
                    b.rel[1][i] = b.ux[i] * cs - b.uz[i] * sn;
                    b.rel[5][i] = b.uz[i] * cs + b.ux[i] * sn;
                    b.rel[2][i] = b.y[i] - sync[2];
                    b.rel[3][i] = b.uy[i];
                } else {
                    const double dy = b.y[i] - sync[2];
                    b.rel[2][i] = cs * dy + sn * dz;
                    b.rel[4][i] = dz * cs - dy * sn;
                    b.rel[3][i] = b.uy[i] * cs + b.uz[i] * sn;
                    b.rel[5][i] = b.uz[i] * cs - b.uy[i] * sn;
                    b.rel[0][i] = b.x[i] - sync[0];
                    b.rel[1][i] = b.ux[i];
                }
            }
            external_field(i, tEnd);
            if (!compat_ && b.z[i] > segEnd_) {
                // past the end of this time-step segment the particle is already recorded for the
                // next (hard-edge) segment: it drifts without field (AVAS.dll applies the simplified
                // t-code fields of the hard-edge elements there)
                b.ex[i] = b.ey[i] = b.ez[i] = b.bx[i] = b.by[i] = b.bz[i] = 0.0;
            }
            if (b.uz[i] < 0.0 && lat_.spOutCount <= lat_.spOutCur && b.z[i] < 0.0) b.lost[i] = 1;
            const size_t m = static_cast<size_t>(b.nextPlane[i]);
            if (m + 1 <= events.size()) {
                const double pl = events[m];
                if (pl <= b.z[i] && pl <= lat_.sp_exit()) {
                    const double r = (pl - old[4]) / (b.z[i] - old[4]);
                    Rec& rec = planeRec_[i];
                    const double cur[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
                    for (int j = 0; j < 6; ++j) rec[j] = (cur[j] - old[j]) * r + old[j];
                    if (!compat_) interp_momentum_norm(rec.data(), old.data(), cur, r);
                    rec[6] = (r - 1.0) * dt_ + tEnd;
                    rec[7] = 1.0;
                    b.nextPlane[i] += 1;
                }
            }
            // superposeout exit crossing (recorded in the exit frame; applied by tcode)
            if (lat_.spOutCur < lat_.spOutCount && lat_.elements[b.elemIndex[i]].inSuperposeOut != 0 &&
                spRec_[i][7] == 0.0 && b.z[i] > b.spOutZ0[i]) {
                const auto& row = lat_.spOut[lat_.spOutCur];
                const double zs = lat_.elements[b.elemIndex[i]].zStart;
                const double cur[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
                double lc[6], lp[6];
                sp_local(row, zs, cur, lc);
                sp_local(row, zs, old.data(), lp);
                if (lc[4] > 0.0) {
                    const double r = lp[4] / (lp[4] - lc[4]);
                    Rec& sr = spRec_[i];
                    for (int j = 0; j < 6; ++j) sr[j] = (lc[j] - lp[j]) * r + lp[j];
                    if (!compat_) interp_momentum_norm(sr.data(), lp, lc, r);
                    sr[6] = (r - 1.0) * dt_ + tEnd;
                    sr[7] = 1.0;
                    acc.spCnt += 1;
                }
            }
            if (b.lost[i] != 1 && i != n) {
                const double w = b.weight[i];
                acc.cnt0 = acc.cnt0 + 1.0;
                acc.cnt1 = w + acc.cnt1;
                for (int j = 0; j < 6; ++j) acc.sum1[j] = w * b.rel[j][i] + acc.sum1[j];
                const double u2 = compat_ ? b.uz[i] * b.uz[i]
                                          : b.ux[i] * b.ux[i] + b.uy[i] * b.uy[i] + b.uz[i] * b.uz[i];
                acc.gsum = std::sqrt(u2 + 1.0) * w + acc.gsum;
            }
        }
    }

    // Centroid, mean gamma, zMin/zMax at the start of a time-step segment (UpdateBeamParameters).
    void update_beam_parameters() {
        Beam& b = b_;
        const int n = b.n;
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

    void progress(const std::vector<double>& events) {
        const Beam& b = b_;
        const double zs = b.z[b.n];
        const double pct = (zs - zStartRun_) / ((p_.outputPlanes.back() - zStartRun_) + zs - b.zMin) * 100.0;
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
        const int nBlocks = p_.cores[0];
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
                if (sc_->importing()) sc_->import_scf(b);
                else sc_->solve(b, b.scComp, oldCoord_[n].data());
            }
            pool.run(nBlocks, [&](int k) {
                trans(bounds[k], bounds[k + 1], zSeg, events, t, gp, sync, acc[k]);
            });
            pushTime_ += std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - tp0).count();
            if (n == 1) single_out(t);
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
            dataSet_.write(row.str());
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
                        zSeg = zev;
                        const std::vector<double> ev2(events.begin() + static_cast<long>(iEv), events.end());
                        const std::vector<int> mk2(marks.begin() + static_cast<long>(iEv), marks.end());
                        tcode(t, zSeg, ev2, mk2);
                        return;
                    }
                    if (m != -4) {  // -4: spacechargecomp change (no output)
                        dst_out(zev, planeRec_);
                        for (auto& r : planeRec_) r.fill(0.0);
                    }
                }
            }
            const auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration<double>(now - lastProgress).count() > 3.0) {
                lastProgress = now;
                progress(events);
            }
            t = dt_ + t;
            trans(n, n + 1, zSeg, events, t, b.gamma, sync, syncAcc);
            syncAcc.reset();
            rot_mode(sync);
            if (p_.dumpPeriodicity != 0 && step_ % p_.dumpPeriodicity == 0) beam_out(0, t - dt_, b.z[n], step_);
            if (p_.pcHistogram[0] == 1.0) ex_output();
            step_ += 1;
            // particles that stop crossing the next plane are declared lost
            int nc = 0;
            for (int i = 0; i < n; ++i)
                if (planeRec_[i][7] == 1.0) ++nc;
            if (nc == lastCross && nc != 0) stall += 1;
            else stall = 0;
            lastCross = nc;
            if (static_cast<double>(stall) > 0.8 * static_cast<double>(step_)) {
                Text lt;
                for (int i = 0; i < n; ++i) {
                    if (planeRec_[i][7] == 0.0) {
                        b.lost[i] = 1;
                        lt.i(i).fixed(b.x[i], 12).s("  ").fixed(b.ux[i], 12).s("  ").fixed(b.y[i], 12).s("  ")
                            .fixed(b.uy[i], 12).s("  ").fixed(b.z[i], 12).s("  ").fixed(b.uz[i], 12).nl();
                    }
                }
                write_file(paths().output + "LongitudinalBeamLoss.txt", lt.str(), true);
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
        for (int i = 0; i < n; ++i) {
            if (b.lost[i] != 0) continue;
            while (true) {
                while (b.z[i] - b.z[n] > half) b.z[i] = b.z[i] - full;
                if (-half <= b.z[i] - b.z[n]) break;
                b.z[i] = b.z[i] + full;
            }
        }
    }

    // track: one line per listed particle (SingleTrack -> ParticleTracking.txt)
    void single_track() {
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
        for (int i = 0; i < n; ++i) {
            const double gi = compat_ ? std::sqrt(b.uz[i] * b.uz[i] + 1.0)
                                      : std::sqrt(b.ux[i] * b.ux[i] + b.uy[i] * b.uy[i] + b.uz[i] * b.uz[i] + 1.0);
            if (std::fabs((gi - 1.0) * b.restMass[i] - ws) > p_.longLimits[2]) b.lost[i] = 1;
            if (std::fabs(b.z[i] - b.z[n]) > dzMax) b.lost[i] = 1;
        }
    }

    // ---- boundary.txt (tcodeRun 4.2.4, Geometry::Boundary) ----
    void boundary_check(double t) {
        Beam& b = b_;
        const Boundary& bd = lat_.bnd;
        for (int i = 0; i < b.n; ++i) {
            const double z = b.z[i];
            if (z < 0.0 || b.lost[i] != 0 || z >= bd.segEnd.back() || z < b.spOutZ0[i]) {
                if (z > lat_.total_length() && lat_.spOutCur == static_cast<int>(lat_.spOut.size()) && b.lost[i] == 0)
                    b.lost[i] = 2;
                continue;
            }
            const size_t ri = static_cast<size_t>(b.spOutCount[i]);
            if (ri >= bd.rows.size()) continue;
            const auto& row = bd.rows[ri];
            const double zz = row[0] + z;
            const int r1 = static_cast<int>(std::lround(row[1])), r2 = static_cast<int>(std::lround(row[2]));
            if (!(bd.segStart[static_cast<size_t>(r1)] <= zz && zz <= bd.segEnd[static_cast<size_t>(r2)])) continue;
            int gi = b.geomIndex[i];
            if (compat_ ? (r2 < gi && gi < r1) : (gi < r1 || r2 < gi)) gi = compat_ ? r2 : r1;
            while (gi > 0 && zz < bd.segStart[static_cast<size_t>(gi)]) --gi;
            while (gi + 1 < static_cast<int>(bd.segEnd.size()) && bd.segEnd[static_cast<size_t>(gi)] <= zz) ++gi;
            b.geomIndex[i] = gi;
            geometry_hit(bd.geoms[static_cast<size_t>(gi)], t, i);
        }
    }

    static void geom_local(const Geometry& g, double x, double y, double z, double& u, double& v, double& zl) {
        const auto& p = g.place;
        double s3, c3, s4, c4, s5, c5;
        const double dx = x - p[1], dy = y - p[2];
        dll_sincos(p[3], s3, c3);
        const double u0 = s3 * dy + c3 * dx;
        const double v0 = c3 * dy - s3 * dx;
        const double dz = (z - g.zStart) - p[0];
        dll_sincos(p[5], s5, c5);
        const double w = dz * c5 + u0 * s5;
        u = u0 * c5 - dz * s5;
        dll_sincos(p[4], s4, c4);
        v = w * s4 + v0 * c4;
        zl = w * c4 - v0 * s4;
    }

    void geometry_hit(const Geometry& g, double t, int i) {
        Beam& b = b_;
        double u, v, zl;
        geom_local(g, b.x[i], b.y[i], b.z[i], u, v, zl);
        if (!(0.0 < zl && zl < g.par.at(0))) return;
        double P[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
        double tHit = t;
        const auto& old = oldCoord_[static_cast<size_t>(i)];
        if (g.shape == 1) {
            const double r = std::sqrt(v * v + u * u);
            const double R = g.par.at(1);
            if (r < R) return;
            double u0, v0, zl0;
            geom_local(g, old[0], old[2], old[4], u0, v0, zl0);
            const double r0 = std::sqrt(v0 * v0 + u0 * u0);
            double f = (R - r0) / (r - r0);
            const double zh = (zl - zl0) * f + zl0;
            if (zh < 0.0) f = -(zl0 / (zl - zl0));
            for (int j = 0; j < 6; ++j) P[j] = (P[j] - old[j]) * f + old[j];
            tHit = t - (1.0 - f) * dt_;
        } else {
            const double rho = g.par.at(2);
            double phi = 0.0, cx = 0.0, cy = 0.0, cz = 0.0;
            double s, c;
            const double plane = g.par.size() > 4 ? g.par[4] : 0.0, dir = g.par.size() > 5 ? g.par[5] : 0.0;
            if (plane == 0.0) {
                if (dir <= 0.0) {
                    phi = std::atan(zl / (u + rho));
                    dll_sincos(phi, s, c);
                    cx = (c - 1.0) * rho;
                } else {
                    phi = std::atan(zl / (rho - u));
                    dll_sincos(phi, s, c);
                    cx = (1.0 - c) * rho;
                }
                cz = s * rho;
            } else if (plane == 1.0) {
                if (dir <= 0.0) {
                    phi = std::atan(zl / (rho + v));
                    dll_sincos(phi, s, c);
                    cy = (c - 1.0) * rho;
                } else {
                    phi = std::atan(zl / (rho - v));
                    dll_sincos(phi, s, c);
                    cy = (1.0 - c) * rho;
                }
                cz = s * rho;
            }
            if (!(phi <= std::fabs(g.par.at(3)) * 0.017453292519943295)) return;
            const double du = u - cx;
            const double d = std::sqrt((v - cy) * (v - cy) + du * du + (zl - cz) * (zl - cz));
            if (d < g.par.at(1)) return;
        }
        const double gm = g.shape == 1 ? std::sqrt(P[5] * P[5] + P[3] * P[3] + P[1] * P[1] + 1.0)
                                       : std::sqrt(P[3] * P[3] + P[5] * P[5] + P[1] * P[1] + 1.0);
        const double ig = 1.0 / gm;
        Text s;
        s.g(P[0]).s(" ").g(P[2]).s("  ").g(P[4]).s("  ").g(P[1] * k::c * ig).s("  ").g(P[3] * k::c * ig).s("  ")
            .g(P[5] * k::c * ig).s("  ").g(b.charge[i] * 6.241509479607718e+18).s("  ").g(b.restMass[i]).s("  ")
            .g(b.weight[i]).s("  ").g(tHit).s("  ").s(b.kind.empty() ? p_.kind : b.kind[static_cast<size_t>(i)]).nl();
        write_file(paths().output + "collisionData.txt", s.str(), true);
        b.lost[i] = 1;
    }

    void final_output(double zev, double t) {
        full_particle_out();
        dst_out(zev, planeRec_);
        beam_out(0, t, b_.z[b_.n], step_);
    }
};

}  // namespace

void run_tracking(Lattice& lat, Beam& beam, Params& p) {
    Tracker(lat, beam, p).run();
}

}  // namespace avas
