#include "core/distribution.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "core/dllmath.h"
#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"

namespace avas {

namespace {

constexpr double kInvRandMax = 3.051850947599719e-05;     // 1/32767 as folded by the DLL
constexpr double kAngle = 0.00019175345033660654;         // 2 pi/32767
constexpr double kSqrt3 = 1.7320508075688772;

// ---- Eigen 3.3 SelfAdjointEigenSolver<MatrixXd>::eigenvalues() of a 2x2 matrix -----------------
// (scaling, tridiagonal form, implicit symmetric QR with Wilkinson shift, ascending sort);
// the operation order follows the AVAS.dll disassembly (FUN_180011ac0 / 1800120d0 / 180012920).
double eigen_hypot(double x, double y) {
    const double ax = std::fabs(x), ay = std::fabs(y);
    const double p = std::max(ax, ay);
    if (p == 0.0) return 0.0;
    const double q = std::min(ax, ay) / p;
    return std::sqrt(q * q + 1.0) * p;
}

void eigen_sym2(double m00, double m10, double m11, double ev[2]) {
    double scale = std::max(std::max(std::fabs(m00), std::fabs(m10)), std::fabs(m11));
    if (scale == 0.0) scale = 1.0;
    double d[2] = {m00 / scale, m11 / scale};
    double e = m10 / scale;
    int iter = 0;
    while (true) {
        if (std::fabs(e) <= (std::fabs(d[1]) + std::fabs(d[0])) * 4.440892098500626e-16 ||
            std::fabs(e) <= 2.2250738585072014e-308)
            e = 0.0;
        if (e == 0.0) break;
        if (++iter > 60) break;  // Eigen gives up after 30 n iterations
        // tridiagonal_qr_step(start 0, end 1)
        const double td = (d[0] - d[1]) * 0.5;
        double mu = d[1];
        if (td == 0.0) {
            mu -= std::fabs(e);
        } else {
            const double e2 = e * e;
            const double h = eigen_hypot(td, e);
            if (e2 == 0.0) mu -= (e / (td + (td > 0.0 ? 1.0 : -1.0))) * (e / h);
            else mu -= e2 / (td + (td > 0.0 ? h : -h));
        }
        const double x = d[0] - mu, z = e;
        double c, s;
        if (z == 0.0) {
            c = x < 0.0 ? -1.0 : 1.0;
            s = 0.0;
        } else if (x == 0.0) {
            c = 0.0;
            s = z < 0.0 ? 1.0 : -1.0;
        } else if (std::fabs(x) > std::fabs(z)) {
            const double t = z / x;
            double u = std::sqrt(t * t + 1.0);
            if (x < 0.0) u = -u;
            c = 1.0 / u;
            s = -(c * t);
        } else {
            const double t = x / z;
            double u = std::sqrt(t * t + 1.0);
            if (z < 0.0) u = -u;
            s = -1.0 / u;
            c = -(s * t);
        }
        const double sdk = d[0] * s + e * c;
        const double dkp1 = d[1] * c + e * s;
        const double n0 = (d[0] * c - e * s) * c - (e * c - d[1] * s) * s;
        const double n1 = sdk * s + dkp1 * c;
        const double ne = sdk * c - dkp1 * s;
        d[0] = n0;
        d[1] = n1;
        e = ne;
    }
    if (d[1] < d[0]) std::swap(d[0], d[1]);
    ev[0] = d[0] * scale;
    ev[1] = d[1] * scale;
}

// ---- MSVC <complex> internals used by PB2D --------------------------------------------------
struct Cx {
    double re, im;
};

// std::sqrt(complex<double>) (FUN_180011340), finite arguments.
Cx msvc_csqrt(double x, double y) {
    double big = std::fabs(x), small = std::fabs(y);
    if (big < small) std::swap(big, small);
    if (big == 0.0) return {0.0, y};
    int e;
    if (big < 1.0) {
        if (2.004168360008973e-292 <= big) {
            big = big * 4.0;
            small = small * 4.0;
            e = -2;
        } else {
            big = std::ldexp(big, 106);
            small = std::ldexp(small, 106);
            e = -106;
        }
    } else {
        big = big * 0.0625;
        small = small * 0.0625;
        e = 4;
    }
    double d = big - small;
    if (d != big) {
        if (d <= small) {
            d = (1.0 / small) * d;
            const double t = (d + 2.0) * d;
            d = t / (std::sqrt(t + 2.0) + 1.4142135623730951) + 2.414213562373095 + d;
        } else {
            d = (1.0 / small) * big;
            d = std::sqrt(d * d + 1.0) + d;
        }
        big = big + small / d;
    }
    const double ax = std::ldexp(std::fabs(x), -e);
    const double r = std::ldexp(std::sqrt(ax + ax + big + big), e / 2 - 1);
    if (0.0 < x || x == 0.0) return {r, (0.5 / r) * y};
    const double im = std::copysign(r, y);
    return {std::fabs((0.5 / r) * y), im};
}

// exact square: hi = x*x, lo = error (FUN_18000fe20)
void sq_exact(double x, double& hi, double& lo) {
    hi = x * x;
    std::uint64_t b;
    std::memcpy(&b, &x, 8);
    b = (b + 0x3ffffffu) & 0xfffffffff8000000ull;
    double h;
    std::memcpy(&h, &b, 8);
    const double l = x - h;
    lo = (h + h) * l + (h * h - hi) + l * l;
}

// log1p of the MSVC STL (FUN_1800124c0)
double msvc_log1p(double x) {
    if (x <= -0.5 || 2.0 <= x) return std::log(x + 1.0);
    if (2.220446049250313e-16 <= std::fabs(x)) {
        const double s = 1.0 + x;
        const double err = x - ((1.0 + x) - 1.0);
        return std::log(s) + err / s;
    }
    if (x == 0.0) return x;
    return x - x * 0.5 * x;
}

// log(abs(x + iy)) of the MSVC STL (FUN_180011e80), finite arguments.
double msvc_log_abs(double x, double y) {
    double a = std::fabs(x), b = std::fabs(y);
    if (a < b) std::swap(a, b);  // a = max, b = min
    if (b == 0.0) return std::log(a);
    if (a <= 1.4156865331029228e-146 || 6.703903964971298e+153 <= a) {
        const int ex = std::ilogb(a);
        const double as = std::scalbn(a, -ex), bs = std::scalbn(b, -ex);
        return std::log(bs * bs + as * as) * 0.5 + static_cast<double>(ex) * 0.6931471805599453;
    }
    double v = b * b;
    if (a != 1.0) {
        v = a * a + v;
        if (v <= 0.5 || 3.0 <= v) return std::log(v) * 0.5;
        double ah, al, bh, bl;
        sq_exact(a, ah, al);
        sq_exact(b, bh, bl);
        // (a^2 - 1) as a double-double
        const double d = ah - 1.0;
        const double t = (ah - (d - -1.0)) + al;
        const double s0 = t + d;
        const double s1 = t - (s0 - d);
        // + b^2
        const double s = s0 + bh;
        const double z = s - s0;
        v = (((s0 - (s - z)) + (bh - z)) + (s1 + bl)) + s;
    }
    return msvc_log1p(v) * 0.5;
}

// MSVCP140 _Exp(px, y, 0): y * exp(*px) (Dinkumware algorithm).
double msvc_exp_times(double x, double y) {
    if (x < -1418.0 || y == 0.0) return 0.0;
    double g = x * 1.4426950408889634;
    const short xexp = static_cast<short>(g + (g < 0.0 ? -0.5 : 0.5));
    g = xexp;
    g = (x - g * (22713.0 / 32768.0)) - g * 1.4286068203094172e-06;
    double r;
    int ex = xexp;
    if (-2.220446049250313e-16 < g && g < 2.220446049250313e-16) {
        r = y;
    } else {
        const double z = g * g;
        const double w = (30.01511290683317 * z + 3362.72154416553) * z + 30265.401893609497;
        g *= (z + 420.30235984910635) * z + 15132.700946804748;
        r = (w + g) / (w - g) * 2.0 * y;
        --ex;
    }
    return std::ldexp(r, ex);
}

// std::polar(exp(rho), theta) as in FUN_180011920
Cx msvc_polar_exp(double rho, double theta) {
    double s, c;
    dll_sincos(theta, s, c);
    return {msvc_exp_times(rho, c), msvc_exp_times(rho, s)};
}

// R^2 scale factor s of PB2D for one MSVC rand() value.
double dll_pb_s(int k) {
    const double dk = static_cast<double>(k);
    const double u = dk * kInvRandMax;
    const Cx q = msvc_csqrt(dk * dk * 9.313794206365304e-10 - u, 0.0);
    const double wim = q.im + q.im;
    const double wre = (1.0 - (u + u)) + (q.re + q.re);
    Cx c;
    if (wim == 0.0) {
        if (wre < 0.0) {
            c = msvc_polar_exp(msvc_log_abs(wre, 0.0) * 0.3333333333333333,
                               std::atan2(0.0, wre) * 0.3333333333333333);
        } else {
            c = {std::pow(wre, 0.3333333333333333), std::copysign(0.0, 0.3333333333333333)};
        }
        if (std::signbit(wim)) c.im = -c.im;
    } else {
        c = msvc_polar_exp(msvc_log_abs(wre, wim) * 0.3333333333333333,
                           std::atan2(wim, wre) * 0.3333333333333333);
    }
    // real part of (1 - i sqrt3)/(4c) (Smith division)
    const double re4 = c.re * 4.0, im4 = c.im * 4.0;
    double d;
    if (std::fabs(re4) <= std::fabs(im4)) {
        if (im4 == 0.0) {
            d = 1.0 / re4;
        } else {
            const double t = re4 / im4;
            const double den = t * re4 + im4;
            d = (t - kSqrt3) / den;
        }
    } else {
        const double t = im4 / re4;
        const double den = t * im4 + re4;
        d = (1.0 - t * kSqrt3) / den;
    }
    return (c.re - c.im * kSqrt3) * 0.25 + (d + 0.5);
}

// GenerateCoordinates (FUN_180011070): one point on the 4-D hyper-ellipsoid T[5].
void dll_coordinates(const std::array<double, 6>& T, MsvcRand& rng, Row4& row) {
    const int k0 = rng.rand();
    const double r2 = static_cast<double>(k0) * kInvRandMax * T[5];
    const double r = std::sqrt(r2);
    const double ry = std::sqrt((T[5] - r2) / T[4]);
    const int k1 = rng.rand();
    const int k2 = rng.rand();
    const double c1 = std::cos(static_cast<double>(k1) * kAngle);
    row[0] = r * T[0] * c1;
    const double s1 = std::sin(static_cast<double>(k1) * kAngle);
    row[1] = (c1 * T[1] - s1 / T[0]) * r;
    const double c2 = std::cos(static_cast<double>(k2) * kAngle);
    row[2] = ry * T[2] * c2;
    const double s2 = std::sin(static_cast<double>(k2) * kAngle);
    row[3] = (c2 * T[3] - s2 / T[2]) * ry;
    if (!(row[2] < 100000.0) || !(-100000.0 < row[3])) {  // NaN guard: copy the first plane
        row[2] = row[0];
        row[3] = row[1];
    }
}

}  // namespace

std::array<double, 6> dll_pretreatment(const Twiss2& tw) {
    struct Axis {
        double a, b, theta;
    };
    auto plane = [](double eps, double beta, double alpha, bool absEv, double& s11, double& s22) -> Axis {
        s11 = eps * beta;
        s22 = ((alpha * alpha + 1.0) / beta) * eps;
        const double s12 = alpha * eps;
        if (!(-s12 * -s12 < s11 * s22)) {
            say("ERROR!!!   @SigmaToAxial2D!");
            if (compat_dll()) compat_exit(1);
            throw EngineError("ERROR!!!   @SigmaToAxial2D!");
        }
        const double sq22 = std::sqrt(s22);
        const double sq11 = std::sqrt(s11);
        const double t = -s12 / sq11;
        const double f = 1.0 / ((sq11 * sq22 - (t / sq22) * -s12) * sq22);
        const double m12x2 = f * t * -2.0;
        const double m22 = f * sq11;
        const double m11 = (t * t * m22 + 1.0) / s11;
        double ev[2];
        eigen_sym2(m11, m12x2 * 0.5, m22, ev);
        if (s11 < s22) std::swap(ev[0], ev[1]);
        Axis ax;
        ax.a = absEv ? std::sqrt(std::fabs(1.0 / ev[0])) : std::sqrt(1.0 / ev[0]);
        ax.b = absEv ? std::sqrt(std::fabs(1.0 / ev[1])) : std::sqrt(1.0 / ev[1]);
        // sigma_11 == sigma_22 gives theta = 0 even when sigma_12 != 0 (defect kept in compat)
        ax.theta = m11 == m22 ? 0.0 : std::atan(m12x2 / (m11 - m22)) * 28.64788975654116;
        return ax;
    };
    double s11, s22;
    const Axis p1 = plane(tw.eps1, tw.beta1, tw.alpha1, true, s11, s22);
    const Axis p2 = plane(tw.eps2, tw.beta2, tw.alpha2, false, s11, s22);
    const double area1 = p1.b * p1.a * 4.0;
    const double area2 = p2.b * p2.a * 4.0;
    std::array<double, 6> T{};
    {
        double s, c;
        dll_sincos(p1.theta * 0.017453292519943295, s, c);
        const double r1 = p1.b / p1.a, r2 = p1.a / p1.b;
        T[0] = std::sqrt(s * s * r1 + c * c * r2);
        T[1] = std::sin(p1.theta * 0.03490658503988659) * ((r2 - r1) * (0.5 / T[0]));
    }
    {
        double s, c;
        dll_sincos(p2.theta * 0.017453292519943295, s, c);
        const double r1 = p2.a / p2.b, r2 = p2.b / p2.a;
        T[2] = std::sqrt(s * s * r2 + c * c * r1);
        T[3] = std::sin(p2.theta * 0.03490658503988659) * ((r1 - r2) * (0.5 / T[2]));
    }
    T[4] = area1 / area2;
    T[5] = 0.0;
    return T;
}

void dll_generate(const std::string& type, const Twiss2& tw, std::vector<Row4>& rows, int seed, MsvcRand& rng) {
    if (seed != 0) rng.srand(static_cast<std::uint32_t>(seed));
    std::array<double, 6> T = dll_pretreatment(tw);
    const double eps = tw.eps1;
    if (type == "GS") {
        const double isq = 1.0 / std::sqrt(T[4]);
        const double s = std::sqrt(eps * 4.0) * 0.5;
        double u[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        for (Row4& row : rows) {
            bool redo;
            do {
                for (int k = 0; k < 8; ++k) {
                    const double v = static_cast<double>(rng.rand()) * kInvRandMax;
                    u[k] = v;
                    for (int j = 0; j < k; ++j) {
                        if (v == u[j]) {
                            --k;  // draw this one again
                            break;
                        }
                    }
                }
                double g[4] = {0, 0, 0, 0};
                redo = false;
                for (int j = 0; j < 4; ++j) {
                    const double a = (u[2 * j] + u[2 * j]) - 1.0;
                    const double b = (u[2 * j + 1] + u[2 * j + 1]) - 1.0;
                    const double t = b * b + a * a;
                    if (1.0 < t) {
                        redo = true;
                        break;
                    }
                    g[j] = std::sqrt((std::log(t) * -2.0) / t) * a;
                }
                for (double& v : g) v = v * s;
                row[0] = g[0] * T[0];
                row[1] = T[1] * g[0] + g[1] / T[0];
                row[2] = T[2] * g[2] * isq;
                row[3] = (g[3] / T[2] + T[3] * g[2]) * isq;
            } while (redo);
        }
        return;
    }
    for (Row4& row : rows) {
        if (type == "KV") {
            T[5] = eps * 4.0;
        } else if (type == "WB") {
            T[5] = std::sqrt(static_cast<double>(rng.rand()) * kInvRandMax) * (eps * 6.0);
        } else {  // PB
            const double s = dll_pb_s(rng.rand());
            T[5] = s * (eps * 8.0);
        }
        dll_coordinates(T, rng, row);
    }
}

void default_generate(const std::string& type, const Twiss2& tw, std::vector<Row4>& rows, const Philox4x32& rng,
                      std::uint32_t stream) {
    auto check = [](double eps, double beta) {
        if (!(eps > 0.0) || !(beta > 0.0))
            throw EngineError("The emittance and beta of twissx, twissy and twissz must be positive");
    };
    check(tw.eps1, tw.beta1);
    check(tw.eps2, tw.beta2);
    const double sb1 = std::sqrt(tw.beta1), sb2 = std::sqrt(tw.beta2);
    const double se1 = std::sqrt(tw.eps1), se2 = std::sqrt(tw.eps2);
    const std::uint32_t n = static_cast<std::uint32_t>(rows.size());
    for (std::uint32_t i = 0; i < n; ++i) {
        double u[4];
        rng.uniform2(i, stream, 0, u);
        rng.uniform2(i, stream, 1, u + 2);
        // normalised coordinates (X, X') of each plane with <X^2> = <X'^2> = 1
        double X1, P1, X2, P2;
        if (type == "GS") {
            p_gauss2(u[0], u[1], X1, P1);
            p_gauss2(u[2], u[3], X2, P2);
        } else {
            double rho2;  // 4-D radius^2 in units of the rms emittance
            if (type == "KV") rho2 = 4.0;
            else if (type == "WB") rho2 = 6.0 * std::sqrt(u[3]);
            else rho2 = 8.0 * p_parabolic_inv(u[3]);
            const double j1 = u[0] * rho2;
            const double a1 = std::sqrt(j1), a2 = std::sqrt(rho2 - j1);
            double s1, c1, s2, c2;
            p_sincos_turns(u[1], s1, c1);
            p_sincos_turns(u[2], s2, c2);
            X1 = a1 * c1;
            P1 = -(a1 * s1);
            X2 = a2 * c2;
            P2 = -(a2 * s2);
        }
        rows[i][0] = se1 * sb1 * X1;
        rows[i][1] = se1 / sb1 * (P1 - tw.alpha1 * X1);
        rows[i][2] = se2 * sb2 * X2;
        rows[i][3] = se2 / sb2 * (P2 - tw.alpha2 * X2);
    }
}

}  // namespace avas
