// Random numbers and portable transcendental functions for the initial distribution (spec 50).
//
// * MsvcRand: the MSVC CRT rand() of AVAS.dll (LCG, RAND_MAX 32767), reproduced exactly on every
//   platform for compat=dll.
// * Philox4x32-10: the counter-based generator of the corrected engine.  Every random number is a
//   pure function of (seed, particle index, stream, draw), so the result does not depend on the
//   thread count, the order of the calls or the device.
// * p_log / p_sincos_turns: log and sin/cos written with + - * / only (no libm, no FMA), so that a
//   seed gives bit-identical particles on Windows, macOS, Linux and later CUDA / Metal.
#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>

#ifndef AVAS_HD
#define AVAS_HD
#endif

namespace avas {

// ---- MSVC CRT rand() -------------------------------------------------------------------------
struct MsvcRand {
    std::uint32_t state = 1;
    void srand(std::uint32_t seed) { state = seed; }
    int rand() {
        state = state * 214013u + 2531011u;
        return static_cast<int>((state >> 16) & 0x7FFF);
    }
};

// ---- Philox4x32-10 (Salmon et al., SC'11) -----------------------------------------------------
struct Philox4x32 {
    std::uint32_t key[2];

    AVAS_HD static void mulhilo(std::uint32_t a, std::uint32_t b, std::uint32_t& hi, std::uint32_t& lo) {
        const std::uint64_t p = static_cast<std::uint64_t>(a) * b;
        hi = static_cast<std::uint32_t>(p >> 32);
        lo = static_cast<std::uint32_t>(p);
    }

    // Four 32-bit outputs for the counter (c0, c1, c2, c3).
    AVAS_HD void block(const std::uint32_t ctr[4], std::uint32_t out[4]) const {
        std::uint32_t c[4] = {ctr[0], ctr[1], ctr[2], ctr[3]};
        std::uint32_t k0 = key[0], k1 = key[1];
        for (int r = 0; r < 10; ++r) {
            std::uint32_t hi0, lo0, hi1, lo1;
            mulhilo(0xD2511F53u, c[0], hi0, lo0);
            mulhilo(0xCD9E8D57u, c[2], hi1, lo1);
            const std::uint32_t n0 = hi1 ^ c[1] ^ k0;
            const std::uint32_t n2 = hi0 ^ c[3] ^ k1;
            c[0] = n0;
            c[1] = lo1;
            c[2] = n2;
            c[3] = lo0;
            k0 += 0x9E3779B9u;
            k1 += 0xBB67AE85u;
        }
        for (int i = 0; i < 4; ++i) out[i] = c[i];
    }

    // Two uniform doubles in the open interval (0, 1), 53 bits each:
    // counter = (particle, stream, draw, 0).
    AVAS_HD void uniform2(std::uint32_t particle, std::uint32_t stream, std::uint32_t draw, double u[2]) const {
        const std::uint32_t ctr[4] = {particle, stream, draw, 0u};
        std::uint32_t o[4];
        block(ctr, o);
        for (int j = 0; j < 2; ++j) {
            const std::uint64_t v = (static_cast<std::uint64_t>(o[2 * j]) << 32) | o[2 * j + 1];
            u[j] = (static_cast<double>(v >> 11) + 0.5) * 1.1102230246251565e-16;  // 2^-53
        }
    }
};

// Streams of the corrected engine (independent of each other).
enum RngStream : std::uint32_t { kStreamTransverse = 1, kStreamLongitudinal = 2, kStreamDcEnergy = 3 };

// ---- portable elementary functions ----------------------------------------------------------
// Natural logarithm of x > 0 (finite, normal).  Relative error about 1e-16.
template <class R>
AVAS_HD inline R p_log(R x) {
    std::uint64_t bits;
    double xd = static_cast<double>(x);
    std::memcpy(&bits, &xd, 8);
    int e = static_cast<int>((bits >> 52) & 0x7FF) - 1023;
    bits = (bits & 0x000FFFFFFFFFFFFFull) | 0x3FF0000000000000ull;  // mantissa m in [1, 2)
    double m;
    std::memcpy(&m, &bits, 8);
    if (m > 1.4142135623730951) {
        m = m * 0.5;
        e += 1;
    }
    // log(m) = 2 atanh(f), f = (m - 1)/(m + 1), |f| <= 0.1716
    const R f = (R(m) - R(1)) / (R(m) + R(1));
    const R s = f * f;
    R p = R(1) / R(25);
    p = p * s + R(1) / R(23);
    p = p * s + R(1) / R(21);
    p = p * s + R(1) / R(19);
    p = p * s + R(1) / R(17);
    p = p * s + R(1) / R(15);
    p = p * s + R(1) / R(13);
    p = p * s + R(1) / R(11);
    p = p * s + R(1) / R(9);
    p = p * s + R(1) / R(7);
    p = p * s + R(1) / R(5);
    p = p * s + R(1) / R(3);
    const R lm = (f + f) + (f + f) * (s * p);
    const R ln2hi = R(6.93147180369123816490e-01);  // high part of log(2), 32 significant bits
    const R ln2lo = R(1.90821492927058770002e-10);
    return (R(e) * ln2hi + lm) + R(e) * ln2lo;
}

// sin and cos of 2*pi*u for u in [0, 1).
template <class R>
AVAS_HD inline void p_sincos_turns(R u, R& s, R& c) {
    // quadrant q = nearest multiple of 1/4 turn; r in [-1/8, 1/8] turn
    R t = u * R(4);
    int q = static_cast<int>(t + R(0.5));
    const R r = (t - R(q)) * R(0.25);
    const R x = r * R(6.283185307179586);
    const R x2 = x * x;
    R ps = R(-1.0) / R(1307674368000.0);              // -1/15!
    ps = ps * x2 + R(1.0) / R(6227020800.0);          // 1/13!
    ps = ps * x2 - R(1.0) / R(39916800.0);            // 1/11!
    ps = ps * x2 + R(1.0) / R(362880.0);              // 1/9!
    ps = ps * x2 - R(1.0) / R(5040.0);                // 1/7!
    ps = ps * x2 + R(1.0) / R(120.0);                 // 1/5!
    ps = ps * x2 - R(1.0) / R(6.0);                   // 1/3!
    const R sn = x + x * (x2 * ps);
    R pc = R(1.0) / R(20922789888000.0);              // 1/16!
    pc = pc * x2 - R(1.0) / R(87178291200.0);         // 1/14!
    pc = pc * x2 + R(1.0) / R(479001600.0);           // 1/12!
    pc = pc * x2 - R(1.0) / R(3628800.0);             // 1/10!
    pc = pc * x2 + R(1.0) / R(40320.0);               // 1/8!
    pc = pc * x2 - R(1.0) / R(720.0);                 // 1/6!
    pc = pc * x2 + R(1.0) / R(24.0);                  // 1/4!
    pc = pc * x2 - R(0.5);
    const R cs = R(1) + x2 * pc;
    switch (q & 3) {
        case 0: s = sn; c = cs; break;
        case 1: s = cs; c = -sn; break;
        case 2: s = -sn; c = -cs; break;
        default: s = -cs; c = sn; break;
    }
}

// Two independent standard normal numbers from two uniforms in (0, 1) (Box-Muller).
template <class R>
AVAS_HD inline void p_gauss2(R u1, R u2, R& g1, R& g2) {
    const R rad = std::sqrt(R(-2) * p_log(u1));
    R s, c;
    p_sincos_turns(u2, s, c);
    g1 = rad * c;
    g2 = rad * s;
}

// Inverse of the CDF 3s^2 - 2s^3 on [0, 1] (radial law of the 4-D parabolic distribution),
// bisection followed by Newton steps: arithmetic only.
template <class R>
AVAS_HD inline R p_parabolic_inv(R u) {
    R lo = R(0), hi = R(1);
    for (int i = 0; i < 30; ++i) {
        const R m = (lo + hi) * R(0.5);
        const R f = m * m * (R(3) - R(2) * m);
        if (f < u) lo = m;
        else hi = m;
    }
    R s = (lo + hi) * R(0.5);
    for (int i = 0; i < 4; ++i) {
        const R f = s * s * (R(3) - R(2) * s) - u;
        const R d = R(6) * s * (R(1) - s);
        if (d > R(0)) s = s - f / d;
    }
    if (s < R(0)) s = R(0);
    if (s > R(1)) s = R(1);
    return s;
}

}  // namespace avas
