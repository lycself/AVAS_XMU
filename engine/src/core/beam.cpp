#include "core/beam.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/physics.h"
#include "core/text.h"

namespace avas {

void Beam::resize(int nMacro) {
    n = nMacro;
    const size_t m = static_cast<size_t>(nMacro) + 1;
    for (auto* v : {&x, &ux, &y, &uy, &z, &uz, &ex, &ey, &ez, &bx, &by, &bz, &weight, &qOverM, &charge, &mass,
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
    const int maxT = omp_max_threads();
    for (int i = 0; i < 10; ++i) {
        if (p.cores[i] > maxT) p.cores[i] = maxT;
        if (p.maxThreads > 0 && p.cores[i] > p.maxThreads) p.cores[i] = p.maxThreads;
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
void init_single(Beam& b, Params& p, const Lattice& lat, double betaGamma0, double atomicMass) {
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

void init_generated(Beam& b, Params& p, const Lattice& lat) {
    b.frequency = p.frequency;
    const double mc2 = p.restMass;
    say("Starting to generate initial bunch");
    const double gamma0 = p.kineticEnergy / mc2 + 1.0;
    const double ig0 = 1.0 / gamma0;
    const double betaGamma0 = std::sqrt(1.0 - ig0 * ig0) * gamma0;
    const double atomicMass = mc2 * k::inv_proton_mc2;
    p.atomicMass = atomicMass;
    p.current = p.current * 0.001;
    b.current = p.current;
    if (p.current == 0.0 || p.particleNumber == 1) {
        p.current = b.current = 0.0;
        p.spaceCharge = 0;
        say("0 intensity, stop SCfield calculations");
    }
    if (p.probe == 1) throw EngineError("input.txt probe 1 is not implemented in the new engine yet");
    if (p.particleNumber != 1) {
        throw EngineError("Generating the initial distribution is not implemented in the new engine yet; "
                          "use readparticledistribution with a .dst file or particlenumber 1");
    }
    b.resize(1);
    say("The intensity is set to 0");
    default_cores(p, 1);
    p.cores.fill(1);
    init_single(b, p, lat, betaGamma0, atomicMass);
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
        say("Particle weight loaded.");
    }

    // inData.dst: the input copied (header bytes kept), with beam errors (none in M1).
    if (!write_file(paths().output + "inData.dst", raw.substr(0, 23 + rows.size() * 8 + 8), false, true))
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
        throw EngineError("Reading .edst distributions is not implemented in the new engine yet");
    } else {
        throw EngineError(kDistOpenError);
    }
}

}  // namespace avas
