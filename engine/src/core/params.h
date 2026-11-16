// Run parameters from input.txt and beam.txt (the original's Initial object, spec 10 2.3-2.5).
#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <vector>

namespace avas {

constexpr double kTwissUnset = -2027.041;   // "not given" sentinel of the original
constexpr double kTwissDcMark = -2027.0415; // beamtype dc mark

struct Params {
    // input.txt
    std::string method = "MP";
    int secondaryBeam = 0;
    int boundary = 0;
    int probe = 0;
    int lpBoundary = 0;
    int lpPeriodic = 0;  // corrected engine: lpboundary V1 1, periodic longitudinal space-charge box (M10 D7)
    int indWeight = 0;
    int indCharge = 0;
    int spaceCharge = 1;
    int singleParticleMode = 0;
    int scSolverRecord = 0;
    int dumpPeriodicity = 0;
    int statOutputInterval = 1;  // statoutputinterval (libPIC.so, corrected engine only): DataSet row every N steps
    int runPeriod = 100000;
    int multithreading = 1;
    int maxThreads = -1;
    double scanTStep = 0.03;
    double stepPerCycle = 50.0;
    double seStep = 0.01;
    double randomSeed = 0.0;
    std::int64_t fixedSeed = 0;  // corrected engine: seed resolved once for all error samples
    std::array<double, 5> longLimits{};
    std::array<double, 6> pcHistogram{};
    std::array<int, 3> numOfGrid{{-1, -1, -1}};
    std::array<int, 10> cores{{-1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    std::array<double, 20> scPar{{1, -1, -1, -1, -1, -1, -1, -1, 0.09, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    std::vector<int> track;

    // beam.txt
    std::string distFile = "unknown";
    double charge = -1.0;
    double atomicMass = -1.0;
    std::array<std::string, 2> distribution{{"GS", "GS"}};
    int particleNumber = -1;
    std::array<double, 9> twiss{{kTwissUnset, kTwissUnset, kTwissUnset, kTwissUnset, kTwissUnset, kTwissUnset,
                                 kTwissUnset, kTwissUnset, kTwissUnset}};
    std::array<double, 3> displacePos{};   // m (beam.txt value in mm x 0.001)
    std::array<double, 3> displacedPos{};  // beam.txt value x 0.001
    // corrected engine (M10 D2): initpos dx dy dz (mm) and initmom x' y' (mrad) delta (1e-3), both x 0.001
    std::array<double, 3> initPos{}, initMom{};
    bool hasDisplacePos = false, hasDisplacedPos = false, hasInitPos = false, hasInitMom = false;
    double restMass = 938.272029;          // MeV
    double current = -1.0;                 // mA in beam.txt, A after InitParticle
    double frequency = -1.0;               // Hz
    double kineticEnergy = -1.0;           // MeV
    double kineticEnergySpread = 0.0;      // DLL only: optional second value of kneticenergy
    std::string kind = "proton";
    std::array<double, 2> beamLength{};
    int bunchNumber = 0;
    int recScField = 0;
    int impScField = 0;

    // filled by the lattice reader
    int needSynPhase = 0;
    std::vector<double> outputPlanes;
    std::vector<std::string> outputNames;  // corrected engine: outputplane d NAME (parallel, "" = unnamed)
};

// Read input.txt / beam.txt from the input directory.  Throws EngineError with the original
// messages; number syntax errors raise NumberError (handled by the caller).
void read_input(Params& p);
void read_beam(Params& p);

}  // namespace avas
