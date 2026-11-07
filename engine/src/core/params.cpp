#include "core/params.h"

#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/text.h"

namespace avas {

namespace {

// word k of a line; a missing word is undefined behaviour in the original (operator[]),
// here it is reported.
const std::string& word(const std::vector<std::string>& w, size_t k) {
    if (k >= w.size()) throw EngineError("keyword: " + w[0] + ", Insufficient parameters");
    return w[k];
}

std::vector<std::vector<std::string>> read_lines(const std::string& name) {
    std::string raw;
    if (!read_file(paths().input + name, raw)) {
        throw EngineError("File \"" + name + "\" cannot be opened, Check the inputFile routing and " + name + ".");
    }
    std::vector<std::vector<std::string>> out;
    for (const std::string& line : split_lines(normalize_line_endings(raw))) {
        std::vector<std::string> w = split_words(line);
        if (!w.empty()) {
            w[0] = to_lower(w[0]);  // only the keyword is lower-cased
            out.push_back(std::move(w));
        }
    }
    return out;
}

}  // namespace

void read_input(Params& p) {
    for (const auto& w : read_lines("input.txt")) {
        const std::string& k = w[0];
        bool known = true;
        if (k == "method") {
            p.method = word(w, 1);
            known = false;  // the original falls through to the "unknown keyword" warning
        } else if (k == "secondarybeam") {
            p.secondaryBeam = parse_int(word(w, 1));
        } else if (k == "boundary") {
            p.boundary = parse_int(word(w, 1));
        } else if (k == "probe") {
            p.probe = parse_int(word(w, 1));
        } else if (k == "dimension") {
            throw EngineError("input.txt Contains unknown keywords: Dimension");
        } else if (k == "scmethod") {
            const std::string& v = word(w, 1);
            if (v == "FFT") p.scPar[0] = 0.0;
            else if (v == "SPICNIC") p.scPar[0] = 1.0;
            else throw EngineError("SCMethod Contains unknown item, available options include FFT / SPICNIC");
        } else if (k == "negligibleratio") {
            p.scPar[8] = parse_double(word(w, 1));
        } else if (k == "lpboundary") {
            p.lpBoundary = parse_int(word(w, 1));
        } else if (k == "indweight") {
            p.indWeight = parse_int(word(w, 1));
        } else if (k == "longlimits") {
            for (size_t i = 1; i < w.size() && i <= p.longLimits.size(); ++i) p.longLimits[i - 1] = parse_double(w[i]);
        } else if (k == "indcharge") {
            p.indCharge = parse_int(word(w, 1));
        } else if (k == "meshrms") {
            for (size_t i = 1; i < w.size() && i < p.scPar.size(); ++i) p.scPar[i] = parse_double(w[i]);
        } else if (k == "integralgrid") {
            for (size_t i = 1; i < w.size() && 3 + i < p.scPar.size(); ++i) p.scPar[3 + i] = parse_double(w[i]);
        } else if (k == "scutoffdistance") {
            p.scPar[7] = static_cast<double>(parse_int(word(w, 1)));
        } else if (k == "scantstep") {
            p.scanTStep = parse_double(word(w, 1));
        } else if (k == "dumpperiodicity") {
            p.dumpPeriodicity = static_cast<int>(parse_double(word(w, 1)));
        } else if (k == "spacecharge") {
            p.spaceCharge = parse_int(word(w, 1));
            if (p.spaceCharge == 0) say("Simulation ignores space charge effects");
        } else if (k == "singleparticlemode") {
            p.singleParticleMode = parse_int(word(w, 1));
        } else if (k == "scsolverrecord") {
            p.scSolverRecord = parse_int(word(w, 1));
        } else if (k == "pchistogram") {
            for (size_t i = 1; i < w.size() && i <= p.pcHistogram.size(); ++i)
                p.pcHistogram[i - 1] = static_cast<double>(parse_int(w[i]));
        } else if (k == "numofgrid") {
            for (size_t i = 1; i < w.size() && i <= p.numOfGrid.size(); ++i) p.numOfGrid[i - 1] = parse_int(w[i]);
        } else if (k == "steppercycle") {
            p.stepPerCycle = parse_double(word(w, 1));
            if (w.size() >= 3) p.seStep = parse_double(w[2]);
        } else if (k == "runperiod") {
            p.runPeriod = parse_int(word(w, 1));
        } else if (k == "track") {
            for (size_t i = 1; i < w.size(); ++i) p.track.push_back(parse_int(w[i]));
        } else if (k == "randomseed") {
            p.randomSeed = static_cast<double>(parse_int(word(w, 1)));
        } else if (k == "multithreading") {
            p.multithreading = parse_int(word(w, 1));
        } else if (k == "setnumberofcores") {
            if (w.size() < 11) throw EngineError("keyword: setnumberofcores, Insufficient parameters, need 10 parameters");
            for (size_t i = 1; i < w.size() && i <= p.cores.size(); ++i) p.cores[i - 1] = parse_int(w[i]);
            const int maxT = omp_max_threads();
            for (size_t i = 0; i < p.cores.size(); ++i) {
                if (p.cores[i] > maxT) p.cores[i] = maxT;
                p.scPar[10 + i] = static_cast<double>(p.cores[i]);
            }
        } else if (k == "maxthreads") {
            p.maxThreads = parse_int(word(w, 1));
        } else {
            known = false;
        }
        if (!known) say("input.txt Contains unknown keywords, check input.txt");
    }
    if (p.secondaryBeam == 1) p.spaceCharge = 0;
    say("Parameters and functions are set");
    say("  ");
}

void read_beam(Params& p) {
    for (const auto& w : read_lines("beam.txt")) {
        const std::string& k = w[0];
        if (k == "readparticledistribution") {
            p.distFile = word(w, 1);
        } else if (k == "numofcharge") {
            p.charge = parse_double(word(w, 1));
        } else if (k == "particlerestmass") {
            p.restMass = parse_double(word(w, 1));
        } else if (k == "kindofparticle") {
            p.kind = word(w, 1);
        } else if (k == "distribution") {
            if (w.size() != 3) {
                throw EngineError("The initial bunch distribution contains two parameters, separately in transvers and longitudinal");
            }
            for (int i = 0; i < 2; ++i) {
                p.distribution[i] = w[i + 1];
                const std::string& d = p.distribution[i];
                if (d != "KV" && d != "WB" && d != "PB" && d != "GS") {
                    throw EngineError("Distribution Type Error, The Format is: distribution  KV/WB/PB/GS  KV/WB/PB/GS");
                }
            }
        } else if (k == "beamtype") {
            if (word(w, 1) == "dc") p.twiss[6] = kTwissDcMark;
        } else if (k == "twissx" || k == "twissy" || k == "twissz") {
            if (w.size() != 4) throw EngineError(k + " have 3 parameters, emittance, beta, alpha");
            const int base = k == "twissx" ? 0 : (k == "twissy" ? 3 : 6);
            if (base == 6 && p.twiss[6] == kTwissDcMark) {
                p.twiss[6] = kTwissUnset;  // dc given before twissz: twissz ignored
            } else {
                p.twiss[base + 2] = parse_double(w[1]);  // alpha
                p.twiss[base + 1] = parse_double(w[2]);  // beta
                p.twiss[base + 0] = parse_double(w[3]);  // emittance
            }
        } else if (k == "particlenumber") {
            p.particleNumber = parse_int(word(w, 1));
        } else if (k == "displacepos") {
            for (size_t i = 1; i < w.size() && i <= 3; ++i) p.displacePos[i - 1] = parse_double(w[i]) * 0.001;
        } else if (k == "displacedpos") {
            for (size_t i = 1; i < w.size() && i <= 3; ++i) p.displacedPos[i - 1] = parse_double(w[i]) * 0.001;
        } else if (k == "frequency") {
            p.frequency = parse_double(word(w, 1));
        } else if (k == "kneticenergy") {
            p.kineticEnergy = parse_double(word(w, 1));
            if (w.size() >= 3) p.kineticEnergySpread = parse_double(w[2]);
        } else if (k == "beamlength") {
            p.beamLength[0] = static_cast<double>(parse_int(word(w, 1)));
            p.beamLength[1] = parse_double(word(w, 2));
        } else if (k == "current") {
            p.current = parse_double(word(w, 1));
        } else if (k == "bunchnumber") {
            p.bunchNumber = parse_int(word(w, 1));
        } else if (k == "randomseed") {
            p.randomSeed = parse_double(word(w, 1));
        } else if (k == "recscfield") {
            p.recScField = parse_int(word(w, 1));
        } else if (k == "impscfield") {
            p.impScField = parse_int(word(w, 1));
        } else {
            say("beam.txt Contains unknown keywords, check input.txt");
        }
    }
}

}  // namespace avas
