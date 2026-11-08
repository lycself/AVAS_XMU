// Portable elementary functions (double precision) for code that runs on the CPU and on the GPU and
// must give bit-identical results on both.
//
// Written with + - * / only, plus exact operations (fabs, integer conversion, exponent bits), so the
// result depends only on IEEE 754 double arithmetic -- not on the C library, the CUDA math library or
// the compiler -- as long as FMA contraction is disabled (CMake: -ffp-contract=off / /fp:precise,
// nvcc --fmad=false).  The algorithms and coefficients are those of fdlibm (Sun Microsystems,
// "Developed at SunPro ... Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved"); accuracy below 1 ulp in the ranges used here.
//
// The corrected (default) engine uses these where a transcendental function is evaluated per
// particle (hard-edge quadrupole and solenoid matrices, the bent pipe of boundary.txt, the RF
// phase of secondary particles).  Values that are the same for every particle (RF phase of a step,
// rotation angles of elements) are computed once on the host with the C library and passed in.
#pragma once
#include <cstdint>
#include <cstring>

#include "core/hd.h"

namespace avas {
namespace pm {

AVAS_HD inline std::uint64_t bits(double x) { return hm::dbits(x); }
AVAS_HD inline double from_bits(std::uint64_t u) { return hm::dfrom(u); }
AVAS_HD inline std::uint32_t high_word(double x) { return static_cast<std::uint32_t>(bits(x) >> 32); }

// x * 2^k (k within [-2042, 2046]); exact when the result is normal.
AVAS_HD inline double scale2(double x, int k) {
    if (k > 1023) {
        x = x * from_bits(static_cast<std::uint64_t>(2046) << 52);  // 2^1023
        k -= 1023;
    } else if (k < -1021) {
        x = x * from_bits(static_cast<std::uint64_t>(1) << 52);  // 2^-1022
        k += 1022;
    }
    return x * from_bits(static_cast<std::uint64_t>(k + 1023) << 52);
}

// fdlibm __kernel_sin / __kernel_cos on [-pi/4, pi/4]; y is the tail of the reduced argument.
AVAS_HD inline double kernel_sin(double x, double y, bool tail) {
    const double S1 = -1.66666666666666324348e-01, S2 = 8.33333333332248946124e-03,
                 S3 = -1.98412698298579493134e-04, S4 = 2.75573137070700676789e-06,
                 S5 = -2.50507602534068634195e-08, S6 = 1.58969099521155010221e-10;
    const double z = x * x;
    const double v = z * x;
    const double r = S2 + z * (S3 + z * (S4 + z * (S5 + z * S6)));
    if (!tail) return x + v * (S1 + z * r);
    return x - ((z * (0.5 * y - v * r) - y) - v * S1);
}

AVAS_HD inline double kernel_cos(double x, double y) {
    const double C1 = 4.16666666666666019037e-02, C2 = -1.38888888888741095749e-03,
                 C3 = 2.48015872894767294178e-05, C4 = -2.75573143513906633035e-07,
                 C5 = 2.08757232129817482790e-09, C6 = -1.13596475577881948265e-11;
    const std::uint32_t ix = high_word(x) & 0x7fffffffu;
    const double z = x * x;
    const double r = z * (C1 + z * (C2 + z * (C3 + z * (C4 + z * (C5 + z * C6)))));
    if (ix < 0x3FD33333u) return 1.0 - (0.5 * z - (z * r - x * y));  // |x| < 0.3
    double qx;
    if (ix > 0x3fe90000u) qx = 0.28125;  // |x| > 0.78125
    else qx = from_bits(static_cast<std::uint64_t>(ix - 0x00200000u) << 32);  // x/4, low word 0
    const double hz = 0.5 * z - qx;
    const double a = 1.0 - qx;
    return a - (hz - (z * r - x * y));
}

// Reduction x = n pi/2 + (y0 + y1), |y0 + y1| <= pi/4 (fdlibm __ieee754_rem_pio2, medium range:
// accurate for |x| up to about 2^20 pi/2; the phases and angles of the engine are far below).
AVAS_HD inline int rem_pio2(double x, double& y0, double& y1) {
    const double invpio2 = 6.36619772367581382433e-01, pio2_1 = 1.57079632673412561417e+00,
                 pio2_1t = 6.07710050650619224932e-11, pio2_2 = 6.07710050630396597660e-11,
                 pio2_2t = 2.02226624879595063154e-21, pio2_3 = 2.02226624871116645580e-21,
                 pio2_3t = 8.47842766036889956997e-32;
    const std::uint32_t hx = high_word(x);
    const std::uint32_t ix = hx & 0x7fffffffu;
    if (ix <= 0x3fe921fbu) {  // |x| ~<= pi/4
        y0 = x;
        y1 = 0.0;
        return 0;
    }
    const double t = x < 0.0 ? -x : x;
    const int n = static_cast<int>(t * invpio2 + 0.5);
    const double fn = static_cast<double>(n);
    double r = t - fn * pio2_1;
    double w = fn * pio2_1t;
    y0 = r - w;
    const int j = static_cast<int>(ix >> 20);
    int i = j - static_cast<int>((high_word(y0) >> 20) & 0x7ff);
    if (i > 16) {
        double tt = r;
        w = fn * pio2_2;
        r = tt - w;
        w = fn * pio2_2t - ((tt - r) - w);
        y0 = r - w;
        i = j - static_cast<int>((high_word(y0) >> 20) & 0x7ff);
        if (i > 49) {
            tt = r;
            w = fn * pio2_3;
            r = tt - w;
            w = fn * pio2_3t - ((tt - r) - w);
            y0 = r - w;
        }
    }
    y1 = (r - y0) - w;
    if (x < 0.0) {
        y0 = -y0;
        y1 = -y1;
        return -n;
    }
    return n;
}

}  // namespace pm

AVAS_HD inline void p_sincos(double x, double& s, double& c) {
    double y0, y1;
    const int n = pm::rem_pio2(x, y0, y1);
    const bool tail = n != 0 || y1 != 0.0;
    const double ks = pm::kernel_sin(y0, y1, tail), kc = pm::kernel_cos(y0, y1);
    switch (n & 3) {
        case 0: s = ks; c = kc; break;
        case 1: s = kc; c = -ks; break;
        case 2: s = -ks; c = -kc; break;
        default: s = -kc; c = ks; break;
    }
}

AVAS_HD inline double p_sin(double x) {
    double s, c;
    p_sincos(x, s, c);
    return s;
}

AVAS_HD inline double p_cos(double x) {
    double s, c;
    p_sincos(x, s, c);
    return c;
}

// fdlibm __ieee754_exp for finite x with a normal result (|x| < 708).
AVAS_HD inline double p_exp(double x) {
    const double ln2HI = 6.93147180369123816490e-01, ln2LO = 1.90821492927058770002e-10,
                 invln2 = 1.44269504088896338700e+00, P1 = 1.66666666666666019037e-01,
                 P2 = -2.77777777770155933842e-03, P3 = 6.61375632143793436117e-05,
                 P4 = -1.65339022054652515390e-06, P5 = 4.13813679705723846039e-08;
    const std::uint32_t ix = pm::high_word(x) & 0x7fffffffu;
    const bool neg = x < 0.0;
    double hi = 0.0, lo = 0.0;
    int k = 0;
    if (ix > 0x3fd62e42u) {      // |x| > 0.5 ln2
        if (ix < 0x3FF0A2B2u) {  // |x| < 1.5 ln2
            hi = neg ? x + ln2HI : x - ln2HI;
            lo = neg ? -ln2LO : ln2LO;
            k = neg ? -1 : 1;
        } else {
            k = static_cast<int>(invln2 * x + (neg ? -0.5 : 0.5));
            const double t = static_cast<double>(k);
            hi = x - t * ln2HI;
            lo = t * ln2LO;
        }
        x = hi - lo;
    } else if (ix < 0x3e300000u) {  // |x| < 2^-28
        return 1.0 + x;
    }
    const double t = x * x;
    const double c = x - t * (P1 + t * (P2 + t * (P3 + t * (P4 + t * P5))));
    if (k == 0) return 1.0 - ((x * c) / (c - 2.0) - x);
    const double y = 1.0 - ((lo - (x * c) / (2.0 - c)) - hi);
    return pm::scale2(y, k);
}

AVAS_HD inline double p_cosh(double x) {
    const double a = x < 0.0 ? -x : x;
    const double e = p_exp(a);
    return 0.5 * e + 0.5 / e;
}

AVAS_HD inline double p_sinh(double x) {
    const double a = x < 0.0 ? -x : x;
    double s;
    if (a < 1.0) {
        // odd Taylor series up to x^23/23! (truncation below 1e-22 for |x| < 1)
        const double z = a * a;
        double p = 1.0 / 25852016738884976640000.0;  // 1/23!
        p = p * z + 1.0 / 51090942171709440000.0;    // 1/21!
        p = p * z + 1.0 / 121645100408832000.0;      // 1/19!
        p = p * z + 1.0 / 355687428096000.0;         // 1/17!
        p = p * z + 1.0 / 1307674368000.0;           // 1/15!
        p = p * z + 1.0 / 6227020800.0;              // 1/13!
        p = p * z + 1.0 / 39916800.0;                // 1/11!
        p = p * z + 1.0 / 362880.0;                  // 1/9!
        p = p * z + 1.0 / 5040.0;                    // 1/7!
        p = p * z + 1.0 / 120.0;                     // 1/5!
        p = p * z + 1.0 / 6.0;                       // 1/3!
        s = a + a * (z * p);
    } else {
        const double e = p_exp(a);
        s = 0.5 * e - 0.5 / e;
    }
    return x < 0.0 ? -s : s;
}

// fdlibm __ieee754_log for finite x > 0 (the Green table of SPICNIC / PICNIC; 0 for x <= 0).
AVAS_HD inline double p_log(double x) {
    const double ln2HI = 6.93147180369123816490e-01, ln2LO = 1.90821492927058770002e-10,
                 two54 = 1.80143985094819840000e+16, Lg1 = 6.666666666666735130e-01,
                 Lg2 = 3.999999999940941908e-01, Lg3 = 2.857142874366239149e-01, Lg4 = 2.222219843214978396e-01,
                 Lg5 = 1.818357216161805012e-01, Lg6 = 1.531383769920937332e-01, Lg7 = 1.479819860511658591e-01;
    if (!(x > 0.0)) return 0.0;
    std::int32_t hx = static_cast<std::int32_t>(pm::high_word(x));
    int k = 0;
    if (hx < 0x00100000) {  // subnormal
        k -= 54;
        x = x * two54;
        hx = static_cast<std::int32_t>(pm::high_word(x));
    }
    if (hx >= 0x7ff00000) return x + x;
    k += (hx >> 20) - 1023;
    hx &= 0x000fffff;
    std::int32_t i = (hx + 0x95f64) & 0x100000;
    x = pm::from_bits((static_cast<std::uint64_t>(static_cast<std::uint32_t>(hx | (i ^ 0x3ff00000))) << 32) |
                      (pm::bits(x) & 0xffffffffull));  // x or x/2 in [sqrt(2)/2, sqrt(2))
    k += i >> 20;
    const double f = x - 1.0;
    const double dk = static_cast<double>(k);
    if ((0x000fffff & (2 + hx)) < 3) {  // |f| < 2^-20
        if (f == 0.0) return k == 0 ? 0.0 : dk * ln2HI + dk * ln2LO;
        const double R = f * f * (0.5 - 0.33333333333333333 * f);
        return k == 0 ? f - R : dk * ln2HI - ((R - dk * ln2LO) - f);
    }
    const double s = f / (2.0 + f);
    const double z = s * s;
    i = hx - 0x6147a;
    const double w = z * z;
    const std::int32_t j = 0x6b851 - hx;
    const double t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
    const double t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
    i |= j;
    const double R = t2 + t1;
    if (i > 0) {
        const double hfsq = 0.5 * f * f;
        if (k == 0) return f - (hfsq - s * (hfsq + R));
        return dk * ln2HI - ((hfsq - (s * (hfsq + R) + dk * ln2LO)) - f);
    }
    if (k == 0) return f - s * (f - R);
    return dk * ln2HI - ((s * (f - R) - dk * ln2LO) - f);
}

// fdlibm atan.
AVAS_HD inline double p_atan(double x) {
    const double atanhi[4] = {4.63647609000806093515e-01, 7.85398163397448278999e-01, 9.82793723247329054082e-01,
                              1.57079632679489655800e+00};
    const double atanlo[4] = {2.26987774529616870924e-17, 3.06161699786838301793e-17, 1.39033110312309984516e-17,
                              6.12323399573676603587e-17};
    const double aT[11] = {3.33333333333329318027e-01,  -1.99999999998764832476e-01, 1.42857142725034663711e-01,
                           -1.11111104054623557880e-01, 9.09088713343650656196e-02,  -7.69187620504482999495e-02,
                           6.66107313738753120669e-02,  -5.83357013379057348645e-02, 4.97687799461593236017e-02,
                           -3.65315727442169155270e-02, 1.62858201153657823623e-02};
    if (x != x) return x;  // NaN
    const bool neg = x < 0.0;
    const std::uint32_t ix = pm::high_word(x) & 0x7fffffffu;
    if (ix >= 0x44100000u) return neg ? -(atanhi[3] + atanlo[3]) : atanhi[3] + atanlo[3];  // |x| >= 2^66
    int id;
    if (ix < 0x3fdc0000u) {                       // |x| < 0.4375
        if (ix < 0x3e200000u) return x;           // |x| < 2^-29
        id = -1;
    } else {
        x = neg ? -x : x;
        if (ix < 0x3ff30000u) {                   // |x| < 1.1875
            if (ix < 0x3fe60000u) {               // 7/16 <= |x| < 11/16
                id = 0;
                x = (2.0 * x - 1.0) / (2.0 + x);
            } else {                              // 11/16 <= |x| < 19/16
                id = 1;
                x = (x - 1.0) / (x + 1.0);
            }
        } else if (ix < 0x40038000u) {            // |x| < 2.4375
            id = 2;
            x = (x - 1.5) / (1.0 + 1.5 * x);
        } else {                                  // 2.4375 <= |x| < 2^66
            id = 3;
            x = -1.0 / x;
        }
    }
    const double z = x * x;
    const double w = z * z;
    const double s1 = z * (aT[0] + w * (aT[2] + w * (aT[4] + w * (aT[6] + w * (aT[8] + w * aT[10])))));
    const double s2 = w * (aT[1] + w * (aT[3] + w * (aT[5] + w * (aT[7] + w * aT[9]))));
    if (id < 0) return x - x * (s1 + s2);
    const double r = atanhi[id] - ((x * (s1 + s2) - atanlo[id]) - x);
    return neg ? -r : r;
}

}  // namespace avas
