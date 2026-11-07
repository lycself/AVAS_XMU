#include <algorithm>
// Unit tests of the text, format and physics helpers (no framework; exit code = failures).
#include "core/rng.h"
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

int main() {
    test_split();
    test_numbers();
    test_format();
    test_physics();
    test_random();
    test_space_charge_ellipsoid();
    test_error_sampling();
    test_fft_norm();
    std::printf("%s (%d failures)\n", failures ? "FAILED" : "OK", failures);
    return failures;
}
