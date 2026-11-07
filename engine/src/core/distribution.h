// Four-dimensional distribution generators KV / WB / PB / GS (spec 50 2.2).
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "core/rng.h"

namespace avas {

using Row4 = std::array<double, 4>;  // q1, q1', q2, q2' (m, rad)

// Twiss of the two planes: eps (geometric rms, m rad), beta (m/rad), alpha.
struct Twiss2 {
    double eps1, beta1, alpha1, eps2, beta2, alpha2;
};

// AVAS.dll: PreTreatment (FUN_18000fee0) with its Eigen eigen-decomposition; T[5] = 0.
std::array<double, 6> dll_pretreatment(const Twiss2& tw);

// AVAS.dll: KV2D / WB2D / PB2D / GS2D on the MSVC rand() state (reseeded when seed != 0).
void dll_generate(const std::string& type, const Twiss2& tw, std::vector<Row4>& rows, int seed, MsvcRand& rng);

// Corrected engine: independent counter-based draws per particle; stream selects the
// transverse / longitudinal sequence.
void default_generate(const std::string& type, const Twiss2& tw, std::vector<Row4>& rows, const Philox4x32& rng,
                      std::uint32_t stream);

}  // namespace avas
