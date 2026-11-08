// Field of a uniformly charged ellipsoid from the space-charge solvers of the new engine (default
// mode) against the analytic linear field inside, E_i = kappa_i x_i with
//   kappa_i = Q/(4 pi eps0) * 3/2 * Int_0^inf ds / ((a_i^2 + s) sqrt((a^2+s)(b^2+s)(c^2+s))).
// SPICNIC and PICNIC (open boundary): convergence with mesh and particle number.  FFT (grounded walls in x, y,
// periodic in z): convergence to the free-space result as the box (meshrms) grows at a fixed cell
// size, which shows that its deviation is the boundary.  Exit code 1 when a threshold fails.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "core/rng.h"
#include "core/spacecharge.h"

using namespace avas;

static const double kAx[3] = {1.0e-3, 1.5e-3, 3.0e-3};
static const double kQ = 1e-11;

static double kappa(int i) {
    double sum = 0.0;
    const int n = 2000000;
    const double L2 = kAx[0] * kAx[0];
    for (int m = 0; m < n; ++m) {
        const double t = (m + 0.5) / n, s = L2 * t / (1.0 - t), ds = L2 / ((1.0 - t) * (1.0 - t)) / n;
        sum += ds / ((kAx[i] * kAx[i] + s) * std::sqrt((kAx[0] * kAx[0] + s) * (kAx[1] * kAx[1] + s) * (kAx[2] * kAx[2] + s)));
    }
    return kQ * 8987551787.368176 * 1.5 * sum;
}

// slope / analytic for the three axes, particles inside r_norm < rfit
static void run(int method, int grid, double meshrms, int n, double rfit, const double kap[3], double out[3]) {
    Params p;
    p.spaceCharge = 1;
    p.scPar[0] = method;
    p.scPar[1] = p.scPar[2] = p.scPar[3] = meshrms;
    p.multithreading = 1;
    p.numOfGrid = {grid, grid, grid};
    space_charge_defaults(p, n);
    Beam b;
    b.resize(n);
    Philox4x32 ph{{11u, 0u}};
    int k = 0;
    for (std::uint32_t i = 0; k < n; ++i) {
        double u[4];
        ph.uniform2(i, 9, 0, u);
        ph.uniform2(i, 9, 1, u + 2);
        const double x = 2 * u[0] - 1, y = 2 * u[1] - 1, z = 2 * u[2] - 1;
        if (x * x + y * y + z * z > 1.0) continue;
        b.rel[0][k] = x * kAx[0];
        b.rel[2][k] = y * kAx[1];
        b.rel[4][k] = z * kAx[2];
        b.charge[k] = kQ / n;
        ++k;
    }
    b.gamma = 1.0;
    b.centroid = {0, 0, 0, 0, 0, 0};
    b.rms = {kAx[0] / std::sqrt(5.0), kAx[1] / std::sqrt(5.0), kAx[2] / std::sqrt(5.0)};
    SpaceCharge sc(p, n);
    sc.solve(b, 1.0);
    const std::vector<double>* f[3] = {&b.scx, &b.scy, &b.scz};
    const std::vector<double>* q[3] = {&b.rel[0], &b.rel[2], &b.rel[4]};
    for (int a = 0; a < 3; ++a) {
        double sxy = 0, sxx = 0;
        for (int i = 0; i < n; ++i) {
            const double r2 = std::pow(b.rel[0][i] / kAx[0], 2) + std::pow(b.rel[2][i] / kAx[1], 2) + std::pow(b.rel[4][i] / kAx[2], 2);
            if (r2 > rfit * rfit) continue;
            sxy += (*f[a])[i] * (*q[a])[i];
            sxx += (*q[a])[i] * (*q[a])[i];
        }
        out[a] = sxy / sxx / kap[a];
    }
}

int main() {
    double kap[3];
    for (int i = 0; i < 3; ++i) kap[i] = kappa(i);
    const double rfit = 0.8;
    int fails = 0;
    auto worst = [](const double r[3]) {
        return std::max(std::fabs(r[0] - 1), std::max(std::fabs(r[1] - 1), std::fabs(r[2] - 1)));
    };
    std::printf("Uniform ellipsoid, semi-axes 1 / 1.5 / 3 mm, linear field fitted inside 0.8 of each semi-axis\n");
    std::printf("SPICNIC (open boundary, meshrms 3.5): slope / analytic for x, y, z\n");
    double prev = 1e9, last = 0;
    const int grids[3] = {20, 30, 40};
    const int parts[3] = {250000, 500000, 1000000};
    for (int g = 0; g < 3; ++g) {
        double r[3];
        run(1, grids[g], 3.5, parts[g], rfit, kap, r);
        last = worst(r);
        std::printf("  grid %2d^3, %7d particles: %.4f %.4f %.4f  (max dev %.4f)\n", grids[g], parts[g], r[0], r[1], r[2], last);
        if (last > prev + 0.002) ++fails;  // must not get worse when refined
        prev = last;
    }
    if (last > 0.015) ++fails;
    std::printf("  threshold: max dev <= 0.015 at 40^3 and non-increasing: %s\n", last <= 0.015 ? "ok" : "FAIL");
    std::printf("PICNIC (libPIC.so direct sum, open boundary, meshrms 3.5): slope / analytic for x, y, z\n");
    prev = 1e9;
    const int pg[3] = {16, 24, 32};
    const int pp[3] = {250000, 500000, 1000000};
    for (int g = 0; g < 3; ++g) {
        double r[3];
        run(2, pg[g], 3.5, pp[g], rfit, kap, r);
        last = worst(r);
        std::printf("  grid %2d^3, %7d particles: %.4f %.4f %.4f  (max dev %.4f)\n", pg[g], pp[g], r[0], r[1], r[2], last);
        if (last > prev + 0.002) ++fails;
        prev = last;
    }
    if (last > 0.01) ++fails;
    std::printf("  threshold: max dev <= 0.01 at 32^3 and non-increasing: %s\n", last <= 0.01 ? "ok" : "FAIL");
    std::printf("FFT (grounded walls in x, y; period 2 meshrms rms_z in z), same cell size, growing box:\n");
    const double mrs[3] = {6.5, 9.75, 13.0};
    const int fg[3] = {64, 96, 128};
    prev = 1e9;
    for (int g = 0; g < 3; ++g) {
        double r[3];
        run(0, fg[g], mrs[g], 500000, rfit, kap, r);
        last = worst(r);
        std::printf("  meshrms %5.2f, grid %3d^3: %.4f %.4f %.4f  (max dev %.4f)\n", mrs[g], fg[g], r[0], r[1], r[2], last);
        if (last > prev + 0.002) ++fails;
        prev = last;
    }
    if (last > 0.015) ++fails;
    std::printf("  threshold: max dev <= 0.015 at meshrms 13 and non-increasing: %s\n", last <= 0.015 ? "ok" : "FAIL");
    std::printf("%s\n", fails ? "ELLIPSOID CHECK FAILED" : "ELLIPSOID CHECK OK");
    return fails ? 1 : 0;
}
