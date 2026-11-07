// Lattice: elements, z positions, output planes, field maps (spec 10 2.6, spec 30).
#pragma once
#include <array>
#include <string>
#include <vector>

#include "core/fieldmap.h"
#include "core/params.h"

namespace avas {

enum class ElemKind { Drift, Field };

struct Element {
    ElemKind kind = ElemKind::Drift;
    std::string typeName = "drift";
    std::vector<std::string> words;  // field rows: word9 = map index, word14 = cavity index
    int index = 0;
    double zStart = 0.0;
    double length = 0.0;
    double radius = 0.0;
    double zEnd = 0.0;
    int inSuperposeOut = 0;
    int rotMode = 0;
    // field elements
    int fieldType = 1;
    double frequency = 0.0;  // Hz
    double phaseRad = 0.0;
    double ke = 0.0;
    double kb = 0.0;
    int mapIndex = -1;
    int cavIndex = -1;
};

struct Lattice {
    std::vector<Element> elements;
    std::vector<double> elemZStart, elemLength, elemZEnd;
    FieldLibrary fm;
    // superposeout exits {z0, x0, y0, thz, thx, thy, z_exit}; always ends with the sentinel row
    std::vector<std::array<double, 7>> spOut;
    int spOutCur = 0;
    int spOutCount = 0;
    int spCrossCount = 0;

    double total_length() const { return elemZEnd.empty() ? 0.0 : elemZEnd.back(); }
    double sp_exit() const {  // spOut[spOutCur][6]; an index past the end counts as +inf
        return spOutCur < static_cast<int>(spOut.size()) ? spOut[spOutCur][6] : 1e300;
    }
};

// Reads lattice.txt (and fills p.outputPlanes / p.needSynPhase).
void init_lattice(Lattice& lat, Params& p);

}  // namespace avas
