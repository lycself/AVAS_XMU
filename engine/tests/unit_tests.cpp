// Unit tests of the text, format and physics helpers (no framework; exit code = failures).
#include <cmath>
#include <cstdio>
#include <limits>
#include <string>
#include <vector>

#include "core/errors.h"
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

int main() {
    test_split();
    test_numbers();
    test_format();
    test_physics();
    std::printf("%s (%d failures)\n", failures ? "FAILED" : "OK", failures);
    return failures;
}
