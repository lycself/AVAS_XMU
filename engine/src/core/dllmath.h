// sin/cos as evaluated by the original AVAS.dll.
//
// The DLL was compiled by MSVC with /fp:fast.  Where its source needs both the sine and the
// cosine of one argument, MSVC calls its own statically linked SSE2 sincos routine instead of
// the CRT sin/cos, and that routine can differ from them in the last bit.  dllmath.cpp is
// compiled with /fp:fast on MSVC so that dll_sincos() reaches the same routine; elsewhere it is
// std::sin / std::cos.  dll_sin / dll_cos are the plain CRT functions (used where the DLL calls
// sin or cos alone).
#pragma once
#include <cmath>

namespace avas {

void dll_sincos(double a, double& s, double& c);
inline double dll_sin(double a) { return std::sin(a); }
inline double dll_cos(double a) { return std::cos(a); }

}  // namespace avas
