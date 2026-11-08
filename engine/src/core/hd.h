// Device-neutral annotations for the physics code shared by the CPU, CUDA (and later Metal) builds.
//
// The physics headers (physics.h, pmath.h, kernels.h, reduce.h) only use the macros below and the
// math wrappers in namespace avas::hm; nothing in them is specific to one device API.  The CUDA
// back end (src/cuda/) compiles the same headers with nvcc, where AVAS_HD becomes
// __host__ __device__.  Memory management, kernel launches and streams live in the back end only.
#pragma once
#include <math.h>

#include <cmath>
#include <cstdint>
#include <cstring>

#if defined(__CUDACC__)
#undef AVAS_HD
#define AVAS_HD __host__ __device__
#define AVAS_DEVICE_COMPILE 1
#else
#ifndef AVAS_HD
#define AVAS_HD
#endif
#endif

// Restrict-qualified pointers where the back end can use them (no effect on the results).
#if defined(__CUDACC__) || defined(__GNUC__) || defined(__clang__)
#define AVAS_RESTRICT __restrict__
#elif defined(_MSC_VER)
#define AVAS_RESTRICT __restrict
#else
#define AVAS_RESTRICT
#endif

namespace avas {
namespace hm {

// Correctly rounded (IEEE 754) on every target: sqrt, fabs, ceil, floor.  Transcendental functions
// are NOT taken from here when the CPU and the GPU must agree bit for bit; see pmath.h.
AVAS_HD inline double sqrt(double x) { return ::sqrt(x); }
AVAS_HD inline float sqrt(float x) { return ::sqrtf(x); }
AVAS_HD inline double fabs(double x) { return ::fabs(x); }
AVAS_HD inline float fabs(float x) { return ::fabsf(x); }
AVAS_HD inline double ceil(double x) { return ::ceil(x); }
AVAS_HD inline float ceil(float x) { return ::ceilf(x); }
AVAS_HD inline double floor(double x) { return ::floor(x); }
AVAS_HD inline float floor(float x) { return ::floorf(x); }

// Bit pattern of a double and back (exact).
AVAS_HD inline std::uint64_t dbits(double x) {
#if defined(__CUDA_ARCH__)
    return static_cast<std::uint64_t>(__double_as_longlong(x));
#else
    std::uint64_t u;
    std::memcpy(&u, &x, 8);
    return u;
#endif
}
AVAS_HD inline double dfrom(std::uint64_t u) {
#if defined(__CUDA_ARCH__)
    return __longlong_as_double(static_cast<long long>(u));
#else
    double x;
    std::memcpy(&x, &u, 8);
    return x;
#endif
}

}  // namespace hm
}  // namespace avas
