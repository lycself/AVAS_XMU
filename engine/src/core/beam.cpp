#include "core/beam.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>

#include "core/distribution.h"
#include "core/errors.h"
#include "core/spacecharge.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/physics.h"
#include "core/text.h"

namespace avas {

void Beam::resize(int nMacro) {
    n = nMacro;
    const size_t m = static_cast<size_t>(nMacro) + 1;
    for (auto* v : {&x, &ux, &y, &uy, &z, &uz, &ex, &ey, &ez, &bx, &by, &bz, &scx, &scy, &scz, &weight, &qOverM, &charge, &mass,
                    &restMass, &spOutZ0, &spOutZ1})
        v->assign(m, 0.0);
    for (auto& r : rel) r.assign(m, 0.0);
    for (auto* v : {&elemIndex, &lost, &nextPlane, &boundaryMode}) v->assign(m, 0);
}

namespace {

const char* kDistOpenError =
    "The initial bunch distribution file cannot be opened, check particleDistributionPath";

// Default thread table of the original (spec 40 7 / spec 50 2.5.2); only cores[0] (the number of
// particle blocks) matters in M1.
void default_cores(Params& p, long long n) {
    if (p.multithreading == 1 && p.cores[0] == -1) {
        static const int table[8][10] = {{64, 54, 20, 24, 1, 48, 96, 12, 48, 12}, {72, 68, 24, 56, 22, 48, 96, 20, 48, 24},
                                         {96, 96, 32, 96, 26, 48, 96, 24, 48, 36}, {96, 96, 52, 96, 74, 48, 96, 48, 48, 96},
                                         {96, 96, 62, 96, 86, 48, 96, 48, 48, 96}, {96, 96, 82, 96, 92, 48, 96, 48, 48, 96},
                                         {96, 96, 92, 96, 96, 48, 96, 48, 48, 96}, {96, 96, 96, 96, 96, 48, 96, 48, 48, 96}};
        static const double limits[7] = {3e4, 8e4, 3e5, 8e5, 3e6, 8e6, 3e7};
        int row = 7;
        for (int i = 0; i < 7; ++i)
            if (static_cast<double>(n) <= limits[i]) { row = i; break; }
        for (int i = 0; i < 10; ++i) p.cores[i] = table[row][i];
    }
    // AVAS.dll clips the block counts to the OpenMP thread count, so its summation order (and
    // the last bits of every statistic) depends on the machine.  The corrected engine keeps the
    // table values as fixed partitions; the number of threads only sets how fast they run.
    const int maxT = compat_dll() ? omp_max_threads() : 1 << 30;
    for (int i = 0; i < 10; ++i) {
        if (p.cores[i] > maxT) p.cores[i] = maxT;
        if (compat_dll() && p.maxThreads > 0 && p.cores[i] > p.maxThreads) p.cores[i] = p.maxThreads;
        p.scPar[10 + i] = static_cast<double>(p.cores[i]);
    }
    p.scPar[9] = static_cast<double>(p.multithreading);
}

void finish_sp_out(Beam& b, const Lattice& lat) {
    const double z1 = lat.sp_exit();
    for (int i = 0; i <= b.n; ++i) {
        b.spOutZ0[i] = 0.0;
        b.spOutZ1[i] = z1;
    }
}

// particlenumber 1: one macro particle displaced by displacepos / displacedpos.
void init_single(Beam& b, Params& p, const Lattice& lat, double betaGamma0, double atomicMass,
                 const std::array<double, 26>* berr) {
    for (int i = 0; i <= 1; ++i) {
        b.x[i] = b.y[i] = b.ux[i] = b.uy[i] = b.z[i] = 0.0;
        b.uz[i] = betaGamma0;
        b.elemIndex[i] = b.lost[i] = b.nextPlane[i] = 0;
        b.charge[i] = p.charge * k::e_charge;
        b.mass[i] = atomicMass * k::m_proton;
        b.qOverM[i] = (p.charge / atomicMass) * k::e_over_mp;
        b.restMass[i] = p.restMass;
        b.boundaryMode[i] = p.boundary;
    }
    // AVAS.dll never initialises the weight of particle 0 (its DataSet.txt then shows
    // garbage); the corrected engine gives it weight 1.
    b.weight[0] = compat_dll() ? 0.0 : 1.0;
    b.weight[1] = 1.0;
    b.x[0] = p.displacePos[0];
    b.y[0] = p.displacePos[1];
    b.z[0] = p.displacePos[2];
    b.ux[0] = p.displacedPos[0] * 0.01 * b.uz[0];
    b.uy[0] = p.displacedPos[1] * 0.01 * b.uz[0];
    b.uz[0] = (p.displacedPos[2] * 0.01 + 1.0) * b.uz[0];
    if (berr) {
        // beam errors on the single particle: dx, dy, dx', dy' (slopes), dphi as a shift in z at
        // the particle velocity, dE on the kinetic energy (direction kept)
        const auto& e = *berr;
        const double mc2 = p.restMass;
        b.x[0] = e[1] * 0.001 + b.x[0];
        b.y[0] = e[2] * 0.001 + b.y[0];
        b.ux[0] = e[4] * 0.001 * b.uz[0] + b.ux[0];
        b.uy[0] = e[5] * 0.001 * b.uz[0] + b.uy[0];
        const double p2 = b.ux[0] * b.ux[0] + b.uy[0] * b.uy[0] + b.uz[0] * b.uz[0];
        const double g = std::sqrt(p2 + 1.0);
        const double vz = b.uz[0] / g * k::c;
        b.z[0] = b.z[0] - (e[3] * k::deg_to_rad) / (k::two_pi * p.frequency) * vz;
        const double g1 = (g - 1.0) + e[6] / mc2 + 1.0;
        const double scale = std::sqrt(g1 * g1 - 1.0) / std::sqrt(p2);
        b.ux[0] *= scale;
        b.uy[0] *= scale;
        b.uz[0] *= scale;
    }
    b.zMin = b.zMax = b.z[0];
    b.totalWeight = 1.0;
    b.tMin = b.bunchTime = 0.0;

    Text t;
    t.s("dx:").g(b.x[0] * 1000.0).s("mm  dy:").g(b.y[0] * 1000.0).s("mm  dz:").g(b.z[0] * 1000.0).s("mm").nl();
    const double ig = 1.0 / b.uz[0];
    t.s("dpx:").g(ig * b.ux[0] * 1000.0).s("mrad  dpy:").g(ig * b.uy[0] * 1000.0).s("mrad  dpz:").s("--").nl();
    t.s("Energy:").g(p.kineticEnergy).s("MeV").nl();
    write_file(paths().output + "SingleParticle.txt", t.str(), false);
    say("Synchron particle addition completed");
    say("Bunch initialisation complete");
    say("  ");
    // The original leaves spOutZ0/spOutZ1 unset here; the corrected engine sets them.
    if (!compat_dll()) finish_sp_out(b, lat);
}

// "Read back" of a .dst particle table (spec 50 2.4.6): the file gives each particle where it
// crosses z = 0 (at different times); the engine moves them to the time tMin of the first one.
// rows: x(cm) x' y(cm) y' phi W per particle; mc2(i) the rest energy of particle i.  Uses
// b.frequency and b.tMin.
template <class Mass>
void readback_particles(Beam& b, const std::vector<double>& rows, int n, Mass mc2of) {
    for (int i = 0; i < n; ++i) {
        b.x[i] = rows[6 * i + 0];
        b.ux[i] = rows[6 * i + 1];
        b.y[i] = rows[6 * i + 2];
        b.uy[i] = rows[6 * i + 3];
        b.z[i] = rows[6 * i + 4];
        b.uz[i] = rows[6 * i + 5];
    }
    if (compat_dll()) {
        for (int i = 0; i < n; ++i) {
            const double g = b.uz[i] / mc2of(i) + 1.0;
            const double ig = 1.0 / g;
            const double beta = std::sqrt(1.0 - ig * ig);
            const double v = beta * k::c;
            const double t = (k::inv_two_pi / b.frequency) * b.z[i] - b.tMin;
            b.z[i] = -(v * t);
            const double pz = beta * g;  // AVAS.dll: |p| = beta*gamma is put into p_z alone
            b.uz[i] = pz;
            b.x[i] = b.x[i] * 0.01 - t * b.ux[i] * v;
            b.y[i] = b.y[i] * 0.01 - t * b.uy[i] * v;
            b.ux[i] = pz * b.ux[i];
            b.uy[i] = pz * b.uy[i];
        }
    } else {
        // libAVAS.so formula: the longitudinal velocity is split off from the total one, so the
        // particle energy equals the file energy.
        const double w2pi = b.frequency * k::two_pi;
        for (int i = 0; i < n; ++i) {
            const double xp = b.ux[i], yp = b.uy[i];
            const double g = b.uz[i] / mc2of(i) + 1.0;
            const double beta = std::sqrt(1.0 - (1.0 / g) / g);
            const double vz = std::sqrt((((beta * k::c) * beta) * k::c) / ((xp * xp + yp * yp) + 1.0));
            const double t = b.z[i] / w2pi - b.tMin;
            b.z[i] = (-t) * vz;
            const double pz = (vz / k::c) * g;
            b.uz[i] = pz;
            b.ux[i] = xp * pz;
            b.uy[i] = pz * yp;
            b.x[i] = b.x[i] / 100.0 - (xp * t) * vz;
            b.y[i] = b.y[i] / 100.0 - (t * yp) * vz;
        }
    }
}

// bunchTime and tMin from the phases phi (min / max start at 0)
void bunch_time(Beam& b, double phiMin, double phiMax) {
    if (compat_dll()) {
        const double kf = k::inv_two_pi / b.frequency;
        b.bunchTime = (phiMax - phiMin) * kf;
        b.tMin = kf * phiMin;
    } else {
        const double w2pi = b.frequency * k::two_pi;
        b.bunchTime = (phiMax - phiMin) / w2pi;
        b.tMin = phiMin / w2pi;
    }
}

void readback(Beam& b, const std::vector<double>& rows, int n, double mc2) {
    double phiMax = 0.0, phiMin = 0.0;
    for (int i = 0; i < n; ++i) {
        const double phi = rows[6 * i + 4];
        if (phi < phiMin) phiMin = phi;
        if (phiMax < phi) phiMax = phi;
    }
    bunch_time(b, phiMin, phiMax);
    readback_particles(b, rows, n, [mc2](int) { return mc2; });
}

std::string dst_bytes(std::int32_t n, double ibMa, double fMHz, const std::vector<double>& rows, double mc2) {
    std::string out;
    out.push_back(static_cast<char>(0x7D));
    out.push_back(static_cast<char>(0x64));
    out.append(reinterpret_cast<const char*>(&n), 4);
    out.append(reinterpret_cast<const char*>(&ibMa), 8);
    out.append(reinterpret_cast<const char*>(&fMHz), 8);
    out.push_back(static_cast<char>(0x7D));
    out.append(reinterpret_cast<const char*>(rows.data()), rows.size() * 8);
    out.append(reinterpret_cast<const char*>(&mc2), 8);
    return out;
}

// The random seed of the corrected engine: randomseed, or the time when it is 0.  The value is
// printed and written next to inData.dst so that every run can be repeated.
std::uint64_t default_seed(const Params& p) {
    std::int64_t seed = p.fixedSeed != 0 ? p.fixedSeed : static_cast<std::int64_t>(p.randomSeed);
    if (seed == 0) seed = static_cast<std::int64_t>(std::time(nullptr));
    say("Random seed: " + std::to_string(seed));
    write_file(paths().output + "inData.seed.txt", "randomseed " + std::to_string(seed) + "\n", false);
    return static_cast<std::uint64_t>(seed);
}

// Beam errors of the current error sample (spec 70 5), columns 1-13: dx dy (mm), dphi (deg),
// dx' dy' (mrad), dE (MeV), demittance x y z (%), mismatch x y z (%), dIb (mA).  AVAS.dll reads only
// the dynamic beam row; a static beam error alone leaves that row out of the sample and the
// original dereferences a null pointer.  The corrected engine adds the static and the dynamic row.
// Returns false without beam errors; `marked` is the "applied" flag of the row (AVAS.dll changes
// the current and the Twiss parameters only when it is set, the shifts always).
bool beam_errors(const Lattice& lat, std::array<double, 26>& e, bool& marked) {
    e.fill(0.0);
    marked = false;
    const ErrorSample* es = lat.sample;
    if (!es || es->flags[0] + es->flags[1] == 0) return false;
    if (compat_dll()) {
        if (es->dyn.empty()) compat_abort("access violation reading 0x0000000000000000 (err_beam_stat)");
        const std::vector<double>& r = es->dyn[0];
        for (size_t c = 0; c < r.size() && c < e.size(); ++c) e[c] = r[c];
        marked = !r.empty() && r[0] == 1.0;
        return true;
    }
    for (const auto* rows : {&es->stat, &es->dyn}) {
        if (rows->empty() || (*rows)[0].empty() || (*rows)[0][0] != 1.0) continue;
        const std::vector<double>& r = (*rows)[0];
        for (size_t c = 1; c < r.size() && c < e.size(); ++c) e[c] += r[c];
        marked = true;
    }
    return marked;
}

// Shifts of one .dst row (x, y cm; x', y' rad; phi rad; W MeV).
void shift_dst_row(double* r, const std::array<double, 26>& e) {
    r[0] = e[1] * 0.1 + r[0];
    r[1] = e[4] * 0.001 + r[1];
    r[2] = e[2] * 0.1 + r[2];
    r[3] = e[5] * 0.001 + r[3];
    r[4] = e[3] * 0.017453292519943295 + r[4];
    r[5] = r[5] + e[6];
}

// Generation from the Twiss parameters (spec 50 2.4.1-2.4.5).
void init_generated(Beam& b, Params& p, const Lattice& lat) {
    const bool compat = compat_dll();
    b.frequency = p.frequency;
    const double mc2 = p.restMass;
    say("Starting to generate initial bunch");
    const double gamma0 = p.kineticEnergy / mc2 + 1.0;
    const double ig0 = 1.0 / gamma0;
    const double beta0 = std::sqrt(1.0 - ig0 * ig0);
    const double betaGamma0 = beta0 * gamma0;
    const double atomicMass = mc2 * k::inv_proton_mc2;
    p.atomicMass = atomicMass;
    p.current = p.current * 0.001;
    std::array<double, 26> berr{};
    bool bmark = false;
    const bool hasBeamErr = beam_errors(lat, berr, bmark);
    if (hasBeamErr && bmark) {
        p.current = berr[13] * 0.001 + p.current;
        auto& tw = p.twiss;
        const bool zSet = compat || (tw[6] != kTwissUnset && tw[6] != kTwissDcMark);
        tw[0] = (berr[7] * 0.01 + 1.0) * tw[0];
        tw[3] = (berr[8] * 0.01 + 1.0) * tw[3];
        if (zSet) tw[6] = (berr[9] * 0.01 + 1.0) * tw[6];
        const int idx[3][2] = {{1, 2}, {4, 5}, {7, 8}};
        for (int q = 0; q < 3; ++q) {
            if (q == 2 && !zSet) continue;
            const double f = berr[10 + q] * 0.01 + 1.0;
            for (int j : idx[q]) tw[j] = (f * f) * tw[j];  // pow(f, 2) of AVAS.dll is exact
        }
    }
    b.current = p.current;
    const int N = p.particleNumber;
    if (p.current == 0.0 || N == 1) {
        p.current = b.current = 0.0;
        p.spaceCharge = 0;
        say("0 intensity, stop SCfield calculations");
    }
    if (N < 1) throw EngineError("particlenumber must be a positive number");
    const bool probe = p.probe == 1;
    const int nb = probe ? 125 : N;  // beam.n
    if (probe) p.spaceCharge = 0;
    b.resize(nb);
    double rpm = 1.0;
    if (b.current == 0.0) {
        say("The intensity is set to 0");
    } else {
        rpm = std::fabs(((b.current / (static_cast<double>(N) * b.frequency)) * 6.241509479607718e+18) / p.charge);
        b.realPerMacro = rpm;
        say(Text().s("The number of particles represented by a single macro particle: ").g(rpm).str());
    }
    space_charge_defaults(p, N);
    default_cores(p, N);
    if (N == 1) {
        p.cores.fill(1);
        // AVAS.dll leaves the single particle without beam errors; the corrected engine shifts it
        init_single(b, p, lat, betaGamma0, atomicMass, (hasBeamErr && !compat) ? &berr : nullptr);
        return;
    }

    // Twiss -> geometric emittances (m rad)
    auto& tw = p.twiss;
    const double ibg = 1.0 / betaGamma0;
    tw[0] = tw[0] * 1e-06 * ibg;
    tw[3] = tw[3] * 1e-06 * ibg;
    if (tw[6] == kTwissDcMark) tw[6] = kTwissUnset;
    else if (tw[6] != kTwissUnset) tw[6] = (tw[6] * 1e-06) / (betaGamma0 * gamma0 * gamma0);
    const bool dc = tw[6] == kTwissUnset;

    // AVAS.dll reads past the generated rows when probe 1 has fewer than 125 particles; the
    // corrected engine generates at least 125 (compat: undefined behaviour, same as corrected).
    const int nRows = probe ? std::max(N, 125) : N;
    std::vector<Row4> rows(static_cast<size_t>(nRows), Row4{0, 0, 0, 0});
    b.totalWeight = static_cast<double>(N);
    MsvcRand msrng;
    Philox4x32 ph{};
    const int seed = static_cast<int>(p.randomSeed);
    const Twiss2 twT{tw[0], tw[1], tw[2], tw[3], tw[4], tw[5]};
    if (compat) {
        msrng.srand(static_cast<std::uint32_t>(std::time(nullptr)));
        dll_generate(p.distribution[0], twT, rows, seed, msrng);
    } else {
        const std::uint64_t s64 = default_seed(p);
        ph.key[0] = static_cast<std::uint32_t>(s64);
        ph.key[1] = static_cast<std::uint32_t>(s64 >> 32);
        default_generate(p.distribution[0], twT, rows, ph, kStreamTransverse);
    }
    for (int i = 0; i < nb; ++i) {
        b.x[i] = rows[i][0];
        b.ux[i] = rows[i][1];  // x' for now
        b.y[i] = rows[i][2];
        b.uy[i] = rows[i][3];
        b.elemIndex[i] = b.lost[i] = 0;
        b.charge[i] = rpm * p.charge * k::e_charge;
        b.mass[i] = rpm * atomicMass * k::m_proton;
        b.qOverM[i] = (p.charge / atomicMass) * k::e_over_mp;
        b.restMass[i] = mc2;
        b.nextPlane[i] = 0;
        b.boundaryMode[i] = p.boundary;
        b.weight[i] = 1.0;
    }
    // probe 1: rms sizes of the generated transverse distribution
    double sigX = 0.0, sigY = 0.0;
    if (probe) {
        const double n = static_cast<double>(compat ? N : nRows);
        const int cnt = compat ? N : nRows;
        double sx = 0.0, sy = 0.0;
        for (int i = 0; i < cnt; ++i) {
            sx = sx + rows[i][0];
            sy = sy + rows[i][2];
        }
        // AVAS.dll divides the sum by n twice and uses the x mean for y as well
        const double mx = compat ? sx / (n * n) : sx / n;
        const double my = compat ? mx : sy / n;
        double ax = 0.0, ay = 0.0;
        for (int i = 0; i < cnt; ++i) {
            const double dx = rows[i][0] - mx;
            ax = ax + dx * dx;
            const double dy = rows[i][2] - my;
            ay = ay + dy * dy;
        }
        const double in = 1.0 / n;
        sigX = std::sqrt(ax * in);
        sigY = std::sqrt(ay * in);
    }
    const int s = nb;
    b.elemIndex[s] = b.lost[s] = 0;
    b.charge[s] = p.charge * k::e_charge;
    b.mass[s] = atomicMass * k::m_proton;
    b.qOverM[s] = (p.charge / atomicMass) * k::e_over_mp;
    b.restMass[s] = mc2;
    b.nextPlane[s] = 0;
    b.boundaryMode[s] = p.boundary;
    b.weight[s] = 1.0;

    if (!dc) {
        const Twiss2 twZ{tw[6], tw[7], tw[8], tw[6], tw[7], tw[8]};
        if (compat) dll_generate(p.distribution[1], twZ, rows, seed, msrng);
        else default_generate(p.distribution[1], twZ, rows, ph, kStreamLongitudinal);
        for (int i = 0; i < nb; ++i) {
            b.z[i] = rows[i][0];
            b.uz[i] = rows[i][1];  // delta for now
        }
    }
    // .dst units: x, y (cm), x', y' (rad), phi (rad), W (MeV)
    for (int i = 0; i < nb; ++i) {
        if (dc) {
            b.x[i] = b.x[i] * 100.0;
            b.y[i] = b.y[i] * 100.0;
            b.z[i] = k::pi - static_cast<double>(i) * (k::two_pi / static_cast<double>(nb));
            double u;
            if (compat) {
                u = static_cast<double>(msrng.rand()) * 3.051850947599719e-05;
            } else {
                double uu[2];
                ph.uniform2(static_cast<std::uint32_t>(i), kStreamDcEnergy, 0, uu);
                u = uu[0];
            }
            b.uz[i] = (u - 0.5) * p.kineticEnergySpread + p.kineticEnergy;
        } else if (compat) {
            // AVAS.dll: delta on p_z, gamma without the transverse momentum, phase from the
            // total velocity
            const double pz = (b.uz[i] + 1.0) * betaGamma0;
            b.uz[i] = pz;
            const double g = std::sqrt(pz * pz + 1.0);
            const double ig = 1.0 / g;
            b.x[i] = b.x[i] * 100.0;
            b.y[i] = b.y[i] * 100.0;
            b.uz[i] = (g - 1.0) * mc2;
            b.z[i] = (-2.0958450219516816e-08 / std::sqrt(1.0 - ig * ig)) * b.z[i] * b.frequency;
        } else {
            // delta = dp/p of the total momentum; the particles are given at one time and are
            // written where they cross z = 0 (drifted along their slope), at their crossing time
            const double xp = b.ux[i], yp = b.uy[i];
            const double ptot = (b.uz[i] + 1.0) * betaGamma0;
            const double pz = ptot / std::sqrt(1.0 + xp * xp + yp * yp);
            const double g = std::sqrt(ptot * ptot + 1.0);
            const double vz = pz / g * k::c;
            const double z = b.z[i];
            b.x[i] = (b.x[i] - xp * z) * 100.0;
            b.y[i] = (b.y[i] - yp * z) * 100.0;
            b.uz[i] = (g - 1.0) * mc2;
            b.z[i] = -(z / vz) * (k::two_pi * b.frequency);
        }
    }
    const double ibMa = p.current * 1000.0;
    const double fMHz = b.frequency * 1e-06;
    std::vector<double> table(static_cast<size_t>(nb) * 6);
    for (int i = 0; i < nb; ++i) {
        const double v[6] = {b.x[i], b.ux[i], b.y[i], b.uy[i], b.z[i], b.uz[i]};
        for (int j = 0; j < 6; ++j) table[6 * i + j] = v[j];
        if (hasBeamErr) shift_dst_row(&table[6 * i], berr);  // beam error shifts (spec 70 5)
    }
    if (!write_file(paths().output + "inData.dst", dst_bytes(nb, ibMa, fMHz, table, mc2), false, true))
        throw EngineError("Insufficient permissions to write file: inData.dst");
    p.particleNumber = nb;
    readback(b, table, nb, mc2);
    for (int i = 0; i < nb; ++i) {
        b.ex[i] = b.ey[i] = b.ez[i] = b.bx[i] = b.by[i] = b.bz[i] = 0.0;
        for (auto& r : b.rel) r[i] = 0.0;
    }
    // synchronous particle: mean energy of the particles
    double sumW = 0.0;
    for (int i = 0; i < nb; ++i) sumW = sumW + table[6 * i + 5];
    const double wMean = sumW / static_cast<double>(nb);
    const double gS = wMean / mc2 + 1.0;
    double bS;
    if (compat) {
        const double igS = 1.0 / gS;
        bS = std::sqrt(1.0 - igS * igS);
        b.z[s] = bS * b.tMin * k::c;
    } else {
        bS = std::sqrt(1.0 - (1.0 / gS) / gS);
        b.z[s] = (bS * k::c) * b.tMin;
    }
    b.x[s] = b.ux[s] = b.y[s] = b.uy[s] = 0.0;
    b.uz[s] = bS * gS;

    if (probe) {
        // sigma_z of the longitudinal rows (DC: of the transverse x)
        const int cnt = compat ? N : nRows;
        double sz = 0.0;
        for (int i = 0; i < cnt; ++i) sz = sz + rows[i][0];
        const double mz = sz / static_cast<double>(cnt);
        double az = 0.0;
        for (int i = 0; i < cnt; ++i) {
            const double d = rows[i][0] - mz;
            az = az + d * d;
        }
        const double sigZ = std::sqrt(az / static_cast<double>(cnt));
        // the corrected engine centres the grid on the synchronous particle
        const double z0 = compat ? 0.0 : b.z[s];
        int idx = 0;
        for (int a = -2; a <= 2; ++a) {
            for (int bb = -2; bb <= 2; ++bb) {
                const double zs[5] = {sigZ * -2.0, -sigZ, 0.0, sigZ, sigZ + sigZ};
                for (int c = 0; c < 5; ++c, ++idx) {
                    b.x[idx] = static_cast<double>(a) * sigX;
                    b.y[idx] = static_cast<double>(bb) * sigY;
                    b.z[idx] = compat ? zs[c] : zs[c] + z0;
                    b.ux[idx] = b.uy[idx] = 0.0;
                    b.uz[idx] = betaGamma0;
                    b.charge[idx] = 0.0;
                }
            }
        }
        std::vector<double> grid(static_cast<size_t>(nb) * 6);
        for (int i = 0; i < nb; ++i) {
            const double g = std::sqrt(b.uz[i] * b.uz[i] + 1.0);
            const double ig = 1.0 / g;
            const double beta = std::sqrt(1.0 - ig * ig);
            const double vz = std::sqrt((beta / (b.ux[i] * b.ux[i] + b.uy[i] * b.uy[i] + 1.0)) * beta * k::c * k::c);
            grid[6 * i + 0] = b.x[i] * 100.0;
            grid[6 * i + 1] = 0.0;
            grid[6 * i + 2] = b.y[i] * 100.0;
            grid[6 * i + 3] = 0.0;
            grid[6 * i + 4] = (b.z[i] - z0) / vz * -6.283185307179586 * b.frequency;
            grid[6 * i + 5] = (g - 1.0) * mc2;
        }
        if (!write_file(paths().output + "inData.dst", dst_bytes(nb, ibMa, fMHz, grid, mc2), false, true))
            throw EngineError("Insufficient permissions to write file: inData.dst");
    }
    say("Synchron particle addition completed");
    say("Bunch initialisation complete");
    say("  ");
    finish_sp_out(b, lat);
}

void init_from_dst(Beam& b, Params& p, const Lattice& lat) {
    say("Start reading initial beam distribution...");
    std::string raw;
    if (!read_file(paths().input + p.distFile, raw) || raw.size() < 23) throw EngineError(kDistOpenError);
    std::int32_t n32;
    double ib, fMHz;
    std::memcpy(&n32, raw.data() + 2, 4);
    std::memcpy(&ib, raw.data() + 6, 8);
    std::memcpy(&fMHz, raw.data() + 14, 8);
    const int n = n32;
    if (n < 1 || raw.size() < 23 + 48 * static_cast<size_t>(n) + 8) throw EngineError(kDistOpenError);
    say("total number of particles " + std::to_string(n));
    say(Text().s("current intensity ").g(ib).s("(mA)").str());
    say(Text().s("beam frequency ").g(fMHz).s("(MHz)").str());
    space_charge_defaults(p, n);
    default_cores(p, n);
    std::vector<double> rows(static_cast<size_t>(n) * 6);
    std::memcpy(rows.data(), raw.data() + 23, rows.size() * 8);
    double mc2;
    std::memcpy(&mc2, raw.data() + 23 + rows.size() * 8, 8);
    say(Text().s("Particle rest mass: ").g(mc2).s(" (MeV)").str());

    b.resize(n);
    b.totalWeight = static_cast<double>(n);
    for (int i = 0; i < n; ++i) b.weight[i] = 1.0;
    if (p.indWeight == 1) {
        std::string w;
        if (!read_file(paths().input + p.distFile + "b", w) || w.size() < 4) {
            throw EngineError("The initial bunch Weight file cannot be opened, check particleDistributionPath");
        }
        std::int32_t nw;
        std::memcpy(&nw, w.data(), 4);
        p.particleNumber = nw;
        if (nw != n || w.size() < 4 + 8 * static_cast<size_t>(n))
            throw EngineError("The initial bunch Weight file particle number error");
        std::memcpy(b.weight.data(), w.data() + 4, 8 * static_cast<size_t>(n));
        b.totalWeight = 0.0;
        for (int i = 0; i < n; ++i) b.totalWeight += b.weight[i];
        b.useWeightFile = 1;
        say("Particle weight loaded.");
    }

    // beam errors: current and shifts (spec 70 5)
    std::array<double, 26> berr{};
    bool bmark = false;
    if (beam_errors(lat, berr, bmark)) {
        ib = ib + berr[13];
        for (int i = 0; i < n; ++i) shift_dst_row(&rows[6 * i], berr);
    }
    // inData.dst: the input (header bytes kept) with the beam errors applied.
    std::string dst = raw.substr(0, 23);
    std::memcpy(&dst[6], &ib, 8);
    dst.append(reinterpret_cast<const char*>(rows.data()), rows.size() * 8);
    dst.append(raw, 23 + rows.size() * 8, 8);
    if (!write_file(paths().output + "inData.dst", dst, false, true))
        throw EngineError("Insufficient permissions to write file: inData.dst");

    const double atomicMass = mc2 * k::inv_proton_mc2;
    p.atomicMass = atomicMass;
    say(Text().s("atomic mass ").g(atomicMass).s(" electric charge of particles ").g(p.charge).str());
    double sumW = 0.0;
    for (int i = 0; i < n; ++i) sumW = sumW + rows[6 * i + 5] * b.weight[i];
    p.frequency = b.frequency = fMHz * 1000000.0;
    p.current = b.current = ib * 0.001;
    if (ib == 0.0) {
        p.spaceCharge = 0;
        say("0 current, ignoring space charge effects");
    }
    const double wMean = sumW / b.totalWeight;
    p.kineticEnergy = wMean;
    say(Text().s("Average beam energy: ").g(wMean).s(" (MeV)").str());
    const double gammaS = p.kineticEnergy / mc2 + 1.0;
    const double igS = 1.0 / gammaS;
    const double betaS = std::sqrt(1.0 - igS * igS);
    double rpm = 1.0;
    if (b.current != 0.0) {
        rpm = std::fabs(((b.current / (b.frequency * b.totalWeight)) * 6.241509479607718e+18) / p.charge);
        b.realPerMacro = rpm;
        say(Text().s("The number of macroparticle representative particles is ").g(rpm).str());
    }
    double phiMax = 0.0, phiMin = 0.0;
    for (int i = 0; i < n; ++i) {
        b.x[i] = rows[6 * i + 0];
        b.ux[i] = rows[6 * i + 1];
        b.y[i] = rows[6 * i + 2];
        b.uy[i] = rows[6 * i + 3];
        b.z[i] = rows[6 * i + 4];
        b.uz[i] = rows[6 * i + 5];
        b.elemIndex[i] = b.lost[i] = 0;
        b.charge[i] = rpm * b.weight[i] * p.charge * k::e_charge;
        b.mass[i] = rpm * b.weight[i] * atomicMass * k::m_proton;
        b.qOverM[i] = (p.charge / atomicMass) * k::e_over_mp;
        b.restMass[i] = mc2;
        b.nextPlane[i] = 0;
        b.boundaryMode[i] = p.boundary;
        const double phi = b.z[i];
        if (phi < phiMin) phiMin = phi;
        if (phiMax < phi) phiMax = phi;
    }
    const int s = n;
    b.elemIndex[s] = b.lost[s] = 0;
    b.charge[s] = p.charge * k::e_charge;
    b.mass[s] = atomicMass * k::m_proton;
    b.qOverM[s] = (p.charge / atomicMass) * k::e_over_mp;
    b.restMass[s] = mc2;
    b.nextPlane[s] = 0;
    b.boundaryMode[s] = p.boundary;
    b.weight[s] = 1.0;

    // Bring the particles from "crossing z = 0 at different times" to one time (tMin).
    if (compat_dll()) {
        const double kf = k::inv_two_pi / b.frequency;
        b.bunchTime = (phiMax - phiMin) * kf;
        b.tMin = kf * phiMin;
        for (int i = 0; i < n; ++i) {
            const double g = b.uz[i] / mc2 + 1.0;
            const double ig = 1.0 / g;
            const double beta = std::sqrt(1.0 - ig * ig);
            const double v = beta * k::c;
            const double t = (k::inv_two_pi / b.frequency) * b.z[i] - b.tMin;
            b.z[i] = -(v * t);
            const double pz = beta * g;  // AVAS.dll: |p| = beta*gamma is put into p_z alone
            b.uz[i] = pz;
            b.x[i] = b.x[i] * 0.01 - t * b.ux[i] * v;
            b.y[i] = b.y[i] * 0.01 - t * b.uy[i] * v;
            b.ux[i] = pz * b.ux[i];
            b.uy[i] = pz * b.uy[i];
        }
    } else {
        // libAVAS.so formula: the longitudinal velocity is split off from the total one, so the
        // particle energy equals the file energy.
        const double w2pi = b.frequency * k::two_pi;
        b.bunchTime = (phiMax - phiMin) / w2pi;
        b.tMin = phiMin / w2pi;
        for (int i = 0; i < n; ++i) {
            const double xp = b.ux[i], yp = b.uy[i];
            const double g = b.uz[i] / mc2 + 1.0;
            const double beta = std::sqrt(1.0 - (1.0 / g) / g);
            const double vz = std::sqrt((((beta * k::c) * beta) * k::c) / ((xp * xp + yp * yp) + 1.0));
            const double t = b.z[i] / w2pi - b.tMin;
            b.z[i] = (-t) * vz;
            const double pz = (vz / k::c) * g;
            b.uz[i] = pz;
            b.ux[i] = xp * pz;
            b.uy[i] = pz * yp;
            b.x[i] = b.x[i] / 100.0 - (xp * t) * vz;
            b.y[i] = b.y[i] / 100.0 - (t * yp) * vz;
        }
    }
    b.x[s] = b.ux[s] = b.y[s] = b.uy[s] = 0.0;
    b.z[s] = betaS * b.tMin * k::c;
    b.uz[s] = betaS * gammaS;
    say("Synchronised particles have been added");
    say("Beam initialisation complete");
    say("  ");
    finish_sp_out(b, lat);
}

// .edst: extended .dst with per-particle charge state, rest mass and weight; N particle records
// and one synchronous record (spec 50 2.4.8).
void init_from_edst(Beam& b, Params& p, const Lattice& lat) {
    say("Start reading initial beam distribution...");
    std::string raw;
    if (!read_file(paths().input + p.distFile, raw) || raw.size() < 23) throw EngineError(kDistOpenError);
    std::int32_t n32;
    double ib, fMHz;
    std::memcpy(&n32, raw.data() + 2, 4);
    std::memcpy(&ib, raw.data() + 6, 8);
    std::memcpy(&fMHz, raw.data() + 14, 8);
    const int n = n32;
    const size_t nrec = static_cast<size_t>(n) + 1;
    if (n < 1 || raw.size() < 23 + 72 * nrec + 8) throw EngineError(kDistOpenError);
    say("total number of particles " + std::to_string(n));
    say(Text().s("current intensity ").g(ib).s("(mA)").str());
    say(Text().s("beam frequency ").g(fMHz).s("(MHz)").str());
    space_charge_defaults(p, n);
    default_cores(p, n);
    std::vector<double> rec(nrec * 9);
    std::memcpy(rec.data(), raw.data() + 23, rec.size() * 8);
    double mc2;
    std::memcpy(&mc2, raw.data() + 23 + rec.size() * 8, 8);
    say(Text().s("Particle rest mass: ").g(mc2).s(" (MeV)").str());
    if (!compat_dll()) {
        // AVAS.dll ignores beam errors for .edst input; the corrected engine applies them to the
        // particle records (not to the synchronous record)
        std::array<double, 26> berr{};
        bool bmark = false;
        if (beam_errors(lat, berr, bmark)) {
            ib = ib + berr[13];
            for (int i = 0; i < n; ++i) shift_dst_row(&rec[9 * static_cast<size_t>(i)], berr);
        }
    }
    b.resize(n);
    b.fromDstFile = 1;
    b.useWeightFile = 1;
    double tw = 0.0;
    for (size_t i = 0; i < nrec; ++i) {
        b.weight[i] = rec[9 * i + 8];
        tw = tw + b.weight[i];
    }
    b.totalWeight = tw - b.weight[n];
    p.frequency = b.frequency = fMHz * 1000000.0;
    p.current = b.current = ib * 0.001;
    if (ib == 0.0) {
        p.spaceCharge = 0;
        say("0 current, ignoring space charge effects");
    }
    const double wS = rec[9 * n + 5];
    p.kineticEnergy = wS;
    say(Text().s("Average beam energy: ").g(wS).s(" (MeV)").str());
    double phiMax = 0.0, phiMin = 0.0;
    for (size_t i = 0; i < nrec; ++i) {
        const double* r = &rec[9 * i];
        const double w = b.weight[i];
        b.x[i] = r[0];
        b.ux[i] = r[1];
        b.y[i] = r[2];
        b.uy[i] = r[3];
        b.z[i] = r[4];
        b.uz[i] = r[5];
        b.elemIndex[i] = b.lost[i] = b.nextPlane[i] = 0;
        b.charge[i] = r[6] * w * k::e_charge;
        b.mass[i] = w * r[7] * k::inv_proton_mc2 * k::m_proton;
        b.qOverM[i] = b.charge[i] / b.mass[i];
        b.restMass[i] = r[7];
        b.boundaryMode[i] = p.boundary;
        if (r[4] < phiMin) phiMin = r[4];
        if (phiMax < r[4]) phiMax = r[4];
    }
    std::vector<double> rows(static_cast<size_t>(n) * 6);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < 6; ++j) rows[6 * i + j] = rec[9 * i + j];
    bunch_time(b, phiMin, phiMax);  // the synchronous record is included in min / max
    readback_particles(b, rows, n, [&b](int i) { return b.restMass[i]; });
    const int s = n;
    const double gS = wS / mc2 + 1.0;
    const double bS = compat_dll() ? std::sqrt(1.0 - (1.0 / gS) * (1.0 / gS)) : std::sqrt(1.0 - (1.0 / gS) / gS);
    b.x[s] = b.ux[s] = b.y[s] = b.uy[s] = 0.0;
    b.z[s] = bS * b.tMin * k::c;
    b.uz[s] = bS * gS;
    p.charge = rec[9 * n + 6];
    // AVAS.dll stores the rest energy (MeV) as the atomic mass here; corrected: mc2/938.271875
    p.atomicMass = compat_dll() ? rec[9 * n + 7] : rec[9 * n + 7] * k::inv_proton_mc2;
    say("Synchronised particles have been added");
    say("Beam initialisation complete");
    say("  ");
    finish_sp_out(b, lat);
}

}  // namespace

void init_particles(Beam& b, Params& p, const Lattice& lat) {
    read_beam(p);
    if (p.secondaryBeam != 0) throw EngineError("secondarybeam 1 is not implemented in the new engine yet");
    if (p.distFile == "unknown") {
        init_generated(b, p, lat);
        return;
    }
    bool hasDot = false;
    const std::string ext = extension(p.distFile, hasDot);
    if (!hasDot) throw EngineError(kDistOpenError);
    if (ext == "dst") {
        init_from_dst(b, p, lat);
    } else if (ext == "edst") {
        init_from_edst(b, p, lat);
    } else {
        throw EngineError(kDistOpenError);
    }
}

}  // namespace avas
