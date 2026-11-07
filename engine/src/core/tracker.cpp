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
#include <mutex>
#include <thread>
#include <vector>

#include "core/dllmath.h"
#include "core/engine.h"
#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/physics.h"

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
    Tracker(Lattice& lat, Beam& b, Params& p) : lat_(lat), b_(b), p_(p), compat_(compat_dll()) {}

    void run() {
        const auto t0 = std::chrono::steady_clock::now();
        const int n = b_.n;
        planeRec_.assign(static_cast<size_t>(n) + 1, Rec{});
        dt_ = (1.0 / b_.frequency) / p_.stepPerCycle;
        say(Text().s("The total distance simulated is ").g(lat_.total_length()).s(" (m)").str());
        say(Text().s("Simulation time step is ").g(dt_).s(" (m)").str());
        say("Start simulation, start time " + time_stamp());
        const std::string& out = paths().output;
        for (const char* f : {"DataSet.txt", "synParticle.txt", "test.txt", "testP.txt", "collisionData.txt",
                              "LongitudinalBeamLoss.txt", "BeamSet.plt", "ExData.edt", "FullParticle.txt",
                              "ParticleTracking.txt"})
            write_file(out + f, "", false);
        dataSet_.open(out + "DataSet.txt");
        synFile_.open(out + "synParticle.txt");

        // M1: every element is a time-step element; one segment with the output-plane events.
        std::vector<double> events;
        std::vector<int> marks;
        const auto& planes = p_.outputPlanes;
        const int ne = static_cast<int>(lat_.elements.size());
        for (int i = 0; i < ne; ++i) {
            for (double pl : planes) {
                if (lat_.elemZStart[i] < pl && pl <= lat_.elemZStart[i] + lat_.elemLength[i]) {
                    if (events.empty() || pl > events.back()) {
                        events.push_back(pl);
                        marks.push_back(-1);
                    }
                }
            }
        }
        const double zE = lat_.elemZStart[ne - 1] + lat_.elemLength[ne - 1];
        if (events.empty() || zE != events.back()) {
            events.push_back(zE);
            marks.push_back(-2);
        }
        startClock_ = std::chrono::steady_clock::now();
        zStartRun_ = b_.z[n];
        double t = b_.tMin;
        tcode(t, events, marks);

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
        element_field(i, t, lat_.elements[k]);
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
        const int type = e.fieldType;
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
                tot[0] = b.ex[i] + 0.0; tot[1] = b.ey[i] + 0.0; tot[2] = b.ez[i] + 0.0;
                tot[3] = b.bx[i] + 0.0; tot[4] = b.by[i] + 0.0; tot[5] = b.bz[i] + 0.0;
            }
            auto& old = oldCoord_[i];
            old = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
            if (b.lost[i] == 1) continue;
            boris_push(b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i], tot, tot + 3, b.qOverM[i], dt_);
            b.rel[0][i] = b.x[i] - sync[0];
            b.rel[2][i] = b.y[i] - sync[2];
            b.rel[4][i] = b.z[i] - sync[4];
            b.rel[1][i] = b.ux[i];
            b.rel[3][i] = b.uy[i];
            b.rel[5][i] = b.uz[i];
            external_field(i, tEnd);
            if (b.uz[i] < 0.0 && lat_.spOutCount <= lat_.spOutCur && b.z[i] < 0.0) b.lost[i] = 1;
            const size_t m = static_cast<size_t>(b.nextPlane[i]);
            if (m + 1 <= events.size()) {
                const double pl = events[m];
                if (pl <= b.z[i] && pl <= lat_.sp_exit()) {
                    const double r = (pl - old[4]) / (b.z[i] - old[4]);
                    Rec& rec = planeRec_[i];
                    const double cur[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
                    for (int j = 0; j < 6; ++j) rec[j] = (cur[j] - old[j]) * r + old[j];
                    rec[6] = (r - 1.0) * dt_ + tEnd;
                    rec[7] = 1.0;
                    b.nextPlane[i] += 1;
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
        for (int i = 0; i < n; ++i) {
            if (b.z[i] < b.zMin) b.zMin = b.z[i];
            if (b.zMax < b.z[i]) b.zMax = b.z[i];
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
        char name[64];
        std::snprintf(name, sizeof name, "%f", z);
        if (!write_file(paths().output + "outData_" + name + ".dst", out, false, true)) say("output error");
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
            for (int j = 0; j < 6; ++j) out.append(reinterpret_cast<const char*>(&b.rel[j][i]), 8);
            const std::int32_t alive = b.lost[i] == 1 ? 0 : 1, id = i;
            out.append(reinterpret_cast<const char*>(&alive), 4);
            out.append(reinterpret_cast<const char*>(&id), 4);
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
    void tcode(double& t, const std::vector<double>& events, const std::vector<int>& marks) {
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
        const double zSeg = 0.0;
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
            pool.run(nBlocks, [&](int k) {
                trans(bounds[k], bounds[k + 1], zSeg, events, t, gp, sync, acc[k]);
            });
            pushTime_ += std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - tp0).count();
            if (n == 1) single_out(t);

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
            if (prev[14] == 2.0) {
                if (b.z[n] < zSeg) { prev[14] = 2; prev[15] = prev[16] = prev[17] = 0; }
                else { prev[14] = prev[15] = prev[16] = 0; prev[17] = b.z[n] - zSeg; }
            } else {
                prev[14] = prev[15] = prev[16] = 0;
                prev[17] = b.z[n] - oldCoord_[n][4];
            }
            if (b.nAlive == 0) throw EngineError("End of simulation, all particles lost.                     ");

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
                    dst_out(zev, planeRec_);
                    for (auto& r : planeRec_) r.fill(0.0);
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
            sync[0] = b.x[n]; sync[1] = b.ux[n]; sync[2] = b.y[n]; sync[3] = b.uy[n]; sync[4] = b.z[n]; sync[5] = b.uz[n];
            b.rotMode = 0;
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
        }
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
