// Field map library (the original's FieldMatrix + Lattice::SetMatrix, spec 30 2.6).
#pragma once
#include <string>
#include <vector>

#include "core/physics.h"

namespace avas {

struct FieldMap {
    std::string name;  // path prefix + map name as written in the lattice (no extension)
    int ncomp = 0;     // 6 for RF maps (Ex,Ey,Ez,Bx,By,Bz), 3 for static maps
    int nx = 0, ny = 0, nz = 0;
    double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zlen = 0, dx = 0, dy = 0, dz = 0;
    std::vector<double> data;  // node-major, node = ((ny+1)*iz + iy)*(nx+1) + ix

    MapView<double> view() const {
        return MapView<double>{data.data(), ncomp, nx, ny, nz, xmin, xmax, ymin, ymax, zlen, dx, dy, dz};
    }
};

struct FieldLibrary {
    std::vector<FieldMap> maps;
    std::vector<double> cavPhase;  // per RF cavity: entrance RF phase (deg), from SetSynPhase
    std::vector<double> cavTime;   // per RF cavity: reference time (s)

    // Loads "<name><ext>" for field type 1 (RF), 2 (static electric) or 3 (static magnetic) and
    // returns the map index (an already loaded name is reused).  Type 0 loads nothing (-1).
    int load(const std::string& name, int type);
};

}  // namespace avas
