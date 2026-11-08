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

// Elementary functions of compat=dll for the shared particle kernels (kernels.h); host only.
struct DllMath {
    static constexpr bool compat = true;
    static void sincos(double a, double& s, double& c) { dll_sincos(a, s, c); }
    static double cos(double a) { return dll_cos(a); }
    static double sinh(double a) { return std::sinh(a); }
    static double cosh(double a) { return std::cosh(a); }
    static double atan(double a) { return std::atan(a); }
};

}  // namespace avas
