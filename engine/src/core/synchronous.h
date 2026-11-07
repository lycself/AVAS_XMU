// Synchronous particle and RF phase determination (SetSynPhase / singlePartM, spec 20).
#pragma once
#include "core/lattice.h"
#include "core/params.h"

namespace avas {

// SetSynPhase(lattice, gbzIn): phase scan of the V3 = 0 cavities, main pass writing synData.txt,
// and the final cavity phases / times in lat.fm.  singleParticleMode selects the singlePartM
// variant (gbzIn = -33, main step 0.1/f/N, synData fully in 17 digits).
void set_synchronous_phase(Lattice& lat, Params& p, double gbzIn, bool singleParticleMode);

}  // namespace avas
