// Error studies inside the engine: sampling of the err_* lines (GenerateErrorSet), their
// application to the elements (SetErrPara) and the sample output (errSampleOut), spec 70.
#pragma once
#include <cstdint>
#include <vector>

#include "core/lattice.h"
#include "core/rng.h"

namespace avas {

// All a*b samples.  compat=dll: MSVC rand() from the process state (1 in a new process), as the
// original samples before any srand; corrected: Philox keyed by the seed.
std::vector<ErrorSample> generate_error_set(const Lattice& lat, MsvcRand& rng, std::uint64_t seed);

// SetErrInEle: AVAS.dll changes the quad / solenoid / bend / edge parameters in place (errors of
// successive samples accumulate); the corrected engine is given a fresh copy of the lattice.
void set_errors_in_elements(Lattice& lat, const ErrorSample& es);

// StaticErrorData.txt / DynamicErrorData.txt in the current output folder.
void write_error_sample(const ErrorSample& es);

}  // namespace avas
