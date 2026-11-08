// Mesh work of the space-charge solvers on a device (one call per mesh node / target), shared by the
// CUDA back end and the CPU emulation of the device (device_emu.cpp).
//   SPICNIC: the loops of SpaceCharge::spicnic() of the corrected engine, one upper-octant target per
//            call, in the same order -- bit-identical to the CPU engine.
//   FFT:     direct transforms (DST-I in x, y; complex DFT in z) in a fixed order: deterministic on any
//            device and any number of GPUs; they differ from pocketfft (CPU) in the last bits.
#pragma once
#include <cmath>
#include <vector>

#include "core/hd.h"

namespace avas {
namespace sck {

constexpr double kE = 8987551787.368176;  // 1/(4 pi eps0)

struct SpGrid {
    int nx, ny, nz, mx, my, mz;
    AVAS_HD long long n3() const { return static_cast<long long>(nx) * ny * nz; }
    AVAS_HD long long at(int i, int j, int k) const { return (static_cast<long long>(i) * ny + j) * nz + k; }
    AVAS_HD long long gat(int a, int b, int c) const {
        return (static_cast<long long>(a) * (2 * ny + 1) + b) * (2 * nz + 1) + c;
    }
    AVAS_HD void ijk(long long id, int& i, int& j, int& k) const {
        i = static_cast<int>(id / (static_cast<long long>(ny) * nz));
        j = static_cast<int>((id / nz) % ny);
        k = static_cast<int>(id % nz);
    }
    // upper octant targets
    AVAS_HD long long n_targets() const {
        return static_cast<long long>(nx - nx / 2) * (ny - ny / 2) * (nz - nz / 2);
    }
    AVAS_HD void target(long long id, int& i, int& j, int& k) const {
        const int nj = ny - ny / 2, nk = nz - nz / 2;
        i = nx / 2 + static_cast<int>(id / (static_cast<long long>(nj) * nk));
        j = ny / 2 + static_cast<int>((id / nk) % nj);
        k = nz / 2 + static_cast<int>(id % nk);
    }
};

AVAS_HD inline int imax(int a, int b) { return a > b ? a : b; }
AVAS_HD inline int imin(int a, int b) { return a < b ? a : b; }

// Potential of the octant charge mirrored into the 8 octants at target (i, j, k), times 1/(4 pi eps0),
// written to the 8 mirror nodes.
template <class T>
AVAS_HD inline void sym_target(const SpGrid& g, const T* rho, const T* G, T thr, int i, int j, int k, double* phi) {
    const int nx = g.nx, ny = g.ny, nz = g.nz, mx = g.mx, my = g.my, mz = g.mz;
    const int seg[3][2][2] = {
        {{imax(0, i - mx), imin(imin(nx, i + mx), (nx + 1) / 2)}, {imax(imax(0, i - mx), nx / 2), imin(nx, i + mx)}},
        {{imax(0, j - my), imin(imin(ny, j + my), (ny + 1) / 2)}, {imax(imax(0, j - my), ny / 2), imin(ny, j + my)}},
        {{imax(0, k - mz), imin(imin(nz, k + mz), (nz + 1) / 2)}, {imax(imax(0, k - mz), nz / 2), imin(nz, k + mz)}}};
    T S = T(0);
    for (int si = 0; si < 2; ++si)
        for (int sj = 0; sj < 2; ++sj)
            for (int sk = 0; sk < 2; ++sk)
                for (int a = seg[0][si][0]; a < seg[0][si][1]; ++a) {
                    const int ra = si == 0 ? nx - 1 - a : a;
                    for (int bb = seg[1][sj][0]; bb < seg[1][sj][1]; ++bb) {
                        const int rb = sj == 0 ? ny - 1 - bb : bb;
                        for (int c = seg[2][sk][0]; c < seg[2][sk][1]; ++c) {
                            const int rc = sk == 0 ? nz - 1 - c : c;
                            const T r = rho[g.at(ra, rb, rc)];
                            if (r > thr) S = r * G[g.gat(nx - i + a, ny - j + bb, nz - k + c)] + S;
                        }
                    }
                }
    const double v = static_cast<double>(S) * kE;
    const int Is[2] = {nx - 1 - i, i}, Js[2] = {ny - 1 - j, j}, Ks[2] = {nz - 1 - k, k};
    for (int a = 0; a < 2; ++a)
        for (int b = 0; b < 2; ++b)
            for (int c = 0; c < 2; ++c) phi[g.at(Is[a], Js[b], Ks[c])] = v;
}

// Asymmetric part of the charge at node (a, bb, c): sum over the correction list, added to phi.
template <class T>
AVAS_HD inline void corr_node(const SpGrid& g, const T* G, const int* corr, const T* dval, long long nc, int a, int bb,
                              int c, double* phi) {
    T s = T(0);
    for (long long q = 0; q < nc; ++q)
        s = G[g.gat(g.nx + corr[3 * q] - a, g.ny + corr[3 * q + 1] - bb, g.nz + corr[3 * q + 2] - c)] * dval[q] + s;
    phi[g.at(a, bb, c)] = static_cast<double>(s) * kE + phi[g.at(a, bb, c)];
}

AVAS_HD inline double sp_grad(int i, int n, double pm, double p0, double pp, double h) {
    if (i == 0) return 2.0 * (-(pp - p0) * h);
    if (i == n - 1) return 2.0 * (-(p0 - pm) * h);
    return -(pp - pm) * h;
}

AVAS_HD inline void grad_spicnic_node(const SpGrid& g, const double* phi, double hx, double hy, double hz, int i, int j,
                                      int k, double* ex, double* ey, double* ez) {
    const long long id = g.at(i, j, k);
    const double p0 = phi[id];
    ex[id] = sp_grad(i, g.nx, i > 0 ? phi[g.at(i - 1, j, k)] : 0.0, p0, i + 1 < g.nx ? phi[g.at(i + 1, j, k)] : 0.0, hx);
    ey[id] = sp_grad(j, g.ny, j > 0 ? phi[g.at(i, j - 1, k)] : 0.0, p0, j + 1 < g.ny ? phi[g.at(i, j + 1, k)] : 0.0, hy);
    ez[id] = sp_grad(k, g.nz, k > 0 ? phi[g.at(i, j, k - 1)] : 0.0, p0, k + 1 < g.nz ? phi[g.at(i, j, k + 1)] : 0.0, hz);
}

// DST-I along axis 0 (x) or 1 (y) at node (i, j, k); sinTab[q] = sin(pi q / (N + 1)).
AVAS_HD inline double dst_node(const SpGrid& g, int axis, const double* in, const double* sinTab, int i, int j, int k) {
    const int N = axis == 0 ? g.nx : g.ny;
    const int kk = axis == 0 ? i : j;
    const int period = 2 * (N + 1);
    double s = 0.0;
    for (int m = 0; m < N; ++m) {
        const double v = axis == 0 ? in[g.at(m, j, k)] : in[g.at(i, m, k)];
        s = s + v * sinTab[((m + 1) * (kk + 1)) % period];
    }
    return 2.0 * s;
}

// Complex DFT along z at node (i, j, k): sign -1 forward, +1 backward (im may be null: real input).
AVAS_HD inline void dft_z_node(const SpGrid& g, const double* re, const double* im, const double* cosTab, const double* sinTab,
                               double sign, int i, int j, int k, double& ore, double& oim) {
    const long long base = g.at(i, j, 0);
    const int N = g.nz;
    double sr = 0.0, si = 0.0;
    for (int m = 0; m < N; ++m) {
        const int q = static_cast<int>((static_cast<long long>(m) * k) % N);
        const double c = cosTab[q], s = sign * sinTab[q];
        const double xr = re[base + m], xi = im ? im[base + m] : 0.0;
        sr = sr + (xr * c - xi * s);
        si = si + (xr * s + xi * c);
    }
    ore = sr;
    oim = si;
}

AVAS_HD inline double green_factor(const double* lx, const double* ly, const double* lz, int i, int j, int k) {
    return 112940906683.72818 / ((lx[i] + ly[j]) + lz[k]);
}

AVAS_HD inline void grad_fft_node(const SpGrid& g, const double* phi, double hx, double hy, double hz, int i, int j, int k,
                                  double* ex, double* ey, double* ez) {
    const long long id = g.at(i, j, k);
    const double xp = i + 1 < g.nx ? phi[g.at(i + 1, j, k)] : 0.0;
    const double xm = i > 0 ? phi[g.at(i - 1, j, k)] : 0.0;
    const double yp = j + 1 < g.ny ? phi[g.at(i, j + 1, k)] : 0.0;
    const double ym = j > 0 ? phi[g.at(i, j - 1, k)] : 0.0;
    const double zp = phi[g.at(i, j, (k + 1) % g.nz)];
    const double zm = phi[g.at(i, j, (k + g.nz - 1) % g.nz)];
    ex[id] = -(xp - xm) * hx;
    ey[id] = -(yp - ym) * hy;
    ez[id] = -(zp - zm) * hz;
}

// Host tables of the direct transforms (the same values on the CPU emulation and the GPUs).
struct FftTables {
    std::vector<double> sx, sy, cz, sz;
};
inline FftTables fft_tables(int nx, int ny, int nz) {
    FftTables t;
    auto sinTab = [](int N) {
        std::vector<double> v(static_cast<size_t>(2 * (N + 1)));
        for (size_t q = 0; q < v.size(); ++q) v[q] = std::sin(3.141592653589793 * static_cast<double>(q) / (N + 1));
        return v;
    };
    t.sx = sinTab(nx);
    t.sy = sinTab(ny);
    t.cz.resize(static_cast<size_t>(nz));
    t.sz.resize(static_cast<size_t>(nz));
    for (int q = 0; q < nz; ++q) {
        t.cz[static_cast<size_t>(q)] = std::cos(6.283185307179586 * q / nz);
        t.sz[static_cast<size_t>(q)] = std::sin(6.283185307179586 * q / nz);
    }
    return t;
}

}  // namespace sck
}  // namespace avas
