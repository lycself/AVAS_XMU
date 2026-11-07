// Bunch tracking: MethodMP::Run / tcodeRun / TcodeTrans (spec 40, spec 20 2.11).
#pragma once
#include "core/beam.h"
#include "core/lattice.h"
#include "core/params.h"

namespace avas {

// Runs the time-step tracking of the whole lattice and writes all run outputs.
void run_tracking(Lattice& lat, Beam& beam, Params& p);

}  // namespace avas
