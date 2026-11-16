#include <algorithm>
// Unit tests of the text, format and physics helpers (no framework; exit code = failures).
#include "core/rng.h"
#include "core/pmath.h"
#include "core/reduce.h"
#include "core/kernels.h"
#include <functional>
#include "core/spacecharge.h"
#include <cmath>
#include <cstdio>
#include <limits>
#include <string>
#include <vector>

#include "core/errors.h"
#include "core/errorset.h"
#include "core/options.h"
#include "core/format.h"
#include "core/physics.h"
#include "core/text.h"
#include "avas_engine.h"
#include "core/device.h"
#include "core/dllmath.h"
#include "core/mesh_kernels.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iterator>

static int failures = 0;
#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
            ++failures;                                                   \
        }                                                                 \
    } while (0)

using namespace avas;

static void test_split() {
    auto w = split_words("particlenumber 1000!comment");
    CHECK(w.size() == 1 && w[0] == "particlenumber");  // '!' swallows the touching word
    w = split_words("  field\t0.21   0.02 !x");
    CHECK(w.size() == 3 && w[2] == "0.02");
    w = split_words("twissx 0,1 2");
    CHECK(w.size() == 3 && w[1] == "0,1");
    CHECK(split_words("   ").empty());
    CHECK(normalize_line_endings("a\r\nb\rc\n") == "a\nb\nc\n");
    auto lines = split_lines("a\n\nb");
    CHECK(lines.size() == 3 && lines[2] == "b");
}

static void test_numbers() {
    CHECK(parse_int("3.7") == 3);
    CHECK(parse_int("1e3") == 1);
    CHECK(parse_double("162.5e6") == 162.5e6);
    bool threw = false;
    try { parse_int("abc"); } catch (const NumberError&) { threw = true; }
    CHECK(threw);
}

static void test_format() {
    CHECK(fmt_g(1.5270289036445652) == "1.52703");
    CHECK(fmt_g(-1.33776e-05) == "-1.33776e-05");
    CHECK(fmt_g(0.20999999999999999, 17) == "0.20999999999999999");
    CHECK(fmt_fixed(2.9268783910574518e-09, 13) == "0.0000000029269");
    CHECK(fmt_g(std::numeric_limits<double>::infinity()) == "inf");
    const double nan = std::numeric_limits<double>::quiet_NaN();
    CHECK(fmt_g(nan) == "nan");
    CHECK(fmt_g(-nan) == "-nan(ind)");
    CHECK(fmt_g(0.0 / std::numeric_limits<double>::infinity()) == "0");
}

static void test_physics() {
    // pure drift: Boris push with zero fields moves the particle by v*dt
    double x = 0, ux = 0, y = 0, uy = 0, z = 0, uz = 0.0570756;
    const double E[3] = {0, 0, 0}, B[3] = {0, 0, 0};
    boris_push(x, ux, y, uy, z, uz, E, B, 9.57883e7, 1e-10);
    const double g = std::sqrt(1 + uz * uz);
    CHECK(std::fabs(z - k::c / g * uz * 1e-10) < 1e-15);
    // uniform Bz: |u| conserved
    double c[6] = {0, 0.01, 0, 0, 0, 0.05};
    const double Bz[3] = {0, 0, 1.0};
    double u0 = std::hypot(c[1], c[5]);
    double x2 = 0, y2 = 0, z2 = 0, ux2 = c[1], uy2 = 0, uz2 = c[5];
    for (int i = 0; i < 1000; ++i) boris_push(x2, ux2, y2, uy2, z2, uz2, E, Bz, 9.57883e7, 1e-11);
    CHECK(std::fabs(std::sqrt(ux2 * ux2 + uy2 * uy2 + uz2 * uz2) - u0) < 1e-14);
    // longitudinal synchronous push only uses Ez
    double sc[6] = {1, 1, 1, 1, 0, 0.05};
    const double Ez[3] = {5, 5, 1e6};
    sync_push(sc, Ez, B, 9.57883e7, 1e-11, false);
    CHECK(sc[0] == 0 && sc[1] == 0 && sc[5] > 0.05);
}

static void test_random() {
    // MSVC CRT rand(): srand(1) -> 41, 18467, 6334
    MsvcRand m;
    m.srand(1);
    CHECK(m.rand() == 41 && m.rand() == 18467 && m.rand() == 6334);
    // Philox4x32-10 known-answer tests (Random123)
    Philox4x32 ph{{0u, 0u}};
    const std::uint32_t c0[4] = {0, 0, 0, 0};
    std::uint32_t o[4];
    ph.block(c0, o);
    CHECK(o[0] == 0x6627e8d5u && o[1] == 0xe169c58du && o[2] == 0xbc57ac4cu && o[3] == 0x9b00dbd8u);
    Philox4x32 pp{{0xa4093822u, 0x299f31d0u}};
    const std::uint32_t cp[4] = {0x243f6a88u, 0x85a308d3u, 0x13198a2eu, 0x03707344u};
    pp.block(cp, o);
    CHECK(o[0] == 0xd16cfe09u && o[1] == 0x94fdccebu && o[2] == 0x5001e420u && o[3] == 0x24126ea1u);
    // portable log / sin / cos close to libm
    double worst = 0.0;
    for (int i = 1; i < 200000; ++i) {
        const double x = std::exp(-40.0 + 80.0 * i / 200000.0) * (1.0 + 1e-3 * (i % 7));
        const double r = std::fabs(p_log(x) - std::log(x)) / std::max(1.0, std::fabs(std::log(x)));
        worst = std::max(worst, r);
    }
    CHECK(worst < 4e-16);
    double ws = 0.0;
    for (int i = 0; i < 200000; ++i) {
        const double u = (i + 0.37) / 200000.0;
        double s, c;
        p_sincos_turns(u, s, c);
        ws = std::max(ws, std::max(std::fabs(s - std::sin(6.283185307179586 * u)),
                                   std::fabs(c - std::cos(6.283185307179586 * u))));
    }
    CHECK(ws < 2e-15);
    for (int i = 0; i <= 100; ++i) {
        const double u = i / 100.0;
        const double sv = p_parabolic_inv(u);
        CHECK(std::fabs(sv * sv * (3 - 2 * sv) - u) < 1e-14);
    }
    double u[2];
    ph.uniform2(7, 1, 0, u);
    CHECK(u[0] > 0.0 && u[0] < 1.0 && u[1] > 0.0 && u[1] < 1.0);
}

// Uniformly charged ellipsoid: the field inside is linear, E_i = kappa_i x_i with
// kappa_i = Q/(4 pi eps0) * 3/2 * Int_0^inf ds / ((a_i^2+s) sqrt((a^2+s)(b^2+s)(c^2+s))).
static double ellipsoid_kappa(double Q, const double ax[3], int i) {
    // s = t/(1-t), t in [0, 1): midpoint rule on a fine grid
    double sum = 0.0;
    const int n = 400000;
    for (int m = 0; m < n; ++m) {
        const double L2 = ax[0] * ax[0];  // scale of s
        const double t = (m + 0.5) / n, sv = L2 * t / (1.0 - t), ds = L2 / ((1.0 - t) * (1.0 - t)) / n;
        sum += ds / ((ax[i] * ax[i] + sv) * std::sqrt((ax[0] * ax[0] + sv) * (ax[1] * ax[1] + sv) * (ax[2] * ax[2] + sv)));
    }
    return Q * 8987551787.368176 * 1.5 * sum;
}

static void test_space_charge_ellipsoid() {
    const double ax[3] = {1.0e-3, 1.5e-3, 3.0e-3};
    const double Q = 1e-11;
    const int n = 200000;
    for (int method = 0; method < 2; ++method) {
        Params p;
        p.spaceCharge = 1;
        p.scPar[0] = method;
        p.multithreading = 1;
        space_charge_defaults(p, n);
        if (method == 0) p.numOfGrid = {64, 64, 64};
        Beam b;
        b.resize(n);
        Philox4x32 ph{{7u, 0u}};
        int k = 0;
        for (std::uint32_t i = 0; k < n; ++i) {
            double u[4];
            ph.uniform2(i, 9, 0, u);
            ph.uniform2(i, 9, 1, u + 2);
            const double x = 2 * u[0] - 1, y = 2 * u[1] - 1, z = 2 * u[2] - 1;
            if (x * x + y * y + z * z > 1.0) continue;
            b.rel[0][k] = x * ax[0];
            b.rel[2][k] = y * ax[1];
            b.rel[4][k] = z * ax[2];
            b.charge[k] = Q / n;
            ++k;
        }
        b.gamma = 1.0;
        b.centroid = {0, 0, 0, 0, 0, 0};
        b.rms = {ax[0] / std::sqrt(5.0), ax[1] / std::sqrt(5.0), ax[2] / std::sqrt(5.0)};
        SpaceCharge sc(p, n);
        sc.solve(b, 1.0);
        const std::vector<double>* f[3] = {&b.scx, &b.scy, &b.scz};
        const std::vector<double>* q[3] = {&b.rel[0], &b.rel[2], &b.rel[4]};
        for (int a = 0; a < 3; ++a) {
            double sxy = 0, sxx = 0;
            for (int i = 0; i < n; ++i) {
                const double r2 = std::pow(b.rel[0][i] / ax[0], 2) + std::pow(b.rel[2][i] / ax[1], 2) + std::pow(b.rel[4][i] / ax[2], 2);
                if (r2 > 0.5) continue;
                sxy += (*f[a])[i] * (*q[a])[i];
                sxx += (*q[a])[i] * (*q[a])[i];
            }
            const double slope = sxy / sxx, exact = ellipsoid_kappa(Q, ax, a);
            std::printf("ellipsoid %s axis %d: slope/analytic = %.4f\n", method ? "SPICNIC" : "FFT", a, slope / exact);
            // FFT: grounded walls in x, y and periodic images in z (period 2 meshrms rms_z) bias the
            // field by a few percent; SPICNIC is an open-boundary solver
            CHECK(std::fabs(slope / exact - 1.0) < (method == 0 ? 0.10 : 0.03));
        }
    }
}

// Error sampling (spec 70 3): the corrected distributions (uniform on [-A, A), Gaussian with sigma A,
// untruncated), their independence from the sample order, and the AVAS.dll rand() sequence.
static Lattice error_lattice(int a, double r, int mode) {
    Lattice lat;
    Element q;
    q.kind = ElemKind::Quad;
    q.typeName = "quad";
    lat.elements.push_back(q);
    ErrorParameter& er = lat.err;
    er.a = a;
    er.b = 1;
    std::vector<double> d{0.0, static_cast<double>(mode), 0.0, 1.0, r, 2.0, 0.0, 0.0, 0.0, 0.0, 5.0};
    d.resize(20, 0.0);
    er.elemDefs.push_back(d);
    std::vector<int> sw{0, mode, 1, 0, 0, 0, 0, 1};
    sw.resize(20, 0);
    er.elemSwitches.push_back(sw);
    er.nElements = 1;
    er.flags[mode ? 3 : 2] = 1;
    return lat;
}

// F-30: the FFT normalisation denominator overflows 32-bit int in AVAS.dll for large meshes
static void test_fft_norm() {
    CHECK(fft_norm(64, 64, 64, false) == 1.0 / (260.0 * 65.0 * 64.0));
    CHECK(fft_norm(64, 64, 64, true) == fft_norm(64, 64, 64, false));
    const double big = fft_norm(1024, 1024, 1024, false);
    CHECK(big == 1.0 / (4100.0 * 1025.0 * 1024.0) && big > 0.0);
    // (4*1024+4)*1025*1024 = 4303360000 wraps to 8392704 in 32-bit int
    CHECK(fft_norm(1024, 1024, 1024, true) == 1.0 / 8392704.0);
}

static void test_error_sampling() {
    const Compat saved = options().compat;
    options().compat = Compat::None;
    const int n = 200000;
    for (double r : {1.0, 2.0}) {
        MsvcRand unused;
        const Lattice lat = error_lattice(n, r, 1);
        const std::vector<ErrorSample> S = generate_error_set(lat, unused, 12345);
        double s1 = 0.0, s2 = 0.0, mx = 0.0, s1g = 0.0, cross = 0.0;
        int beyond3 = 0;
        for (const ErrorSample& e : S) {
            const double v = e.dyn[1][1], g = e.dyn[1][6];  // dx (A = 2), dG (A = 5)
            s1 += v; s2 += v * v; s1g += g; cross += v * g;
            mx = std::max(mx, std::fabs(v));
            if (std::fabs(v) > 3.0 * 2.0) ++beyond3;
        }
        const double mean = s1 / n, var = s2 / n - mean * mean;
        const double want = r == 1.0 ? 4.0 / 3.0 : 4.0;  // A^2/3 or A^2
        CHECK(std::fabs(mean) < 5.0 * std::sqrt(want / n));
        CHECK(std::fabs(var / want - 1.0) < 0.02);
        CHECK(std::fabs(cross / n) < 5.0 * std::sqrt(want * (r == 1.0 ? 25.0 / 3.0 : 25.0) / n));  // columns independent
        if (r == 1.0) CHECK(mx < 2.0);
        else CHECK(beyond3 > 0 && beyond3 < n * 0.006);  // not truncated: about 0.27 % beyond 3 sigma
        (void)s1g;
        // the same seed gives the same samples, another seed others
        const std::vector<ErrorSample> S2 = generate_error_set(lat, unused, 12345);
        const std::vector<ErrorSample> S3 = generate_error_set(lat, unused, 12346);
        CHECK(S2[777].dyn[1][1] == S[777].dyn[1][1] && S3[777].dyn[1][1] != S[777].dyn[1][1]);
    }
    // static r = 0: the fixed value; dynamic r = 0: equal steps x (j + 1) / b
    {
        MsvcRand unused;
        Lattice lat = error_lattice(1, 0.0, 1);
        lat.err.b = 4;
        const std::vector<ErrorSample> S = generate_error_set(lat, unused, 1);
        CHECK(S.size() == 4 && S[0].dyn[1][1] == 0.5 && S[3].dyn[1][1] == 2.0 && S[3].dyn[1][6] == 5.0);
        Lattice ls = error_lattice(1, 0.0, 0);
        ls.elements[0].kind = ElemKind::Field;  // static errors only reach static magnetic maps
        ls.elements[0].typeName = "field";
        ls.elements[0].words = {"field", "0.1", "0.02", "0", "3"};
        ls.err.b = 4;
        const std::vector<ErrorSample> T = generate_error_set(ls, unused, 1);
        CHECK(T[0].stat[1][1] == 2.0 && T[3].stat[1][1] == 2.0);
    }
    // AVAS.dll: dynamic uniform d = (u - 0.5) A, value d + d, u = (rand() % 10000) / 10000
    options().compat = Compat::Dll;
    {
        MsvcRand rng;  // state 1, as in a new process
        const Lattice lat = error_lattice(2, 1.0, 1);
        const std::vector<ErrorSample> S = generate_error_set(lat, rng, 0);
        const double d0 = (41.0 * 0.0001 - 0.5) * 2.0, d1 = (static_cast<double>(18467 % 10000) * 0.0001 - 0.5) * 2.0;
        CHECK(S[0].dyn[1][1] == d0 + d0 && S[1].dyn[1][1] == d1 + d1);
    }
    options().compat = saved;
}

// Portable elementary functions (pmath.h) against the C library: |error| <= 1 ulp of the libm value.
static double ulp_err(double a, double ref) {
    if (a == ref) return 0.0;
    const double u = std::nextafter(std::fabs(ref), 1e308) - std::fabs(ref);
    return std::fabs(a - ref) / u;
}

static void test_pmath() {
    std::uint64_t st = 88172645463325252ull;
    auto rnd = [&]() {
        st ^= st << 13;
        st ^= st >> 7;
        st ^= st << 17;
        return static_cast<double>(st >> 11) * (1.0 / 9007199254740992.0);
    };
    double es = 0, ec = 0, ee = 0, esh = 0, ech = 0, ea = 0, el = 0;
    for (int i = 0; i < 400000; ++i) {
        const double sc = i % 4 == 0 ? 1.0 : i % 4 == 1 ? 10.0 : i % 4 == 2 ? 1000.0 : 1e5;
        const double x = (rnd() * 2.0 - 1.0) * sc;
        double s, c;
        p_sincos(x, s, c);
        es = std::max(es, ulp_err(s, std::sin(x)));
        ec = std::max(ec, ulp_err(c, std::cos(x)));
        const double y = (rnd() * 2.0 - 1.0) * (i % 3 == 0 ? 0.5 : i % 3 == 1 ? 5.0 : 50.0);
        ee = std::max(ee, ulp_err(p_exp(y), std::exp(y)));
        esh = std::max(esh, ulp_err(p_sinh(y), std::sinh(y)));
        ech = std::max(ech, ulp_err(p_cosh(y), std::cosh(y)));
        const double z = (rnd() * 2.0 - 1.0) * (i % 3 == 0 ? 1.0 : i % 3 == 1 ? 10.0 : 1e6);
        ea = std::max(ea, ulp_err(p_atan(z), std::atan(z)));
        const double w = std::exp((rnd() * 2.0 - 1.0) * 40.0);
        el = std::max(el, ulp_err(p_log(w), std::log(w)));
    }
    std::printf("pmath max error (ulp): sin %.2f cos %.2f exp %.2f sinh %.2f cosh %.2f atan %.2f log %.2f\n", es, ec, ee, esh,
                ech, ea, el);
    CHECK(es <= 1.0 && ec <= 1.0 && ee <= 1.0 && ea <= 1.0 && el <= 1.0);
    CHECK(p_log(1.0) == 0.0 && p_log(2.0) == 0.6931471805599453 && p_log(0.0) == 0.0);
    CHECK(esh <= 2.0 && ech <= 2.0);
    CHECK(p_sin(0.0) == 0.0 && p_cos(0.0) == 1.0 && p_exp(0.0) == 1.0 && p_atan(0.0) == 0.0);
}

// PICNIC (direct sum over the whole mesh) against SPICNIC without threshold (octant sum plus the
// asymmetric correction): the same Green table, so the fields agree to rounding.
static void test_picnic() {
    auto field = [](int method, std::vector<double>& out) {
        Params p;
        p.spaceCharge = 1;
        p.scPar[0] = method;
        p.scPar[1] = p.scPar[2] = p.scPar[3] = 3.0;
        p.scPar[8] = 0.0;  // negligibleratio 0: SPICNIC keeps every asymmetric node
        p.numOfGrid = {12, 10, 14};
        space_charge_defaults(p, 3000);
        Beam b;
        b.resize(3000);
        std::uint64_t st = 12345;
        auto rnd = [&]() {
            st ^= st << 13;
            st ^= st >> 7;
            st ^= st << 17;
            return static_cast<double>(st >> 11) * (1.0 / 9007199254740992.0) - 0.5;
        };
        for (int i = 0; i < b.n; ++i) {
            b.rel[0][i] = 1e-3 * (rnd() + 0.3 * rnd());
            b.rel[2][i] = 2e-3 * rnd();
            b.rel[4][i] = 3e-3 * (rnd() + rnd());
            b.charge[i] = 1e-15;
        }
        b.gamma = 1.05;
        b.centroid = {0, 0, 0, 0, 0, 0};
        b.rms = {0.35e-3, 0.6e-3, 1.3e-3};
        SpaceCharge sc(p, b.n);
        sc.solve(b, 1.0);
        out.clear();
        for (int i = 0; i < b.n; ++i) {
            out.push_back(b.scx[i]);
            out.push_back(b.scy[i]);
            out.push_back(b.scz[i]);
        }
    };
    std::vector<double> a, c;
    field(2, a);
    field(1, c);
    double num = 0, den = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        num = std::max(num, std::fabs(a[i] - c[i]));
        den = std::max(den, std::fabs(c[i]));
    }
    std::printf("PICNIC against SPICNIC (negligibleratio 0): max |dE| / max |E| = %.2e\n", num / den);
    CHECK(den > 0 && num / den < 1e-12);
}

// Fixed-order sums (reduce.h) and the fixed-point deposit: independent of the grouping.
static void test_reduce() {
    std::vector<double> v(10000);
    for (size_t i = 0; i < v.size(); ++i) v[i] = std::sin(0.37 * static_cast<double>(i)) * std::pow(10.0, static_cast<double>(i % 7) - 3.0);
    const int n = static_cast<int>(v.size());
    double a[1], b[1];
    auto serial = [](int ng, const std::function<void(int)>& fn) { for (int g = 0; g < ng; ++g) fn(g); };
    auto reverse = [](int ng, const std::function<void(int)>& fn) { for (int g = ng - 1; g >= 0; --g) fn(g); };
    fixed_sum<1>(n, serial, [&](int i, double* c) { c[0] = v[static_cast<size_t>(i)]; }, a);
    fixed_sum<1>(n, reverse, [&](int i, double* c) { c[0] = v[static_cast<size_t>(i)]; }, b);
    CHECK(a[0] == b[0]);
    // explicit tree of the first chunk
    double t[256];
    for (int i = 0; i < 256; ++i) t[i] = v[static_cast<size_t>(i)];
    for (int s2 = 128; s2 >= 1; s2 >>= 1)
        for (int i = 0; i < s2; ++i) t[i] = t[i] + t[i + s2];
    double g[1];
    group_sum<1>(0, 256, [&](int i, double* c) { c[0] = v[static_cast<size_t>(i)]; }, g);
    CHECK(g[0] == t[0]);
    CHECK(round_to_ll(2.5) == 2 && round_to_ll(3.5) == 4 && round_to_ll(-2.5) == -2 && round_to_ll(-2.6) == -3);
    long long s1 = 0, s2 = 0;
    for (int i = 0; i < n; ++i) s1 += kern::fix_charge(v[static_cast<size_t>(i)] * 1e-15, 80);
    for (int i = n - 1; i >= 0; --i) s2 += kern::fix_charge(v[static_cast<size_t>(i)] * 1e-15, 80);
    CHECK(s1 == s2);
}

// ---- C API queries (avas_engine_info / devices / last_run) ----

// Minimal JSON reader for the checks: parses one value, returns false on a syntax error.
struct Json {
    const char* p;
    bool ws() {
        while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r') ++p;
        return true;
    }
    bool str() {
        if (*p != '"') return false;
        for (++p; *p && *p != '"'; ++p)
            if (*p == '\\' && !*++p) return false;
        if (*p != '"') return false;
        ++p;
        return true;
    }
    bool num() {
        const char* s = p;
        if (*p == '-') ++p;
        while ((*p >= '0' && *p <= '9') || *p == '.' || *p == 'e' || *p == 'E' || *p == '+' || *p == '-') ++p;
        return p > s;
    }
    bool lit(const char* w) {
        const size_t n = std::strlen(w);
        if (std::strncmp(p, w, n) != 0) return false;
        p += n;
        return true;
    }
    bool value() {
        ws();
        if (*p == '{') {
            ++p;
            ws();
            if (*p == '}') return ++p, true;
            for (;;) {
                ws();
                if (!str()) return false;
                ws();
                if (*p++ != ':') return false;
                if (!value()) return false;
                ws();
                if (*p == ',') { ++p; continue; }
                if (*p == '}') return ++p, true;
                return false;
            }
        }
        if (*p == '[') {
            ++p;
            ws();
            if (*p == ']') return ++p, true;
            for (;;) {
                if (!value()) return false;
                ws();
                if (*p == ',') { ++p; continue; }
                if (*p == ']') return ++p, true;
                return false;
            }
        }
        if (*p == '"') return str();
        if (*p == 't') return lit("true");
        if (*p == 'f') return lit("false");
        if (*p == 'n') return lit("null");
        return num();
    }
};

static bool valid_json(const char* s) {
    Json j{s};
    return j.value() && j.ws() && *j.p == '\0';
}

static bool has(const char* json, const char* key) { return std::strstr(json, key) != nullptr; }

static void write_text(const std::filesystem::path& f, const char* text) {
    std::ofstream o(f, std::ios::binary);
    o << text;
}

static void test_capi_queries() {
    const char* v = avas_engine_version();
    CHECK(std::strlen(v) == 16);
    const char* info = avas_engine_info();
    CHECK(valid_json(info));
    for (const char* k : {"\"version\"", "\"source_hash\"", "\"build\"", "\"cuda\"", "\"metal\"", "\"mpi\"",
                          "\"cuda_archs\"", "\"compiler\"", "\"platform\""})
        CHECK(has(info, k));
    CHECK(has(info, (std::string("\"version\":\"") + v + "\"").c_str()));

    const char* last = avas_engine_last_run();
    CHECK(valid_json(last));
    CHECK(has(last, "\"ran\":false") && has(last, "\"device\":null") && has(last, "\"seconds\":null"));

    CHECK(avas_set_option("device", "cpu") == 0);
    std::string dev = avas_engine_devices();
    CHECK(valid_json(dev.c_str()));
    for (const char* k : {"\"cuda\"", "\"metal\"", "\"cpu\"", "\"threads\"", "\"options\"", "\"auto\"", "\"selected\"",
                          "\"sc\"", "\"nosc\"", "\"devices\""})
        CHECK(has(dev.c_str(), k));
    CHECK(has(dev.c_str(), "\"selected\":{\"sc\":{\"device\":\"cpu\",\"precision\":\"fp64\""));
    // a required device this build or machine cannot give: an error in the plan, not the CPU
    CHECK(avas_set_option("device", "cuda") == 0);
    if (!cuda_compiled()) {
        dev = avas_engine_devices();
        CHECK(valid_json(dev.c_str()));
        CHECK(has(dev.c_str(), "\"selected\":{\"sc\":{\"device\":null"));
        CHECK(has(dev.c_str(), "built without CUDA"));
    }
    CHECK(avas_set_option("device", "metal") == 0);
    if (!metal_compiled()) {
        dev = avas_engine_devices();
        CHECK(has(dev.c_str(), "\"selected\":{\"sc\":{\"device\":null"));
    }
    // compat=dll: auto is the CPU
    CHECK(avas_set_option("compat", "dll") == 0);
    dev = avas_engine_devices();
    CHECK(has(dev.c_str(), "\"auto\":{\"sc\":{\"device\":\"cpu\""));
    CHECK(avas_set_option("compat", "none") == 0);
    CHECK(avas_set_option("device", "cpu") == 0);
    CHECK(avas_set_option("threads", "2") == 0);

    // one small run: last_run reports it
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / ("avas_unit_capi_" + std::to_string(std::rand()));
    fs::create_directories(dir / "out");
    write_text(dir / "input.txt", "sim_type mulp\nspacecharge 1\nsteppercycle 20\ndumpperiodicity 0\nboundary 0\n"
                                  "randomseed 7\nscmethod FFT\n");
    write_text(dir / "beam.txt", "readparticledistribution unknown\nnumofcharge 1\nparticlerestmass 938.272088159928\n"
                                 "current 0.1841\nparticlenumber 300\nfrequency 162500000.0\nkneticenergy 1.5270289036445652\n"
                                 "use_dst 0\nbeamtype notdc\ntwissx -0.65785 0.37743 0.13768\n"
                                 "twissy -0.6341 0.37292 0.14059\ntwissz 0.04275 0.37292 0.26506\ndistribution GS GS\n");
    write_text(dir / "lattice.txt", "start\ndrift 0.02 0.02 0\nend\n");
#if defined(_WIN32)
    const std::wstring in = dir.wstring(), out = (dir / "out").wstring();
#else
    const std::string in = dir.string(), out = (dir / "out").string();
#endif
    path(in.c_str(), out.c_str(), in.c_str());
    int state = 1;
    const int rc = main_agent(&state);
    CHECK(rc == 0);
    last = avas_engine_last_run();
    CHECK(valid_json(last));
    CHECK(has(last, "\"ran\":true") && has(last, "\"device\":\"cpu\"") && has(last, "\"precision\":\"fp64\""));
    CHECK(has(last, "\"compat\":\"none\"") && has(last, "\"threads\":2") && has(last, "\"space_charge\":true"));
    CHECK(has(last, "\"status\":0") && !has(last, "\"seconds\":null"));
    // a failing run (no lattice file) is recorded too, without a device
    fs::remove(dir / "lattice.txt");
    state = 1;
    CHECK(main_agent(&state) != 0);
    last = avas_engine_last_run();
    CHECK(has(last, "\"ran\":true") && has(last, "\"device\":null") && !has(last, "\"status\":0"));
    CHECK(avas_set_option("threads", "0") == 0);
    std::error_code ec;
    fs::remove_all(dir, ec);
}

// rfq (corrected engine): the field is -grad of the 8-term potential (numerical gradient), zero past the
// cells, the particle lost outside the cell aperture.
static void test_rfq_field() {
    using namespace avas;
    const double f = 162.5e6, V = 70e3, t = 1.1e-9, phi = 0.4;
    std::vector<double> cells = {
        0.0,   0.012, 0.012, 0.006, V, 1.0, 0.95, 0.08, 0.01, 0.03, 0.02, 0.005, 0.04, 0.015,
        0.012, 0.025, 0.013, 0.005, V, 1.0, 0.95, 0.09, 0.01, 0.03, 0.02, 0.005, 0.04, 0.015};
    kern::ElemV e{};
    e.kind = kern::kRfq;
    e.zStart = 0.1;
    e.length = 0.03;
    e.radius = 0.01;
    e.rfPlain = e.rfErr = 0;
    e.rfq0 = 0;
    e.rfqN = 2;
    kern::RfSlotV slot{0, 0, f, 0.0};
    double cavTime[1] = {0.0}, cavPhase[1] = {phi * 180.0 / 3.141592653589793};
    double zs[1] = {0.1}, ze[1] = {0.13};
    kern::LatV L{};
    L.elems = &e;
    L.rfSlots = &slot;
    L.cavTime = cavTime;
    L.cavPhase = cavPhase;
    L.elemZStart = zs;
    L.elemZEnd = ze;
    L.nElem = 1;
    L.totalLength = L.spExit = 0.2;
    L.rfqCells = cells.data();
    const kern::RfSrc rf{nullptr, t};
    auto field = [&](double x, double y, double z, double* E, double* B) {
        kern::FieldIn<double> in{};
        in.x = x;
        in.y = y;
        in.fz = z;
        in.spZ0 = -1e9;
        in.spZ1 = 1e9;
        return kern::element_field_t<PortableMath>(L, kern::AbsZ{&L}, 0, e, in, rf, E, B);
    };
    auto potential = [&](double x, double y, double z) {
        const double zl = z - 0.1;
        const int i = zl <= 0.012 ? 0 : 1;
        const double* C = cells.data() + 14 * i;
        const double kk = 3.141592653589793 / C[2], zeta = zl - C[0], sg = i == 0 ? 1.0 : -1.0, a = C[3];
        const double r = std::sqrt(x * x + y * y), th = std::atan2(y, x);
        auto I = [](int n, double v) { return kern::rfq_bessel_i(n, v); };
        const double u = C[6] * std::pow(r / a, 2) * std::cos(2 * th) + C[8] * std::pow(r / a, 6) * std::cos(6 * th) +
                         sg * (C[7] * I(0, kk * r) * std::sin(kk * zeta) + C[12] * I(0, 3 * kk * r) * std::sin(3 * kk * zeta) +
                               C[9] * I(4, kk * r) * std::cos(4 * th) * std::sin(kk * zeta) +
                               C[13] * I(4, 3 * kk * r) * std::cos(4 * th) * std::sin(3 * kk * zeta) +
                               C[10] * I(2, 2 * kk * r) * std::cos(2 * th) * std::sin(2 * kk * zeta) +
                               C[11] * I(6, 2 * kk * r) * std::cos(6 * th) * std::sin(2 * kk * zeta));
        return V * 0.5 * u * std::cos(2 * 3.141592653589793 * f * t + phi);
    };
    const double pts[3][3] = {{0.0021, -0.0013, 0.1043}, {-0.0035, 0.0024, 0.1191}, {0.0004, 0.0047, 0.1245}};
    for (const auto& q : pts) {
        double E[3], B[3];
        CHECK(field(q[0], q[1], q[2], E, B) == kern::kFieldWrite);
        const double h = 1e-7;
        const double num[3] = {-(potential(q[0] + h, q[1], q[2]) - potential(q[0] - h, q[1], q[2])) / (2 * h),
                               -(potential(q[0], q[1] + h, q[2]) - potential(q[0], q[1] - h, q[2])) / (2 * h),
                               -(potential(q[0], q[1], q[2] + h) - potential(q[0], q[1], q[2] - h)) / (2 * h)};
        const double scale = std::fabs(num[0]) + std::fabs(num[1]) + std::fabs(num[2]);
        for (int j = 0; j < 3; ++j) CHECK(std::fabs(E[j] - num[j]) <= 1e-6 * scale);
        CHECK(B[0] == 0.0 && B[1] == 0.0 && B[2] == 0.0);
    }
    double E[3], B[3];
    CHECK(field(0.0, 0.0, 0.128, E, B) == kern::kFieldWrite && E[0] == 0.0 && E[2] == 0.0);  // past the cells
    CHECK(field(0.0055, 0.0, 0.11, E, B) == kern::kFieldWrite);                       // cell 0: inside a = 6 mm
    CHECK(field(0.0055, 0.0, 0.12, E, B) == (kern::kFieldWrite | kern::kFieldLost));  // cell 1: outside a = 5 mm
}

// ---- M10 (2026-07-22 original features in the corrected engine) ----

// boundary.txt shapes 3 / 4 / composite 0 and the "whole short pipe in one step" check (D5)
static void test_boundary_shapes() {
    using namespace avas;
    auto geom = [](int shape, double L, double a, double b, double dx, double dy, double thz) {
        kern::GeomV g{};
        g.shape = shape;
        g.npar = shape == 1 ? 2 : 3;
        g.zStart = 0.1;
        g.par[0] = L;
        g.par[1] = a;
        g.par[2] = b;
        g.place[1] = dx;
        g.place[2] = dy;
        g.place[3] = thz;
        g.s3 = std::sin(thz);
        g.c3 = std::cos(thz);
        g.s4 = g.s5 = 0.0;
        g.c4 = g.c5 = 1.0;
        return g;
    };
    // one particle: old -> new
    double x[1], ux[1], y[1], uy[1], z[1], uz[1], old[6];
    kern::PartV P{};
    P.x = x; P.ux = ux; P.y = y; P.uy = uy; P.z = z; P.uz = uz; P.old = old;
    auto set = [&](double x0, double y0, double z0, double x1, double y1, double z1) {
        old[0] = x0; old[1] = 0.01; old[2] = y0; old[3] = 0.0; old[4] = z0; old[5] = 0.06;
        x[0] = x1; ux[0] = 0.01; y[0] = y1; uy[0] = 0.0; z[0] = z1; uz[0] = 0.06;
    };
    double Pp[6], tHit = 0.0;
    // rectangular pipe 3: half width 4 mm, half height 2 mm
    kern::GeomV r = geom(3, 0.2, 0.004, 0.002, 0.0, 0.0, 0.0);
    set(0.0035, 0.0015, 0.15, 0.0035, 0.0015, 0.16);
    CHECK(kern::geom_hit_one<PortableMath>(r, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 0);
    set(0.0035, 0.0015, 0.15, 0.0035, 0.0025, 0.16);  // leaves through the top
    CHECK(kern::geom_hit_one<PortableMath>(r, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 1);
    CHECK(std::fabs(Pp[2] - 0.002) < 1e-12 && std::fabs(Pp[4] - 0.155) < 1e-9);  // on the wall, half way
    CHECK(std::fabs(tHit - (1.0 - 0.5e-9)) < 1e-15);
    // elliptical pipe 4: semi-axes 4 mm, 2 mm
    kern::GeomV e = geom(4, 0.2, 0.004, 0.002, 0.0, 0.0, 0.0);
    set(0.0035, 0.0, 0.15, 0.0035, 0.0, 0.16);
    CHECK(kern::geom_hit_one<PortableMath>(e, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 0);
    set(0.0035, 0.0012, 0.15, 0.0035, 0.0012, 0.16);  // (3.5/4)^2 + (1.2/2)^2 = 1.13
    CHECK(kern::geom_hit_one<PortableMath>(e, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 1);
    // a thin collimator (1 mm long) crossed in one step: hit in the default mode, not in compat
    kern::GeomV c = geom(1, 0.001, 0.003, 0.0, 0.0, 0.0, 0.0);
    set(0.004, 0.0, 0.0995, 0.004, 0.0, 0.1015);
    CHECK(kern::geom_hit_one<PortableMath>(c, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 1);
    CHECK(std::fabs(Pp[4] - 0.1) < 1e-12 && std::fabs(tHit - (1.0 - 0.75e-9)) < 1e-15);  // entrance face
    CHECK(kern::geom_hit_one<DllMath>(c, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 0);
    set(0.002, 0.0, 0.0995, 0.002, 0.0, 0.1015);
    CHECK(kern::geom_hit_one<PortableMath>(c, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 0);
    // the test is made in the local frame: a collimator displaced by dx = 3 mm and turned by 90 degrees
    kern::GeomV cr = geom(3, 0.001, 0.001, 0.004, 0.003, 0.0, 1.5707963267948966);
    set(0.0035, 0.0, 0.0995, 0.0035, 0.0, 0.1015);  // local u = 0, v = -0.5 mm: inside
    CHECK(kern::geom_hit_one<PortableMath>(cr, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 0);
    set(0.0035, 0.0015, 0.0995, 0.0035, 0.0015, 0.1015);  // local u = 1.5 mm > 1 mm: hit
    CHECK(kern::geom_hit_one<PortableMath>(cr, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 1);
    // ellipse collimator: y is tested with y (the 2026-07-22 original uses the x momentum)
    kern::GeomV ce = geom(4, 0.001, 0.004, 0.001, 0.0, 0.0, 0.0);
    set(0.0, 0.0015, 0.0995, 0.0, 0.0015, 0.1015);
    CHECK(kern::geom_hit_one<PortableMath>(ce, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 1);
    // row offset zOff: the geometry at z - zOff
    set(0.0035, 0.0015, 0.35, 0.0035, 0.0025, 0.36);
    CHECK(kern::geom_hit_one<PortableMath>(r, P, 0, 1.0, 1e-9, 0.1, Pp, tHit) == 1);
    CHECK(kern::geom_hit_one<PortableMath>(r, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 0);
    // composite: two overlapping sub-shapes, the aperture is the intersection
    kern::GeomV tab[3] = {kern::GeomV{}, geom(1, 0.2, 0.005, 0.0, 0.0, 0.0, 0.0), geom(3, 0.1, 0.006, 0.002, 0.0, 0.0, 0.0)};
    tab[0].shape = 0;
    tab[0].npar = 1;
    tab[0].par[0] = 0.2;
    tab[0].sub0 = 1;
    tab[0].nsub = 2;
    set(0.0, 0.0018, 0.15, 0.0, 0.0018, 0.16);  // inside both
    CHECK(kern::geometry_hit<PortableMath>(tab, 0, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 0);
    set(0.0, 0.0018, 0.15, 0.0, 0.0025, 0.16);  // inside the circle, outside the rectangle (z < 0.2)
    CHECK(kern::geometry_hit<PortableMath>(tab, 0, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 1);
    set(0.0, 0.0018, 0.25, 0.0, 0.0025, 0.26);  // past the rectangle: only the circle
    CHECK(kern::geometry_hit<PortableMath>(tab, 0, P, 0, 1.0, 1e-9, 0.0, Pp, tHit) == 0);
}

// lpboundary V1 1 (D7): the deposit cell is periodic in z, the last cell joins node nz-1 to node 0
static void test_periodic_mesh() {
    using namespace avas;
    const int nx = 4, ny = 4, nz = 8;
    std::vector<double> xs(nx), ys(ny), zs(nz);
    const double dx = 0.001, dz = 0.002;
    for (int i = 0; i < nx; ++i) xs[i] = (i - nx * 0.5 + 0.5) * dx;
    for (int j = 0; j < ny; ++j) ys[j] = (j - ny * 0.5 + 0.5) * dx;
    for (int k = 0; k < nz; ++k) zs[k] = (k - nz * 0.5 + 0.5) * dz;
    kern::MeshV M{};
    M.nx = nx; M.ny = ny; M.nz = nz;
    M.dx = M.dy = dx;
    M.dz = dz;
    M.vol = dx * dx * dz;
    M.gamma = M.igamma = 1.0;
    M.lpRel = 1;
    M.lpWrap = 1;
    M.lpLen = nz * dz;
    M.xs = xs.data(); M.ys = ys.data(); M.zs = zs.data();
    int i, j, k, i2, j2, k2;
    double w[8], w2[8];
    CHECK(kern::mesh_cell_t<double>(M, 0.0002, -0.0001, 0.0031, i, j, k, w));
    CHECK(kern::mesh_cell_t<double>(M, 0.0002, -0.0001, 0.0031 + 3 * M.lpLen, i2, j2, k2, w2));
    CHECK(i == i2 && j == j2 && k == k2);
    double sum = 0.0;
    for (int m = 0; m < 8; ++m) {
        sum += w[m];
        CHECK(std::fabs(w[m] - w2[m]) < 1e-9);
    }
    CHECK(std::fabs(sum - 1.0) < 1e-12);
    // beyond the last node: cell nz-1, its upper corners are node 0
    CHECK(kern::mesh_cell_t<double>(M, 0.0, 0.0, zs[nz - 1] + 0.25 * dz, i, j, k, w));
    CHECK(k == nz - 1);
    CHECK(kern::mesh_corner(M, i, j, k, 4) == kern::mesh_at(M, i, j, 0));
    CHECK(std::fabs(w[0] + w[1] + w[2] + w[3] - 0.75) < 1e-12);  // weight 3/4 on node nz-1
    M.lpWrap = 0;
    CHECK(!kern::mesh_cell_t<double>(M, 0.0, 0.0, zs[nz - 1] + 0.25 * dz, i, j, k, w));
}

// a small run in a temporary folder; returns the exit code and leaves the outputs in dir/out
static int run_small(const std::filesystem::path& dir, const std::string& beam, const std::string& lattice,
                     const std::string& input = "spacecharge 0\nsteppercycle 20\n") {
    namespace fs = std::filesystem;
    fs::remove_all(dir);
    fs::create_directories(dir / "out");
    write_text(dir / "input.txt", input.c_str());
    write_text(dir / "beam.txt", ("numofcharge 1\nparticlerestmass 938.272088159928\ncurrent 0\nfrequency 162500000.0\n"
                                  "distribution GS GS\ntwissx -0.65785 0.37743 0.13768\ntwissy -0.6341 0.37292 0.14059\n"
                                  "twissz 0.04275 0.37292 0.26506\n" + beam).c_str());
    write_text(dir / "lattice.txt", lattice.c_str());
#if defined(_WIN32)
    const std::wstring in = dir.wstring(), out = (dir / "out").wstring();
#else
    const std::string in = dir.string(), out = (dir / "out").string();
#endif
    path(in.c_str(), out.c_str(), in.c_str());
    int state = 1;
    return main_agent(&state);
}

static std::string slurp(const std::filesystem::path& f) {
    std::ifstream i(f, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(i), std::istreambuf_iterator<char>());
}

// initpos / initmom (D2): new units, the new names win; RF field with Ke = 0 is a drift (D6);
// SingleParticlePlane.txt (D3); reference energy of a .dst file (D1)
static void test_m10_runs() {
    namespace fs = std::filesystem;
    CHECK(avas_set_option("compat", "none") == 0);
    CHECK(avas_set_option("threads", "1") == 0);
    const fs::path dir = fs::temp_directory_path() / ("avas_unit_m10_" + std::to_string(std::rand()));
    const std::string drift = "start\ndrift 0.05 0.02 0\noutputplane -0.02 MID\nend\n";
    auto header = [&](const std::string& beam) {
        CHECK(run_small(dir, "particlenumber 1\nkneticenergy 1.5\n" + beam, drift) == 0);
        return slurp(dir / "out" / "SingleParticle.txt");
    };
    // initpos 1 2 0 mm, initmom 3 -4 mrad
    std::string h = header("initpos 1 2 0\ninitmom 3 -4 0\n");
    CHECK(h.find("dx:1mm  dy:2mm") != std::string::npos);
    CHECK(h.find("dpx:3mrad  dpy:-4mrad") != std::string::npos);
    // displacedpos keeps its unit (300 -> 3 mrad)
    h = header("displacepos 1 2 0\ndisplacedpos 300 -400 0\n");
    CHECK(h.find("dx:1mm  dy:2mm") != std::string::npos);
    CHECK(h.find("dpx:3mrad  dpy:-4mrad") != std::string::npos);
    // both: the new names win
    h = header("displacepos 5 5 0\ndisplacedpos 100 100 0\ninitpos 1 2 0\ninitmom 3 -4 0\n");
    CHECK(h.find("dx:1mm  dy:2mm") != std::string::npos && h.find("dpx:3mrad") != std::string::npos);
    // SingleParticlePlane.txt: one line per output plane (MID at 0.03, the end at 0.05), 7 columns
    {
        std::istringstream ls(slurp(dir / "out" / "SingleParticlePlane.txt"));
        std::string line;
        int rows = 0;
        while (std::getline(ls, line)) {
            std::istringstream ws(line);
            std::vector<double> v;
            double d;
            while (ws >> d) v.push_back(d);
            CHECK(v.size() == 7);
            if (v.size() == 7 && rows == 0) CHECK(std::fabs(v[0] - 0.03) < 1e-12 && std::fabs(v[1] - 1.5) < 1e-4);
            ++rows;
        }
        CHECK(rows == 2);
        CHECK(fs::exists(dir / "out" / "MID_0.030000.dst") && fs::exists(dir / "out" / "outData_0.050000.dst"));
    }
    // compat=dll: initpos unknown (particle on the axis), no SingleParticlePlane.txt
    CHECK(avas_set_option("compat", "dll") == 0);
    h = header("initpos 1 2 0\n");
    CHECK(h.find("dx:0mm  dy:0mm") != std::string::npos);
    CHECK(!fs::exists(dir / "out" / "SingleParticlePlane.txt"));
    CHECK(avas_set_option("compat", "none") == 0);
    // RF field with Ke = 0: a drift (no map is read), also in a superpose block; too few words: error
    const std::string beam1 = "particlenumber 1\nkneticenergy 1.5\n";
    CHECK(run_small(dir, beam1, "start\ndrift 0.05 0.02 0\ndrift 0.21 0.02 0\nend\n") == 0);
    const std::string ref = slurp(dir / "out" / "SingleParticle.txt");
    CHECK(run_small(dir, beam1, "start\ndrift 0.05 0.02 0\nfield 0.21 0.02 0 1 162.5e6 -30 0 0 nomap\nend\n") == 0);
    CHECK(slurp(dir / "out" / "SingleParticle.txt") == ref);
    CHECK(run_small(dir, beam1, "start\ndrift 0.05 0.02 0\nsuperpose 0\nfield 0.21 0.02 0 1 162.5e6 -30 0 0 nomap\n"
                                "superpose 0.01\ndrift 0.1 0.03 0\nsuperposeend\nend\n") == 0);
    CHECK(run_small(dir, beam1, "start\ndrift 0.05 0.02 0\nfield 0.21 0.02 0 1 162.5e6\nend\n") != 0);
    // .dst input: the synchronous particle starts with kneticenergy (D1), negative kneticenergy: the mean
    {
        const int n = 10;
        std::string dst;
        dst.push_back(static_cast<char>(0x7D));
        dst.push_back(static_cast<char>(0x64));
        const std::int32_t n32 = n;
        const double ib = 0.0, fm = 162.5, mc2 = 938.272088159928;
        dst.append(reinterpret_cast<const char*>(&n32), 4);
        dst.append(reinterpret_cast<const char*>(&ib), 8);
        dst.append(reinterpret_cast<const char*>(&fm), 8);
        dst.push_back(static_cast<char>(0x7D));
        for (int q = 0; q < n; ++q) {
            const double row[6] = {0.0, 0.0, 0.0, 0.0, -0.01 * q, 1.0 + 0.01 * q};
            dst.append(reinterpret_cast<const char*>(row), 48);
        }
        dst.append(reinterpret_cast<const char*>(&mc2), 8);
        for (const char* w : {"1.2", "-1"}) {
            fs::remove_all(dir);
            fs::create_directories(dir / "out");
            {
                std::ofstream o(dir / "p.dst", std::ios::binary);
                o << dst;
            }
            write_text(dir / "input.txt", "spacecharge 0\nsteppercycle 20\n");
            write_text(dir / "beam.txt", (std::string("readparticledistribution p.dst\nnumofcharge 1\nkneticenergy ") + w + "\n").c_str());
            write_text(dir / "lattice.txt", "start\ndrift 0.05 0.02 0\nend\n");
#if defined(_WIN32)
            const std::wstring in = dir.wstring(), out = (dir / "out").wstring();
#else
            const std::string in = dir.string(), out = (dir / "out").string();
#endif
            path(in.c_str(), out.c_str(), in.c_str());
            int state = 1;
            CHECK(main_agent(&state) == 0);
            std::istringstream ds(slurp(dir / "out" / "DataSet.txt"));
            std::string line;
            std::getline(ds, line);
            std::istringstream ws(line);
            std::vector<std::string> v;
            std::string tok;
            while (ws >> tok) v.push_back(tok);  // (rms columns may be nan with zero emittance)
            CHECK(v.size() == 41);
            if (v.size() == 41) {
                const double u = std::strtod(v[34].c_str(), nullptr);  // synchronous gamma*beta_z
                const double W = (std::sqrt(1.0 + u * u) - 1.0) * mc2;
                CHECK(std::fabs(W - (w[0] == '1' ? 1.2 : 1.045)) < 1e-4);  // DataSet: 6 digits
            }
        }
    }
    CHECK(avas_set_option("threads", "0") == 0);
    std::error_code ec;
    fs::remove_all(dir, ec);
}

int main() {
    test_split();
    test_numbers();
    test_format();
    test_physics();
    test_random();
    test_space_charge_ellipsoid();
    test_error_sampling();
    test_fft_norm();
    test_pmath();
    test_picnic();
    test_reduce();
    test_capi_queries();
    test_rfq_field();
    test_boundary_shapes();
    test_periodic_mesh();
    test_m10_runs();
    std::printf("%s (%d failures)\n", failures ? "FAILED" : "OK", failures);
    return failures;
}
