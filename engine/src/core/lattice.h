// Lattice: elements, z positions, output planes, field maps (spec 10 2.6, spec 30).
#pragma once
#include <array>
#include <string>
#include <vector>

#include "core/fieldmap.h"
#include "core/params.h"

namespace avas {

enum class ElemKind { Drift, Field, Quad, Solenoid, Bend, Edge, Steerer, Rfq };

struct Element {
    ElemKind kind = ElemKind::Drift;
    std::string typeName = "drift";
    std::vector<std::string> words;  // field rows: word9 = map index, word14 = cavity index
    int index = 0;
    double zStart = 0.0;
    double length = 0.0;
    double radius = 0.0;
    double zEnd = 0.0;
    int inSuperposeOut = 0;  // 1 or 2 on a superpose block ended by superposeout
    int rotMode = 0;
    // field elements
    int fieldType = 1;
    double frequency = 0.0;  // Hz
    double phaseRad = 0.0;
    double ke = 0.0;
    double kb = 0.0;
    int mapIndex = -1;
    int cavIndex = -1;
    // field type 4 (superpose block): one entry per sub-field
    std::vector<int> subType;  // 1 static magnetic, 2 RF, 3 static electric, 0 drift (original coding)
    std::vector<int> subMap;
    std::vector<double> subKe, subKb, subZStart, subRadius, subLength;
    std::vector<std::array<double, 6>> subPlace;  // z0, x0, y0, thz, thx, thy (rad)
    // matrix elements
    double strength = 0.0;  // quad G (T/m), solenoid B (T)
    int hv = 0;             // bend / edge plane: 0 horizontal, otherwise vertical
    double alpha = 0.0, rho = 0.0, fieldIndex = 0.0, arc = 0.0, h = 0.0, kx = 0.0, ky = 0.0;  // bend
    double poleAngle = 0.0, gap = 0.0, fringeK1 = 0.0, fringeK2 = 0.0, psi = 0.0;            // edge
    int steerKind = 0;
    double steerX = 0.0, steerY = 0.0, steerMax = 0.0;  // steerer Bx/Ex, By/Ey

    bool is_matrix() const {
        return kind == ElemKind::Quad || kind == ElemKind::Solenoid || kind == ElemKind::Bend || kind == ElemKind::Edge;
    }
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
    // input.txt-independent extras of the lattice
    std::vector<std::array<double, 3>> spaceChargeComp;  // AVAS.dll spacechargecomp rows {z, 1, V}

    double total_length() const { return elemZEnd.empty() ? 0.0 : elemZEnd.back(); }
    double sp_exit() const {  // spOut[spOutCur][6]; an index past the end counts as +inf
        return spOutCur < static_cast<int>(spOut.size()) ? spOut[spOutCur][6] : 1e300;
    }
};

// Reads lattice.txt (and fills p.outputPlanes / p.needSynPhase).
void init_lattice(Lattice& lat, Params& p);

}  // namespace avas
