// Flat copy of the lattice for the shared particle kernels (kernels.h): plain arrays that can be
// used directly on the CPU or copied to a GPU.  Angles that are the same for every particle are
// turned into sin / cos here, once, with the C library (as the tracker did before).
#pragma once
#include <vector>

#include "core/beam.h"
#include "core/kernels.h"
#include "core/lattice.h"

namespace avas {

// Particle view of a Beam (old / rec / spRec left null).
kern::PartV beam_view(Beam& b);

struct LatHost {
    std::vector<kern::ElemV> elems, eff;
    std::vector<kern::SubV> subs;
    std::vector<MapView<double>> maps;
    std::vector<double> zStart, zEnd;
    std::vector<kern::SpOutV> spOut;
    std::vector<kern::RfSlotV> slots;
    std::vector<double> cavTime, cavPhase;
    std::vector<kern::GeomV> geoms;
    std::vector<double> segStart, segEnd;
    std::vector<double> bndZ;
    std::vector<int> bndR1, bndR2;
    std::vector<double> rfqCells;  // 14 per rfq cell (kernels.h LatV::rfqCells)
    bool errOn = false;
    bool hasEff = false;

    // errUse / hetErr: element error flags of the current sample (empty: none); spEff: corrected
    // superpose blocks with sub-field errors (empty: none).
    void build(const Lattice& lat, const std::vector<char>& errUse, const std::vector<char>& hetErr,
               const std::vector<Element>& spEff);
    // Element rotMode (changed by the superposeout beam frame during the run).
    // Returns true when an element changed.
    bool sync_dynamic(const Lattice& lat);
    // View on the host arrays; the scalars (superposeout state) are taken from lat.
    kern::LatV view(const Lattice& lat) const;
    // cos(phase), cos(phase - pi/2) of every RF slot at time t (2 per slot).
    void rf_table(double t, std::vector<double>& out) const;
};

}  // namespace avas
