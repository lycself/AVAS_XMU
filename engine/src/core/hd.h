// Device-neutral annotations for the physics code shared by the CPU, CUDA and Metal builds.
//
// The physics headers (physics.h, pmath.h, kernels.h, reduce.h) only use the macros below and the
// math wrappers in namespace avas::hm; nothing in them is specific to one device API.  The CUDA
// back end (src/cuda/) compiles the same headers with nvcc, where AVAS_HD becomes
// __host__ __device__.  Memory management, kernel launches and streams live in the back end only.
#pragma once

#if defined(__METAL_VERSION__)
// ---- Metal (MSL, compiled at run time by the Metal back end, src/metal/) ----
// MSL has no double, no long long and needs an address space on every pointer and reference:
// the back end defines `double` as float before this header; AVAS_G / AVAS_T mark device and
// thread memory, avas_i64 is the 64-bit integer.  Only the headers listed in metal_backend.mm
// are compiled for Metal.
#include <metal_stdlib>
#define AVAS_HD
#define AVAS_RESTRICT
#define AVAS_G device
#define AVAS_T thread
#define AVAS_PCONST constant  // program-scope constants
#define AVAS_DEVICE_COMPILE 1
#define AVAS_METAL 1
#define AVAS_FLOAT_ONLY 1
typedef long avas_i64;
typedef ulong avas_u64;

namespace avas {
namespace hm {
inline float sqrt(float x) { return metal::precise::sqrt(x); }
inline float fabs(float x) { return metal::fabs(x); }
inline float ceil(float x) { return metal::ceil(x); }
inline float floor(float x) { return metal::floor(x); }
inline float ldexp(float x, int e) { return metal::ldexp(x, e); }
}  // namespace hm
}  // namespace avas

#elif defined(AVAS_F32_CUDA)
// ---- the CUDA fp32 tier (src/cuda/cuda_f32_device.cu): the Metal world compiled by nvcc ----
// As AVAS_F32_CPU (namespace avas_f32, `double` defined as float by the including file), with
// __host__ __device__ functions; --fmad=false, IEEE division and square root (no fast math).
#define AVAS_HD __host__ __device__
#define AVAS_RESTRICT __restrict__
#define AVAS_G
#define AVAS_T
#define AVAS_PCONST
#define AVAS_DEVICE_COMPILE 1
#define AVAS_FLOAT_ONLY 1
typedef long long avas_i64;
typedef unsigned long long avas_u64;

namespace avas {
namespace hm {
AVAS_HD inline float sqrt(float x) { return ::sqrtf(x); }
AVAS_HD inline float fabs(float x) { return ::fabsf(x); }
AVAS_HD inline float ceil(float x) { return ::ceilf(x); }
AVAS_HD inline float floor(float x) { return ::floorf(x); }
AVAS_HD inline float ldexp(float x, int e) { return ::ldexpf(x, e); }
}  // namespace hm
}  // namespace avas

#elif defined(AVAS_F32_CPU)
// ---- the fp32 tier of the CPU device back end (core/f32_cpu.cpp): the Metal world in C++ ----
// The including file has included the standard headers, renamed the namespace (avas -> avas_f32)
// and defined `double` as float; the shared headers then compile to the float code of Metal.
#define AVAS_HD
#define AVAS_RESTRICT
#define AVAS_G
#define AVAS_T
#define AVAS_PCONST
#define AVAS_FLOAT_ONLY 1
typedef long long avas_i64;
typedef unsigned long long avas_u64;

namespace avas {
namespace hm {
inline float sqrt(float x) { return std::sqrt(x); }
inline float fabs(float x) { return std::fabs(x); }
inline float ceil(float x) { return std::ceil(x); }
inline float floor(float x) { return std::floor(x); }
inline float ldexp(float x, int e) { return std::ldexp(x, e); }
}  // namespace hm
}  // namespace avas

#else
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

// Address spaces of the Metal build (empty on the CPU and in CUDA).
#define AVAS_G
#define AVAS_T
#define AVAS_PCONST
typedef long long avas_i64;
typedef unsigned long long avas_u64;

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

#endif  // __METAL_VERSION__
