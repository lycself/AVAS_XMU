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
    // element errors (SetErrPara): displacement (m), rotations (rad), scaled amplitudes
    double errDx = 0, errDy = 0, errDz = 0, errRotX = 0, errRotY = 0, errRotZ = 0;
    double keErr = 0, kbErr = 0, phaseErr = 0, fieldErr = 0;
    // superpose block: per sub-field {flag, dx, dy, dz, thx, thy, thz, kb, ke, phase, 0, 0} (SetErrPara)
    std::vector<std::array<double, 12>> subErr;
    // rfq: one row per cell of the structure file (word 7), RFQ::SetParameter of libAVAS.so:
    // {z start, z end, length (m), aperture a (m), inter-vane voltage V (V), column 4 (unused),
    //  A0 .. A7 of the 8-term potential}
    std::vector<std::array<double, 14>> rfqCells;

    bool is_matrix() const {
        return kind == ElemKind::Quad || kind == ElemKind::Solenoid || kind == ElemKind::Bend || kind == ElemKind::Edge;
    }
};

// err_* lines of lattice.txt as stored by AVAS.dll's InitLattice (spec 70 2.1-2.3).
struct ErrorParameter {
    int a = 1, b = 1;                          // err_step a b (b = amplitude levels, a = repeats)
    std::vector<double> beamVals;              // err_beam_stat / err_beam_dyn values (0 / 1 first)
    std::vector<int> beamSwitches;             // err_beam_*_on switches, padded to 30 / 60
    std::vector<double> beamLine[2];           // corrected engine: {r, x1..} of the static / dynamic line
    std::vector<int> beamSw[2];                // corrected engine: switches of the static / dynamic line
    std::vector<std::vector<double>> elemDefs; // {class, mode, E, N, r, x0..} padded to 20
    std::vector<std::vector<int>> elemSwitches;// {class, mode, s0..} padded to 20
    int nElements = 0;
    int flags[4] = {0, 0, 0, 0};               // beam static, beam dynamic, element static, element dynamic
    bool any() const { return flags[0] + flags[1] + flags[2] + flags[3] != 0; }
};

// One error sample: row 0 beam, row e + 1 element e (spec 70 3.2).
struct ErrorSample {
    std::vector<std::vector<double>> stat, dyn;
    int flags[4] = {0, 0, 0, 0};
};

// boundary.txt (spec 10 2.9, spec 40 6.12): pipe geometries along z.
struct Geometry {
    int shape = 0;  // 1 straight pipe, 2 bent pipe
    int index = 0;
    std::string name;
    double zStart = 0.0;
    std::vector<double> par;            // par[0] length, par[1] radius, bent: par[2] rho, par[3] angle (deg),
                                        // par[4] plane (0 x-z, 1 y-z), par[5] direction
    std::array<double, 6> place{};      // z0, x0, y0 (m), thz, thx, thy (rad) after "RLP"
};
struct Boundary {
    std::vector<Geometry> geoms;
    std::vector<double> segStart, segLen, segEnd;
    std::vector<std::array<double, 3>> rows;  // per superposeout section: {z offset, first, last geometry}
};

struct Lattice {
    ErrorParameter err;
    Boundary bnd;
    bool hasBoundary = false;
    bool noEnd = false;  // lattice.txt without an end line (AVAS.dll leaves the lattice unfinished)
    int bndSpOut = 0;  // superposeout sections consumed by boundary.txt
    const ErrorSample* sample = nullptr;  // current error sample (null: no errors)
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
