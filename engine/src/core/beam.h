// Macro particles (structure of arrays) and their initialisation (InitParticle, spec 50).
#pragma once
#include <array>
#include <string>
#include <vector>

#include "core/lattice.h"
#include "core/params.h"

namespace avas {

struct Beam {
    int n = 0;  // number of macro particles; index n is the synchronous particle

    // per particle, size n + 1
    std::vector<double> x, ux, y, uy, z, uz;     // m and gamma*beta
    std::vector<double> ex, ey, ez, bx, by, bz;  // external field at the particle (V/m, T)
    std::vector<double> scx, scy, scz;           // space-charge field (lab frame, magnetic part folded in)
    std::vector<double> rel[6];                  // coordinates relative to the synchronous particle
    std::vector<int> elemIndex, lost, nextPlane;
    std::vector<double> weight, qOverM, charge, mass, restMass;
    std::vector<double> spOutZ0, spOutZ1;
    std::vector<int> boundaryMode;

    double frequency = 0.0;     // Hz
    double current = 0.0;       // A
    double realPerMacro = 1.0;
    double totalWeight = 0.0;
    double tMin = 0.0, bunchTime = 0.0;
    double zMin = 0.0, zMax = 0.0;
    double gamma = 1.0;
    double betaGammaLambda = 0.0;
    std::array<double, 6> centroid{};
    std::array<double, 3> rms{};
    int nAlive = 0;
    int rotMode = 0;
    double scComp = 1.0;    // spacechargecomp: AVAS.dll value V (1 = none) / corrected factor 1 - V
    int fromDstFile = 0;    // particles read from .edst (per-particle charge, mass, weight)
    int useWeightFile = 0;  // .dstb weights (indweight 1) or .edst
    double rotAngle = 0.0;
    double time = 0.0;

    void resize(int nMacro);
};

// InitParticle: reads beam.txt and builds the initial macro particles (single particle or .dst
// file in M1).  Writes SingleParticle.txt / inData.dst like the original.
void init_particles(Beam& beam, Params& p, const Lattice& lat);

}  // namespace avas
