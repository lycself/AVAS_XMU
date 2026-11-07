#include "core/synchronous.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "core/dllmath.h"
#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/physics.h"
#include "core/text.h"

namespace avas {

namespace {

const char* kReverse = "Synchronous particle reverse motion, program stopped";
const char* kNoExit = "Synchronised particles can't reach the exit plane, problems with the beamline";

double kinetic(double gbz, double mc2) { return (std::sqrt(std::pow(gbz, 2.0) + 1.0) - 1.0) * mc2; }

// The reference particle used by the phase determination (SynParticle of the original).
struct SynParticle {
    double coord[6] = {0, 0, 0, 0, 0, 0};  // x, ux, y, uy, z, uz
    double E[3] = {0, 0, 0};
    double B[3] = {0, 0, 0};
    std::vector<double> cavPhase, cavTime;
    int elemIndex = 0;
    bool fullPush = false;
    double qOverM = 0, restMass = 0, ezAmp = 0, frequency = 0;

    double& z() { return coord[4]; }
    double& uz() { return coord[5]; }
};

// SetUpSynParticle: initial gamma*beta, mass and frequency (spec 20 2.3).
void setup(SynParticle& s, const Params& p, double gbzIn) {
    say("Start initialising synchronised particles");
    double w = 0.0, mc2 = 938.272029, gbz = 0.0;
    bool haveW = false;
    if (p.distFile == "unknown" || p.secondaryBeam != 0) {
        std::string raw;
        if (!read_file(paths().input + "beam.txt", raw)) {
            say("File \"beam.txt\" not opened!");
            if (compat_dll()) compat_exit(1);
            throw EngineError("File \"beam.txt\" not opened!");
        }
        for (const std::string& line : split_lines(normalize_line_endings(raw))) {
            std::vector<std::string> wd = split_words(line);
            if (wd.size() < 2) continue;
            const std::string kw = to_lower(wd[0]);
            if (kw == "kneticenergy") { w = parse_double(wd[1]); haveW = true; }
            else if (kw == "particlerestmass") mc2 = parse_double(wd[1]);
            else if (kw == "frequency") s.frequency = parse_double(wd[1]);
        }
        if (!haveW) throw EngineError("beam.txt has no kneticenergy");
        const double g = w / mc2 + 1.0;
        gbz = std::sqrt(1.0 - 1.0 / (g * g)) * g;  // gbzIn is ignored on this branch
    } else {
        std::string raw;
        if (!read_file(paths().input + p.distFile, raw) || raw.size() < 31) {
            say("Cannot read initial distribution");
            if (compat_dll()) compat_exit(1);
            throw EngineError("Cannot read initial distribution");
        }
        std::int32_t n;
        double fMHz;
        std::memcpy(&n, raw.data() + 2, 4);
        std::memcpy(&fMHz, raw.data() + 14, 8);
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            double wi;
            std::memcpy(&wi, raw.data() + 23 + (static_cast<size_t>(i) * 6 + 5) * 8, 8);
            sum = sum + wi;
        }
        std::memcpy(&mc2, raw.data() + 23 + static_cast<size_t>(n) * 48, 8);
        s.frequency = fMHz * 1000000.0;
        const double g = (sum / static_cast<double>(n)) / mc2 + 1.0;
        gbz = gbzIn == -33.0 ? std::sqrt(1.0 - 1.0 / (g * g)) * g : gbzIn;
    }
    const double a = mc2 * k::inv_proton_mc2;
    for (double& c : s.coord) c = 0.0;
    s.coord[5] = gbz;
    s.restMass = mc2;
    s.ezAmp = 0.0;
    s.elemIndex = 0;
    s.qOverM = (p.charge / a) * k::e_over_mp;
    for (int i = 0; i < 3; ++i) s.E[i] = s.B[i] = 0.0;
}

class PhaseFinder {
public:
    PhaseFinder(Lattice& lat, Params& p, double gbzIn, bool single)
        : lat_(lat), p_(p), gbzIn_(gbzIn), single_(single), compat_(compat_dll()) {}

    void run() {
        if (!compat_) {
            // bends whose path (arc) differs from their lattice length L, e.g. written with L = 0
            for (const Element& e : lat_.elements)
                if (e.kind == ElemKind::Bend && e.arc != e.length) bendExtra_.push_back({e.zStart, e.arc - e.length});
        }
        if (p_.needSynPhase == 1) {
            say("Start scan phase....");
            scan_stage();
        } else {
            for (const Element& e : lat_.elements) {
                if (e.typeName == "field" && parse_double(e.words.at(5)) != 0.0)
                    lat_.fm.cavPhase.push_back(parse_double(e.words.at(6)));
            }
        }
        main_stage();
    }

private:
    Lattice& lat_;
    Params& p_;
    double gbzIn_;
    bool single_;
    bool compat_;
    std::vector<std::pair<double, double>> bendExtra_;  // {zStart, arc - L}
    size_t nextBend_ = 0;

    // The hard-edge transfer moves the beam along the arc of a bend but along L in z.  When the
    // two differ, the reference particle (which drifts through the bend here) gets the missing
    // time (arc - L)/v, so that the RF phases downstream agree with the tracking.  AVAS.dll does
    // not do this (and its tracking even uses half the arc when L < 0.85 arc).
    void bend_time(SynParticle& s, double& t) {
        while (nextBend_ < bendExtra_.size() && bendExtra_[nextBend_].first <= s.z()) {
            const double uz = s.uz();
            const double v = k::c * uz / std::sqrt(uz * uz + 1.0);
            t += bendExtra_[nextBend_].second / v;
            ++nextBend_;
        }
    }

    int nElem() const { return static_cast<int>(lat_.elements.size()); }
    double L() const { return lat_.total_length(); }
    double zStartOf(int i) const { return lat_.elemZStart[i]; }
    double zEndOf(int i) const { return lat_.elemZEnd[i]; }

    // SynParticle::GetField: locate the element and evaluate its field.
    void get_field(SynParticle& s, double t) {
        const double z = s.coord[4];
        if (0.0 <= z && z <= lat_.sp_exit() && z < L()) {
            int i = s.elemIndex;
            if (z < zStartOf(i)) {
                do { --i; } while (z < zStartOf(i));
            } else if (zEndOf(i) <= z) {
                do { ++i; } while (zEndOf(i) <= z);
            }
            s.elemIndex = i;
            element_field(s, t, lat_.elements[i]);
        } else {
            for (int k = 0; k < 3; ++k) s.E[k] = s.B[k] = 0.0;
        }
    }

    void element_field(SynParticle& s, double t, const Element& e) {
        const double r = std::sqrt(s.coord[0] * s.coord[0] + s.coord[2] * s.coord[2]);
        if (e.kind == ElemKind::Drift) {
            if (e.radius <= r) {
                say(kNoExit);
                if (compat_) compat_exit(1);
                throw EngineError(kNoExit);
            }
            for (int k = 0; k < 3; ++k) s.E[k] = s.B[k] = 0.0;
            return;
        }
        if (e.kind != ElemKind::Field) {  // hard-edge elements and steerers: no field here
            for (int k = 0; k < 3; ++k) s.E[k] = s.B[k] = 0.0;
            return;
        }
        const int type = e.fieldType;
        if (type == 4) {
            superpose_field(s, t, e);
            return;
        }
        if (type != 1 && type != 2 && type != 3) return;  // type 0 and others: nothing written
        if (e.radius <= r) {
            say(kNoExit);
            if (compat_) compat_exit(1);
            throw EngineError(kNoExit);
        }
        for (int k = 0; k < 3; ++k) s.E[k] = s.B[k] = 0.0;
        const FieldMap& fmap = lat_.fm.maps.at(e.mapIndex);
        const MapView<double> m = fmap.view();
        const double zl = s.coord[4] - e.zStart;
        if (!(m.xmin < s.coord[0] && s.coord[0] < m.xmax && m.ymin < s.coord[2] && s.coord[2] < m.ymax &&
              e.zStart < s.coord[4] && s.coord[4] < e.zStart + m.zlen)) {
            // AVAS.dll keeps the axial amplitude of the previous call here, so the first step of a
            // cavity that starts at the beginning of its map uses a stale value in the phase
            // integral; the corrected engine uses zero (no field outside the map).
            if (type == 1 && !compat_) s.ezAmp = 0.0;
            return;
        }
        if (type == 1) {
            double ez = 0.0;
            const double ke6 = e.ke * 1000000.0;
            trilinear(m, s.coord[0], s.coord[2], zl, [&](double w, const double* f) { ez = ez + w * f[2] * ke6; });
            const int c = e.cavIndex;
            const double phase = s.cavPhase.at(c) * k::deg_to_rad + (t - s.cavTime.at(c)) * e.frequency * k::two_pi;
            s.E[2] = dll_cos(phase) * ez;
            s.ezAmp = ez;
        } else if (type == 2) {
            trilinear(m, s.coord[0], s.coord[2], zl, [&](double w, const double* f) {
                for (int k = 0; k < 3; ++k) s.E[k] = s.E[k] + w * f[k] * (e.ke * 1000000.0);
            });
        } else {
            trilinear(m, s.coord[0], s.coord[2], zl, [&](double w, const double* f) {
                for (int k = 0; k < 3; ++k) s.B[k] = s.B[k] + w * f[k] * e.kb;
            });
        }
    }

    // Field::GetField(t, SynParticle&) type 4 (spec 30 2.5): sub-fields summed in the lab frame.
    void superpose_field(SynParticle& s, double t, const Element& e) {
        for (int k = 0; k < 3; ++k) s.E[k] = s.B[k] = 0.0;
        const double x = s.coord[0], y = s.coord[2], z = s.coord[4];
        for (size_t j = 0; j < e.subZStart.size(); ++j) {
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
            if (0.0 < zl && zl < e.subLength[j] && e.inSuperposeOut == 0 &&
                e.subRadius[j] <= std::sqrt(xl * xl + yl * yl)) {
                say(kNoExit);
                if (compat_) compat_exit(1);
                throw EngineError(kNoExit);
            }
            const int st = e.subType[j];
            if (st != 1 && st != 2 && st != 3) continue;
            const MapView<double> m = lat_.fm.maps.at(e.subMap[j]).view();
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
                double* D = st == 1 ? s.B : s.E;
                double* O = st == 1 ? s.E : s.B;
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
                const double phase = s.cavPhase.at(c) * k::deg_to_rad + (t - s.cavTime.at(c)) * e.frequency * k::two_pi;
                const double cE = dll_cos(phase);
                const double cB = dll_cos(phase - k::half_pi);
                const double a = cE * S[1] * c4 - cE * S[2] * s4;
                const double bb = s4 * cE * S[1] + cE * S[2] * c4;
                const double cz = bb * c5;
                const double d = S[0] * cE * c5 + s5 * bb;
                s.E[0] = (c3 * d - a * s3) + s.E[0];
                s.E[1] = (d * s3 + a * c3) + s.E[1];
                s.E[2] = (cz - s5 * S[0] * cE) + s.E[2];
                const double a2 = cB * SB[1] * c4 - cB * SB[2] * s4;
                const double b2 = cB * SB[1] * s4 + cB * SB[2] * c4;
                const double cz2 = b2 * c5;
                const double d2 = SB[0] * cB * c5 + s5 * b2;
                s.B[0] = (c3 * d2 - a2 * s3) + s.B[0];
                s.B[1] = (s3 * d2 + a2 * c3) + s.B[1];
                s.B[2] = (cz2 - s5 * SB[0] * cB) + s.B[2];
                s.ezAmp = S[2];
            }
        }
    }

    void push(SynParticle& s, double dt) { sync_push(s.coord, s.E, s.B, s.qOverM, dt, s.fullPush); }

    // Linear interpolation of z, uz (and t) back to a plane crossed in the last step.
    static void interpolate(SynParticle& s, const double prev[2], double zPlane, double eps, double* t, double dt) {
        const double r = (zPlane - prev[0]) / (s.coord[4] - prev[0]);
        s.coord[4] = (s.coord[4] - prev[0]) * r + prev[0] + eps;
        s.coord[5] = (s.coord[5] - prev[1]) * r + prev[1];
        if (t) *t = r * dt + (*t - dt);
    }

    static double normalise_deg(double v) {
        while (true) {
            while (v < -180.0) v += 360.0;
            if (v <= 180.0) break;
            v -= 360.0;
        }
        return v;
    }

    // ---- scan stage --------------------------------------------------------------------
    void scan_stage() {
        SynParticle s;
        setup(s, p_, gbzIn_);
        write_file(paths().output + "scanPhaseData.txt", "", false);
        write_file(paths().output + "Phase.txt", "", false);
        AppendFile spd, ph;
        spd.open(paths().output + "scanPhaseData.txt");
        ph.open(paths().output + "Phase.txt");
        const double dt = scan_dt(s);
        const double nMax = static_cast<double>(p_.runPeriod) * p_.stepPerCycle;
        double t = 0.0;
        double prev[6] = {s.z(), s.uz(), 0, 0, 0, 0};
        int cnt = 0;
        if (0.0 < nMax / p_.scanTStep) {
            nextBend_ = 0;
            while (true) {
                bend_time(s, t);
                const double z = s.z();
                if (0.0 <= z && z <= lat_.sp_exit() && z < L()) {
                    int i = s.elemIndex;
                    if (z < zStartOf(i)) {
                        do { --i; } while (z < zStartOf(i));
                    } else if (zEndOf(i) <= z) {
                        do { ++i; } while (zEndOf(i) <= z);
                    }
                    s.elemIndex = i;
                }
                const Element& e = lat_.elements[s.elemIndex];
                if (e.typeName != "field" && e.typeName != "rfq") {
                    get_field(s, t);
                    prev[0] = s.z();
                    prev[1] = s.uz();
                    push(s, dt);
                    t += dt;
                } else {
                    const double f = parse_double(e.words.at(5));
                    if (f == 0.0) {
                        static_step(s, e, prev, t, dt);
                    } else if (parse_double(e.words.at(3)) <= 0.5) {
                        scan_cavity(s, e, prev, t, dt, spd, ph);
                    } else {
                        register_cavity(s, e, prev, t, dt);
                    }
                }
                if (L() < s.z()) break;
                cnt += 1;
                if (nMax / p_.scanTStep <= static_cast<double>(cnt)) break;
            }
        }
        spd.close();
        ph.close();
        lat_.spOutCur = 0;
        for (size_t i = 0; i < s.cavPhase.size(); ++i) {
            lat_.fm.cavPhase.push_back(s.cavPhase[i]);
            lat_.fm.cavTime.push_back(s.cavTime[i]);
        }
    }

    double scan_dt(const SynParticle& s) const {
        if (compat_) return (p_.scanTStep / s.frequency) / p_.stepPerCycle;
        // Corrected engine: the scan uses the main-pass step so that the phase it finds gives
        // exactly the requested synchronous phase in the main pass (defect B-3).
        return (main_factor() / s.frequency) / p_.stepPerCycle;
    }
    double main_factor() const { return single_ ? 0.1 : 0.01; }

    void static_step(SynParticle& s, const Element& e, double prev[6], double& t, double dt) {
        get_field(s, t);
        if (e.inSuperposeOut != 0) s.fullPush = true;
        prev[0] = s.z(); prev[1] = s.uz();
        prev[2] = s.coord[0]; prev[3] = s.coord[1]; prev[4] = s.coord[2]; prev[5] = s.coord[3];
        push(s, dt);
        const int j = lat_.spOutCur;
        if (j < lat_.spOutCount && lat_.elements[s.elemIndex].inSuperposeOut != 0) {
            s.fullPush = true;
            const auto& row = lat_.spOut[j];
            const double zs = lat_.elements[s.elemIndex].zStart;
            const double cur[6] = {s.coord[0], s.coord[1], s.coord[2], s.coord[3], s.coord[4], s.coord[5]};
            const double old[6] = {prev[2], prev[3], prev[4], prev[5], prev[0], prev[1]};
            double lc[6], lp[6];
            sp_local(row, zs, cur, lc);
            sp_local(row, zs, old, lp);
            if (lc[4] > 0.0) {  // crossed the exit plane of the block
                const double r = lp[4] / (lp[4] - lc[4]);
                for (int q : {0, 1, 2, 3, 5}) s.coord[q] = (lc[q] - lp[q]) * r + lp[q];
                if (!compat_) {  // interpolate |p| separately (the field turns the momentum)
                    const double pa = std::sqrt(lp[1] * lp[1] + lp[3] * lp[3] + lp[5] * lp[5]);
                    const double pb = std::sqrt(lc[1] * lc[1] + lc[3] * lc[3] + lc[5] * lc[5]);
                    const double pr = std::sqrt(s.coord[1] * s.coord[1] + s.coord[3] * s.coord[3] + s.coord[5] * s.coord[5]);
                    if (pr > 0.0) {
                        const double f = ((pb - pa) * r + pa) / pr;
                        s.coord[1] *= f;
                        s.coord[3] *= f;
                        s.coord[5] *= f;
                    }
                }
                s.coord[4] = row[6];
                t = (r * dt + t) - dt;
                if (row[6] == L()) {
                    throw EngineError("A superposeout block that ends the lattice is not implemented in the new engine yet");
                }
                lat_.spOutCur = j + 1;
                s.fullPush = false;
            }
        }
        t += dt;
    }

    // Coordinates {x, ux, y, uy, z, uz} in the frame of a superposeout exit (spec 20 2.6.6).
    static void sp_local(const std::array<double, 7>& row, double zStart, const double* c, double* o) {
        // SetSynPhase calls the CRT sin and cos here (not the static sincos)
        const double s3 = dll_sin(row[3]), c3 = dll_cos(row[3]);
        const double s5 = dll_sin(row[5]), c5 = dll_cos(row[5]);
        const double s4 = dll_sin(row[4]), c4 = dll_cos(row[4]);
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

    // Track the particle through one cavity (or other field element) until it leaves at zEnd.
    void track_through(SynParticle& s, int elem, double& t, double dt) {
        double prev[2];
        while (true) {
            while (true) {
                get_field(s, t);
                prev[0] = s.z();
                prev[1] = s.uz();
                push(s, dt);
                t += dt;
                if (zStartOf(elem) < s.z()) break;
                if (s.z() < zStartOf(elem)) throw EngineError(kReverse);
            }
            if (!(s.z() <= zEndOf(elem))) break;
        }
        interpolate(s, prev, zEndOf(elem), 1e-13, &t, dt);
    }

    // One tracking of the scan through cavity e with entrance phase phi0; returns phi_s (deg).
    double scan_once(SynParticle& s, const Element& e, int c, double zIn, double gIn, double omega, double dt,
                     double eps) {
        s.coord[5] = gIn;
        s.coord[4] = zIn;
        s.coord[3] = s.coord[2] = s.coord[1] = s.coord[0] = 0.0;
        double tau = s.cavTime[c];
        double sc = 0.0, ss = 0.0;
        double prev[2];
        get_field(s, tau);
        prev[0] = s.z(); prev[1] = s.uz();
        push(s, dt);
        const int ei = e.index;
        while (s.z() <= zEndOf(ei)) {
            const double tp = compat_ ? tau : tau + 0.0;
            const double cs = dll_cos(s.cavPhase[c] * k::deg_to_rad + (tp - s.cavTime[c]) * omega) * s.ezAmp;
            sc = sc + (s.z() - prev[0]) * cs;
            const double sn = dll_sin(s.cavPhase[c] * k::deg_to_rad + (tp - s.cavTime[c]) * omega) * s.ezAmp;
            ss = ss + (s.z() - prev[0]) * sn;
            tau = tau + dt;
            get_field(s, tau);
            prev[0] = s.z(); prev[1] = s.uz();
            push(s, dt);
        }
        const double r = (zEndOf(ei) - prev[0]) / (s.z() - prev[0]);
        s.coord[4] = (s.z() - prev[0]) * r + prev[0] + eps;
        s.coord[5] = (s.uz() - prev[1]) * r + prev[1];
        const double cs = dll_cos(s.cavPhase[c] * k::deg_to_rad + (tau - s.cavTime[c]) * omega) * s.ezAmp;
        const double sn = dll_sin(s.cavPhase[c] * k::deg_to_rad + (tau - s.cavTime[c]) * omega) * s.ezAmp;
        const double dz = s.z() - prev[0];
        return std::atan((dz * sn * r + ss) / (dz * cs * r + sc)) * k::rad_to_deg;
    }

    void enter_element(SynParticle& s, const Element& e, const double prev[6], double& t, double dt) {
        if (prev[0] != s.z()) {
            const double r = (zStartOf(e.index) - prev[0]) / (s.z() - prev[0]);
            s.coord[4] = (s.z() - prev[0]) * r + prev[0] + 1e-15;
            s.coord[5] = (s.uz() - prev[1]) * r + prev[1];
            t = r * dt + (t - dt);
        }
    }

    void scan_cavity(SynParticle& s, const Element& e, double prev[6], double& t, double dt, AppendFile& spd,
                     AppendFile& ph) {
        const int c = parse_int(e.words.at(14));
        const double f = parse_double(e.words.at(5));
        enter_element(s, e, prev, t, dt);
        const double zIn = s.z(), gIn = s.uz();
        const double target = parse_double(e.words.at(6));
        s.cavTime.push_back(t);
        s.cavPhase.push_back(-180.0);
        const double mc2 = s.restMass;
        const double wIn = kinetic(gIn, mc2);
        ph.write(Text().i(c + 1).s("  ").fixed(t, 13).s("  ").fixed(zIn, 13).s("  ").fixed(wIn, 13).nl().str());

        const double omega = f * k::two_pi;
        std::vector<double> phi0, phis, gout;
        for (double a = -180.0; a < 181.0; a += 18.0) {
            s.cavPhase[c] = a;
            const double ps = scan_once(s, e, c, zIn, gIn, omega, dt, 1e-15);
            phi0.push_back(s.cavPhase[c]);
            phis.push_back(ps);
            gout.push_back(s.uz());
            spd.write(Text().fixed(a, 10).s(" ").fixed(ps, 10).s(" ").fixed(kinetic(gIn, mc2), 10).s(" ")
                          .fixed(kinetic(s.uz(), mc2), 10).nl().str());
        }
        // two candidates at least 90 degrees apart
        double b1 = 1e10, b2 = 1e10;
        int i1 = -1, i2 = -1;
        const int last = static_cast<int>(phis.size()) - 1;
        for (int i = 0; i <= last; ++i) {
            const double d = std::fabs(phis[i] - target);
            if (!(d < b2)) continue;
            if (b1 <= d) {
                if (std::fabs(phi0[i] - phi0[i1]) >= 90.0 && std::fabs(std::fabs(phi0[i] - phi0[i1]) - 360.0) >= 0.5) {
                    b2 = d;
                    i2 = i;
                }
            } else {
                if (i1 != -1 && std::fabs(phi0[i] - phi0[i1]) >= 90.0 &&
                    std::fabs(std::fabs(phi0[i] - phi0[i1]) - 360.0) >= 0.5) {
                    b2 = b1;
                    i2 = i1;
                }
                b1 = d;
                i1 = i;
            }
        }
        auto fix_end = [&](int& i) {
            if (i == 0 && target < phis[0]) i = last;
            else if (i == last && phis[last] < target) i = 0;
        };
        fix_end(i1);
        fix_end(i2);
        if (i1 < 0 || i2 < 0) throw EngineError("Scan phase error, unexpected situation, check code");
        auto G = [&](int i) -> double {
            if (i < 0 || i > last) throw EngineError("Scan phase error, unexpected situation, check code");
            return gout[i];
        };
        int lo = -1, hi = -1;
        bool decreasing = false;
        if (0.0 <= target) {
            const int cc = (G(i1) <= gIn) ? i2 : i1;
            if (target <= phis[cc]) { lo = cc - 1; hi = cc; }
            else { lo = cc; hi = cc + 1; }
        } else {
            bool haveI1 = false;
            if (phis[i1] < target && G(i1 + 1) - G(i1) > 0.0) { lo = i1; hi = i1 + 1; haveI1 = true; }
            else if (target < phis[i1] && G(i1) - G(i1 - 1) > 0.0) { lo = i1 - 1; hi = i1; haveI1 = true; }
            if (!haveI1) {
                if (phis[i2] < target && G(i2 + 1) - G(i2) > 0.0) { lo = i2; hi = i2 + 1; }
                else if (target < phis[i2] && G(i2) - G(i2 - 1) > 0.0) { lo = i2 - 1; hi = i2; }
                else throw EngineError("Scan phase error, unexpected situation, check code");
            } else if (G(i1) < gIn) {
                decreasing = true;
                if (gIn < G(i2)) {
                    if (phis[i2] < target && G(i2 + 1) - G(i2) > 0.0) { lo = i2; hi = i2 + 1; decreasing = false; }
                    else if (target < phis[i2] && G(i2) - G(i2 - 1) > 0.0) { lo = i2 - 1; hi = i2; decreasing = false; }
                }
            }
        }
        if (lo < 0 || hi > last) throw EngineError("Scan phase error, unexpected situation, check code");
        double a0 = phi0[lo], a1 = phi0[hi], s0 = phis[lo], s1 = phis[hi];
        if (a0 == 0.0 && a1 == 0.0) throw EngineError("Scan phase error, unexpected situation, check code");
        if (decreasing) say("Note that the energy of the particle decreases after it passes through the cavity");

        double p0 = 0.0, p1 = 0.0;
        if (s0 == target) {
            p0 = a0; p1 = s0;
        } else if (s1 == target) {
            p0 = a1; p1 = s1;
        } else {
            if (std::fabs(s0 - target) <= 90.0) {
                if (std::fabs(s1 - target) > 90.0) s1 = a1 + 180.0;  // sic: phase, not phi_s (original)
            } else {
                s0 = s0 - 180.0;
            }
            for (int it = 0; it < 6; ++it) {
                p0 = ((a1 - a0) * (target - s0)) / (s1 - s0) + a0;
                s.cavPhase[c] = p0;
                p1 = scan_once(s, e, c, zIn, gIn, omega, dt, 1e-13);
                const std::string row = Text().fixed(p0, 10).s(" ").fixed(p1, 10).s(" ").fixed(kinetic(gIn, mc2), 10).s(" ")
                                            .fixed(kinetic(s.uz(), mc2), 10).nl().str();
                if (std::fabs(p1 - target) <= 90.0) {
                    if (p1 < target) { a0 = p0; s0 = p1; }
                    else if (p1 <= target) { spd.write(row); break; }
                    else { a1 = p0; s1 = p1; }
                } else {
                    if (p1 < target) { a1 = p0; s1 = p1 + 180.0; }
                    if (target < p1) { a0 = p0; s0 = p1 - 180.0; }
                }
                spd.write(row);
            }
        }
        spd.write("3333 3333 3333 3333\n");
        say(Text().s("The ").i(c + 1).s(" field, phi_0:").g(p0).s("    phi_s:").g(target).s("  Frequency is ").g(f).str());
        if (std::fabs(p1 - target) > 1.0 && std::fabs(p1 + 90.0) > 1.0 && std::fabs(p1 - 90.0) > 1.0) {
            throw EngineError("Abnormal settings of the initial energy or the phase parameters of the lattice cause the "
                              "scan phase to fail to find the corresponding RF phase");
        }
        s.cavPhase[c] = p0;
        s.coord[5] = gIn;
        s.coord[4] = zIn;
        s.coord[3] = s.coord[2] = s.coord[1] = s.coord[0] = 0.0;
        say(Text().s("Entry  time ").g(t).s("  Entering energy ").g(kinetic(s.uz(), mc2)).str());
        track_through(s, e.index, t, dt);
        say(Text().s("Export time ").g(t).s("    Export energy ").g(kinetic(s.uz(), mc2)).str());
        s.elemIndex += 1;
        ph.write(Text().i(c + 1).s("  ").fixed(t, 13).s("  ").fixed(s.z(), 13).s("  ").fixed(kinetic(s.uz(), mc2), 13)
                     .nl().str());
    }

    // V3 = 1 / 2 cavity in the scan stage: register entry time and phase, then pass through.
    void register_cavity(SynParticle& s, const Element& e, double prev[6], double& t, double dt) {
        const double f = parse_double(e.words.at(5));
        parse_int(e.words.at(14));
        enter_element(s, e, prev, t, dt);
        s.cavTime.push_back(t);
        const double v3 = parse_double(e.words.at(3));
        if (v3 <= 1.5) {
            s.cavPhase.push_back(parse_double(e.words.at(6)));
        } else {
            const double ft = f * t;
            // AVAS.dll subtracts here and adds in the main pass (defect B-1)
            const double phi = compat_ ? parse_double(e.words.at(6)) - ft * 360.0 : parse_double(e.words.at(6)) + ft * 360.0;
            s.cavPhase.push_back(normalise_deg(phi));
        }
        s.coord[3] = s.coord[2] = s.coord[1] = s.coord[0] = 0.0;
        track_through(s, e.index, t, dt);
        s.elemIndex += 1;
        if (s.elemIndex > nElem() - 1) s.elemIndex = nElem() - 1;
    }

    // ---- main stage --------------------------------------------------------------------
    void main_stage() {
        SynParticle s;
        setup(s, p_, gbzIn_);
        double t = 0.0;
        const double dt = (main_factor() / s.frequency) / p_.stepPerCycle;
        write_file(paths().output + "synData.txt", "", false);
        AppendFile sd;
        sd.open(paths().output + "synData.txt");
        Text row;
        auto head = [&](int i, int prec) {
            const Element& e = lat_.elements[i];
            row.i(i).s(" ").s(e.typeName).s(" ").g(e.length, prec).s(" ").g(s.z(), prec).s(" ").g(t, prec).s(" ")
                .g(s.uz(), prec);
        };
        head(s.elemIndex, single_ ? 17 : 6);
        double prev[6] = {s.z(), s.uz(), 0, 0, 0, 0};
        s.cavPhase = lat_.fm.cavPhase;
        s.cavTime.clear();
        bool done = false;
        nextBend_ = 0;
        while (!done) {
            bend_time(s, t);
            const double z = s.z();
            if (0.0 <= z && z <= lat_.sp_exit() && z < L()) {
                int i = s.elemIndex;
                if (z < zStartOf(i)) {
                    do { --i; } while (z < zStartOf(i));
                    s.elemIndex = i;
                } else if (zEndOf(i) <= z) {
                    do { ++i; } while (zEndOf(i) <= z);
                    s.elemIndex = i;
                    if (prev[0] != z) {
                        const double r = (zStartOf(i) - prev[0]) / (z - prev[0]);
                        s.coord[4] = (z - prev[0]) * r + prev[0] + 1e-15;
                        s.coord[5] = (s.uz() - prev[1]) * r + prev[1];
                        t = r * dt + (t - dt);
                    }
                    row.s(" invalid invalid").nl();
                    head(i, 17);
                }
            }
            if (L() < s.z()) {
                if (prev[0] != s.z()) {
                    const double r = (L() - prev[0]) / (s.z() - prev[0]);
                    s.coord[4] = (s.z() - prev[0]) * r + prev[0];
                    s.coord[5] = (s.uz() - prev[1]) * r + prev[1];
                    t = dt * r + (t - dt);
                }
                row.s(" invalid invalid").nl();
                const Element& e = lat_.elements[s.elemIndex];
                row.i(s.elemIndex).s(" exitPlane ").g(e.length, 17).s(" ").g(s.z(), 17).s(" ").g(t, 17).s(" ")
                    .g(s.uz(), 17).s(" invalid invalid").nl();
                break;
            }
            const Element& e = lat_.elements[s.elemIndex];
            if (e.typeName == "field" && parse_double(e.words.at(5)) == 0.0) {
                static_step(s, e, prev, t, dt);
            } else if (e.typeName == "field") {
                done = main_cavity(s, e, prev, t, dt, row);
            } else {
                get_field(s, t);
                prev[0] = s.z();
                prev[1] = s.uz();
                push(s, dt);
                t += dt;
            }
            if (row.str().size() > 1 << 16) {
                sd.write(row.str());
                row.clear();
            }
        }
        sd.write(row.str());
        sd.close();
        lat_.spOutCur = 0;
        say(Text().s("Exit bunch energy: ").g(kinetic(s.uz(), s.restMass)).s("                       ").str());
        auto& fm = lat_.fm;
        for (size_t i = 0; i < s.cavPhase.size(); ++i) {
            if (i < fm.cavPhase.size()) fm.cavPhase[i] = s.cavPhase[i];
            else fm.cavPhase.push_back(s.cavPhase[i]);
            const double tc = i < s.cavTime.size() ? s.cavTime[i] : 0.0;
            if (i < fm.cavTime.size()) fm.cavTime[i] = tc;
            else fm.cavTime.push_back(tc);
        }
    }

    // Returns true when the cavity was the last element (synData finished).
    bool main_cavity(SynParticle& s, const Element& e, double prevMain[6], double& t, double dt, Text& row) {
        const int c = parse_int(e.words.at(14));
        const double f = parse_double(e.words.at(5));
        if (s.z() != prevMain[0]) {
            const double dzv = s.z() - prevMain[0];
            const double r = (zStartOf(e.index) - prevMain[0]) / dzv;
            s.coord[4] = r * dzv + prevMain[0] + 1e-15;
            s.coord[5] = (s.uz() - prevMain[1]) * r + prevMain[1];
            t = (t - dt) + dt * r;
        }
        if (parse_double(e.words.at(3)) > 1.5) {
            const double ft = t * f;
            s.cavPhase.at(c) = normalise_deg(parse_double(e.words.at(6)) + ft * 360.0);
        }
        if (s.cavTime.size() < s.cavPhase.size()) s.cavTime.push_back(t);
        s.coord[3] = s.coord[2] = s.coord[1] = s.coord[0] = 0.0;
        double sc = 0.0, ss = 0.0;
        double prev[2];
        const int ei = e.index;
        while (true) {
            get_field(s, t);
            prev[0] = s.z();
            prev[1] = s.uz();
            push(s, dt);
            // AVAS.dll takes the RF phase at the end of the step while the field is taken at its
            // start (defect B-3); the corrected engine uses the start of the step, as the scan does.
            const double tStart = t;
            t = dt + t;
            if (s.z() <= zStartOf(ei)) {
                if (s.z() < zStartOf(ei)) throw EngineError(kReverse);
                continue;
            }
            if (zEndOf(ei) < s.z()) break;
            const double tp = compat_ ? t : tStart;
            double sn, cs;
            dll_sincos((tp - s.cavTime.at(c)) * (f * k::two_pi) + s.cavPhase[c] * k::deg_to_rad, sn, cs);
            sc = sc + cs * s.ezAmp * (s.z() - prev[0]);
            ss = ss + sn * s.ezAmp * (s.z() - prev[0]);
        }
        const double r = (zEndOf(ei) - prev[0]) / (s.z() - prev[0]);
        s.coord[4] = (s.z() - prev[0]) * r + prev[0] + 1e-13;
        s.coord[5] = (s.uz() - prev[1]) * r + prev[1];
        const double tp = compat_ ? t : t - dt;
        const double cs = dll_cos(s.cavPhase[c] * k::deg_to_rad + (tp - s.cavTime[c]) * (f * k::two_pi)) * s.ezAmp;
        const double dzLast = s.z() - prev[0];
        const double sn = dll_sin((tp - s.cavTime[c]) * f * k::two_pi + s.cavPhase[c] * k::deg_to_rad) * s.ezAmp;
        t = dt * r + (t - dt);
        s.elemIndex += 1;
        const double phis = std::atan((sn * dzLast * r + ss) / (cs * dzLast * r + sc));
        if (s.elemIndex <= nElem() - 1) {
            row.s(" ").g(phis * k::rad_to_deg, 17).s(" ").g(s.cavPhase[c], 17).nl();
            const Element& n = lat_.elements[s.elemIndex];
            row.i(s.elemIndex).s(" ").s(n.typeName).s(" ").g(n.length, 17).s(" ").g(s.z(), 17).s(" ").g(t, 17).s(" ")
                .g(s.uz(), 17);
            return false;
        }
        // the cavity is the last element
        row.s(" ").g(phis * k::rad_to_deg, 17).s(" ").g(s.cavPhase[c], 17).nl();
        if (prevMain[0] != s.z()) {
            const double rr = (L() - prevMain[0]) / (s.z() - prevMain[0]);
            s.coord[4] = (s.z() - prevMain[0]) * rr + prevMain[0];
            s.coord[5] = (s.uz() - prevMain[1]) * rr + prevMain[1];
            t = dt * rr + (t - dt);
        }
        row.i(s.elemIndex).s(" exitPlane ").g(lat_.elements[s.elemIndex - 1].length, 17).s(" ").g(s.z(), 17).s(" ")
            .g(t, 17).s(" ").g(s.uz(), 17).s(" invalid invalid").nl();
        return true;
    }
};

}  // namespace

void set_synchronous_phase(Lattice& lat, Params& p, double gbzIn, bool singleParticleMode) {
    PhaseFinder(lat, p, gbzIn, singleParticleMode).run();
}

}  // namespace avas
