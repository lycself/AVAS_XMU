#include "core/spacecharge.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <atomic>
#include <functional>
#include <thread>

#include "core/dllmath.h"
#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/physics.h"
#include "third_party/pocketfft/pocketfft_hdronly.h"

namespace avas {

namespace {

constexpr double kE = 8987551787.368176;  // 1/(4 pi eps0)

// Runs task(0..n-1) on up to hardware_concurrency threads.  Every task writes its own output,
// so the result never depends on the thread count.
void run_tasks(int n, const std::function<void(int)>& task) {
    int t = options().threads > 0 ? options().threads : static_cast<int>(std::thread::hardware_concurrency());
    t = std::max(1, std::min(t, n));
    if (t <= 1) {
        for (int i = 0; i < n; ++i) task(i);
        return;
    }
    std::vector<std::thread> th;
    std::atomic<int> next{0};
    for (int k = 0; k < t; ++k)
        th.emplace_back([&] {
            for (int i = next++; i < n; i = next++) task(i);
        });
    for (auto& x : th) x.join();
}

std::vector<int> particle_blocks(int n, int& tasks) {
    std::vector<int> b{0};
    if (tasks >= 1) {
        const int c = static_cast<int>(std::ceil(static_cast<double>(n) / static_cast<double>(tasks)));
        for (int i = 0; i < tasks; ++i) {
            const int nxt = b.back() + c;
            if (n <= nxt) break;
            b.push_back(nxt);
        }
    }
    b.push_back(n);
    tasks = static_cast<int>(b.size()) - 1;
    return b;
}

}  // namespace

void space_charge_defaults(Params& p, long long n) {
    if (p.spaceCharge != 1) {
        p.numOfGrid = {3, 3, 3};
        return;
    }
    auto& s = p.scPar;
    const double N = static_cast<double>(n);
    if (s[0] == 0.0) {
        if (s[1] == -1.0) s[1] = s[2] = s[3] = 6.5;
        if (p.numOfGrid[0] == -1) p.numOfGrid = {64, 64, 64};
    } else if (s[0] == 1.0) {
        if (s[1] == -1.0) {
            const double v = N <= 8e4 ? 3.5 : N <= 8e5 ? 4.0 : N <= 8e6 ? 5.0 : N <= 8e7 ? 6.0 : 7.0;
            s[1] = s[2] = s[3] = v;
        }
        if (s[4] == -1.0) s[4] = s[5] = s[6] = 8.0;
        if (p.numOfGrid[0] == -1) {
            const int g = N <= 3e4 ? 14 : N <= 8e4 ? 18 : N <= 3e5 ? 20 : N <= 8e5 ? 26 : N <= 3e6 ? 30
                        : N <= 8e6 ? 40 : N <= 3e7 ? 44 : N <= 8e7 ? 56 : 64;
            p.numOfGrid = {g, g, g};
        }
        if (s[7] == -1.0) s[7] = std::ceil(p.numOfGrid[0] / 3.0);
    }
    s[9] = static_cast<double>(p.multithreading);
}

SpaceCharge::SpaceCharge(const Params& p, int nParticles) : compat_(compat_dll()) {
    method_ = static_cast<int>(p.scPar[0]);
    nx_ = p.numOfGrid[0];
    ny_ = p.numOfGrid[1];
    nz_ = p.numOfGrid[2];
    for (int i = 0; i < 3; ++i) mr_[i] = p.scPar[1 + i];
    negligible_ = p.scPar[8];
    lp_ = p.lpBoundary;
    rec_ = p.recScField;
    // AVAS.dll: lpboundary 1 asks for FFT, but scmethod overwrites it right after (no effect);
    // the corrected engine honours it
    if (lp_ == 1 && !compat_) method_ = 0;
    pool_ = static_cast<int>(p.scPar[9]) == 1;
    for (int k = 0; k < 10; ++k) cores_[k] = pool_ ? static_cast<int>(p.scPar[10 + k]) : 1;
    if (nx_ < 2 || ny_ < 2 || nz_ < 2) throw EngineError("numofgrid must be at least 2 in every direction");
    if (method_ == 1 && (nx_ % 2 || ny_ % 2 || nz_ % 2)) {
        // AVAS.dll loops forever on the mirror step 0 of an odd mesh (spec 60 2.7.4)
        throw EngineError("SPICNIC needs an even numofgrid in every direction");
    }
    mx_ = my_ = mz_ = static_cast<int>(std::round(nx_ * 3 * 0.25));
    if (!compat_) {
        // corrected: the kernel covers the whole mesh in every direction (AVAS.dll truncates it at
        // 0.75 Nx cells in all three directions, which cuts off the far part of long bunches)
        mx_ = nx_;
        my_ = ny_;
        mz_ = nz_;
    }
    r2_ = static_cast<int>(std::ceil((nx_ * 0.5 + 1.0) * (nx_ * 0.5 + 1.0)));
    say(Text().i(nx_).s(" ").i(ny_).s(" ").i(nz_).s(" ").g(negligible_).s(" ").g(mr_[0]).s(" ").g(mr_[1]).s(" ")
            .g(mr_[2]).str());
    wBounds_ = particle_blocks(nParticles, cores_[4]);
    aBounds_ = particle_blocks(nParticles, cores_[9]);
    // SPICNIC target segments (PIC::Initial 2-D partition); the corrected engine does not need them
    {
        const int A = nx_ / 2, B = ny_ / 2;
        int c7 = std::max(1, cores_[7]);
        const int c = static_cast<int>(std::ceil(static_cast<double>(A * B) / static_cast<double>(c7)));
        const int q = c / B, r = c % B;
        int i = A, j = B;
        while (true) {
            int ni = i + q, nj = j + r;
            if (nj >= ny_) {
                ni += 1;
                nj = nj - ny_ + B;
            }
            if (ni >= nx_ - 1 || (ni == nx_ - 1 && false)) {
                sTasks_.push_back({i, j, nx_ - 1, ny_});
                break;
            }
            sTasks_.push_back({i, j, ni, nj});
            i = ni;
            j = nj;
        }
        cores_[7] = static_cast<int>(sTasks_.size());
    }
    say(Text().s("mulitthread:").i(cores_[0]).s("  ").i(cores_[1]).s(" ").i(cores_[2]).s(" ").i(cores_[3]).s(" ")
            .i(cores_[4]).s(" ").i(cores_[5]).s(" ").i(cores_[6]).s(" ").i(cores_[7]).s(" ").i(cores_[8]).s("  ")
            .i(cores_[9]).str());
    const size_t n3 = static_cast<size_t>(nx_) * ny_ * nz_;
    xs_.resize(nx_);
    ys_.resize(ny_);
    zs_.resize(nz_);
    rho_.assign(n3, 0.0);
    phi_.assign(n3, 0.0);
    ex_.assign(n3, 0.0);
    ey_.assign(n3, 0.0);
    ez_.assign(n3, 0.0);
    if (method_ == 1) G_.assign(static_cast<size_t>(2 * nx_ + 1) * (2 * ny_ + 1) * (2 * nz_ + 1), 0.0);
    cell_.assign(nParticles, {0, 0, 0});
    w_.assign(nParticles, {});
    out_.assign(nParticles, 0);
}

bool SpaceCharge::solve(Beam& b, double factor, const double* sync) {
    ++calls_;
    for (int k = 0; k < 6; ++k) sync_[k] = sync ? sync[k] : 0.0;
    if (method_ == 1) {
        // autoPIC: skip when rms_z did not change (x, y are overwritten in the original)
        if (prevRms_[0] != 0.0) {
            const double d = std::fabs((prevRms_[2] - b.rms[2]) / prevRms_[2]);
            if (d < 1e-08) return false;
        }
        for (int k = 0; k < 3; ++k) prevRms_[k] = b.rms[k];
    }
    ++solves_;
    weight(b, factor);
    if (method_ == 0) fft();
    else spicnic();
    allocate(b);
    return true;
}

// ---- mesh and CIC deposit (PIC::Weight) ----
void SpaceCharge::weight(Beam& b, double factor) {
    const double* c = b.centroid.data();
    gamma_ = b.gamma;
    if (compat_) {
        dx_ = ((b.rms[0] + b.rms[0]) * mr_[0]) / static_cast<double>(nx_ - 1);
        dy_ = ((b.rms[1] + b.rms[1]) * mr_[1]) / static_cast<double>(ny_ - 1);
        dz_ = lp_ == 1 ? (1.0 / static_cast<double>(nz_ - 1)) * b.betaGammaLambda
                       : ((b.rms[2] + b.rms[2]) * mr_[2]) * (1.0 / static_cast<double>(nz_ - 1));
    } else {
        dx_ = ((b.rms[0] + b.rms[0]) * mr_[0]) / static_cast<double>(nx_ - 1);
        dy_ = ((b.rms[1] + b.rms[1]) * mr_[1]) / static_cast<double>(ny_ - 1);
        // lpboundary 1: one period of beta*gamma*lambda exactly (AVAS.dll: one cell longer)
        dz_ = lp_ == 1 ? b.betaGammaLambda / static_cast<double>(nz_) : ((b.rms[2] + b.rms[2]) * mr_[2]) / static_cast<double>(nz_ - 1);
    }
    const double cx = lp_ == 1 ? 0.0 : c[0], cy = lp_ == 1 ? 0.0 : c[2], cz = lp_ == 1 ? 0.0 : c[4];
    for (int i = 0; i < nx_; ++i) xs_[i] = ((i - nx_ * 0.5) + 0.5) * dx_ + cx;
    for (int j = 0; j < ny_; ++j) ys_[j] = ((j - ny_ * 0.5) + 0.5) * dy_ + cy;
    for (int k = 0; k < nz_; ++k) zs_[k] = ((k - nz_ * 0.5) + 0.5) * dz_ + cz;
    vol_ = dy_ * dx_ * dz_;
    igamma_ = 1.0 / gamma_;
    const int n = b.n;
    auto deposit = [&](int i0, int i1, std::vector<double>& g) {
        for (int p = i0; p < i1; ++p) {
            if (b.lost[p] == 1) continue;
            const double x = b.rel[0][p], y = b.rel[2][p];
            const double zr = lp_ == 1 ? b.rel[4][p] * gamma_ + c[4] : (b.rel[4][p] - c[4]) * gamma_ + c[4];
            const int i = static_cast<int>(std::floor((x - xs_[0]) / dx_));
            const int j = static_cast<int>(std::floor((y - ys_[0]) / dy_));
            const int k = static_cast<int>(std::floor((zr - zs_[0]) / dz_));
            cell_[p] = {i, j, k};
            auto& w = w_[p];
            if (0 <= i && i < nx_ - 1 && 0 <= j && j < ny_ - 1 && 0 <= k && k < nz_ - 1) {
                out_[p] = 0;
                for (int m = 0; m < 8; ++m) {
                    const int ii = i + ((m & 1) ^ 1);
                    const int jj = j + (((m & 3) == 1 || (m & 3) == 0) ? 1 : 0);
                    const int kk = k + (m < 4 ? 1 : 0);
                    w[m] = std::fabs(x - xs_[ii]) * std::fabs(y - ys_[jj]) * std::fabs(zr - zs_[kk]) / vol_;
                }
                const double q = b.charge[p];
                g[at(i, j, k)] = w[0] * q + g[at(i, j, k)];
                g[at(i + 1, j, k)] = w[1] * q + g[at(i + 1, j, k)];
                g[at(i, j + 1, k)] = w[2] * q + g[at(i, j + 1, k)];
                g[at(i + 1, j + 1, k)] = w[3] * q + g[at(i + 1, j + 1, k)];
                g[at(i, j, k + 1)] = w[4] * q + g[at(i, j, k + 1)];
                g[at(i + 1, j, k + 1)] = w[5] * q + g[at(i + 1, j, k + 1)];
                g[at(i, j + 1, k + 1)] = w[6] * q + g[at(i, j + 1, k + 1)];
                g[at(i + 1, j + 1, k + 1)] = w[7] * q + g[at(i + 1, j + 1, k + 1)];
            } else {
                out_[p] = 1;
                w.fill(0.0);
            }
        }
    };
    std::fill(rho_.begin(), rho_.end(), 0.0);
    // AVAS.dll: particle segments of cores[4] tasks into private meshes (serial when
    // multithreading 0 or one task); the corrected engine always uses 64 fixed segments, so the
    // summation order never depends on the machine
    std::vector<int> bounds;
    if (compat_) {
        bounds = (pool_ && cores_[4] != 1) ? wBounds_ : std::vector<int>{0, n};
    } else {
        bounds = {0, n};  // corrected engine: one pass in particle order (independent of threads)
    }
    const int nb = static_cast<int>(bounds.size()) - 1;
    if (nb == 1) {
        deposit(0, n, rho_);
    } else {
        std::vector<std::vector<double>> loc(nb, std::vector<double>(rho_.size(), 0.0));
        run_tasks(nb, [&](int t) { deposit(bounds[t], bounds[t + 1], loc[t]); });
        for (size_t e = 0; e < rho_.size(); ++e) {
            double s = 0.0;
            for (int t = 0; t < nb; ++t) s = s + loc[t][e];
            rho_[e] = s;
        }
    }
    if (compat_) {
        if (factor < 0.999999999)
            for (double& v : rho_) v = v * (1.0 - factor);
    } else if (factor != 1.0) {
        for (double& v : rho_) v = v * factor;
    }
}

// ---- FFT Poisson solver: DST-I in x, y (grounded walls), periodic FFT in z ----
void SpaceCharge::fft() {
    const size_t n3 = rho_.size();
    std::vector<double> a(n3), bbuf(n3);
    const double s = ((1.0 / dx_) / dy_) / dz_;
    for (size_t e = 0; e < n3; ++e) a[e] = rho_[e] * s;
    using pocketfft::shape_t;
    using pocketfft::stride_t;
    const shape_t shape{static_cast<size_t>(nx_), static_cast<size_t>(ny_), static_cast<size_t>(nz_)};
    const stride_t sd{static_cast<std::ptrdiff_t>(sizeof(double) * ny_ * nz_), static_cast<std::ptrdiff_t>(sizeof(double) * nz_),
                      static_cast<std::ptrdiff_t>(sizeof(double))};
    const stride_t sc{static_cast<std::ptrdiff_t>(sizeof(std::complex<double>) * ny_ * nz_),
                      static_cast<std::ptrdiff_t>(sizeof(std::complex<double>) * nz_),
                      static_cast<std::ptrdiff_t>(sizeof(std::complex<double>))};
    pocketfft::dst<double>(shape, sd, sd, {0, 1}, 1, a.data(), bbuf.data(), 1.0, false, 1);
    std::vector<std::complex<double>> c1(n3), c2(n3);
    for (size_t e = 0; e < n3; ++e) c1[e] = {bbuf[e], 0.0};
    pocketfft::c2c<double>(shape, sc, sc, {2}, true, c1.data(), c2.data(), 1.0, 1);
    std::vector<double> lx(nx_), ly(ny_), lz(nz_);
    for (int i = 0; i < nx_; ++i) {
        const double v = std::sin((i + 1) * (1.5707963267948966 / (nx_ + 1)));
        lx[i] = ((4.0 / dx_) / dx_) * v * v;
    }
    for (int j = 0; j < ny_; ++j) {
        const double v = std::sin((j + 1) * (1.5707963267948966 / (ny_ + 1)));
        ly[j] = ((4.0 / dy_) / dy_) * v * v;
    }
    for (int k = 0; k < nz_; ++k) {
        const double v = std::sin(k * (3.141592653589793 / nz_));
        lz[k] = ((4.0 / dz_) / dz_) * v * v;
    }
    for (int i = 0; i < nx_; ++i)
        for (int j = 0; j < ny_; ++j)
            for (int k = 0; k < nz_; ++k) {
                const double f = 112940906683.72818 / ((lx[i] + ly[j]) + lz[k]);
                auto& v = c2[at(i, j, k)];
                v = {v.real() * f, v.imag() * f};
            }
    pocketfft::c2c<double>(shape, sc, sc, {2}, false, c2.data(), c1.data(), 1.0, 1);
    for (size_t e = 0; e < n3; ++e) a[e] = c1[e].real();
    pocketfft::dst<double>(shape, sd, sd, {0, 1}, 1, a.data(), bbuf.data(), 1.0, false, 1);
    const double norm = 1.0 / (static_cast<double>(4LL * nx_ + 4) * (ny_ + 1) * nz_);
    for (size_t e = 0; e < n3; ++e) phi_[e] = bbuf[e] * norm;
    const double hx = 1.0 / (dx_ + dx_), hy = 1.0 / (dy_ + dy_), hz = 1.0 / (dz_ + dz_);
    for (int i = 0; i < nx_; ++i)
        for (int j = 0; j < ny_; ++j)
            for (int k = 0; k < nz_; ++k) {
                const double xp = i + 1 < nx_ ? phi_[at(i + 1, j, k)] : 0.0;
                const double xm = i > 0 ? phi_[at(i - 1, j, k)] : 0.0;
                const double yp = j + 1 < ny_ ? phi_[at(i, j + 1, k)] : 0.0;
                const double ym = j > 0 ? phi_[at(i, j - 1, k)] : 0.0;
                const double zp = phi_[at(i, j, (k + 1) % nz_)];
                const double zm = phi_[at(i, j, (k + nz_ - 1) % nz_)];
                ex_[at(i, j, k)] = -(xp - xm) * hx;
                ey_[at(i, j, k)] = -(yp - ym) * hy;
                ez_[at(i, j, k)] = -(zp - zm) * hz;
            }
}

// ---- SPICNIC ----
// Integral of 1/r over the box [x0,x1]x[y0,y1]x[z0,z1] (closed form, inclusion-exclusion over the
// corners of the antiderivative F = yz ln(x+r) + zx ln(y+r) + xy ln(z+r)
//   - x^2/2 atan(yz/(xr)) - y^2/2 atan(zx/(yr)) - z^2/2 atan(xy/(zr))).
static double box_F(double x, double y, double z) {
    const double r = std::sqrt(x * x + y * y + z * z);
    if (r == 0.0) return 0.0;
    auto lg = [](double u, double v, double w) { return (u == 0.0 || v + w <= 0.0) ? 0.0 : u * std::log(v + w); };
    auto at = [](double u, double num, double den) { return (u == 0.0 || den == 0.0) ? 0.0 : u * u * std::atan(num / den); };
    return lg(y * z, x, r) + lg(z * x, y, r) + lg(x * y, z, r) -
           0.5 * (at(x, y * z, x * r) + at(y, z * x, y * r) + at(z, x * y, z * r));
}

static double box_inv_r(double x0, double x1, double y0, double y1, double z0, double z1) {
    double s = 0.0;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            for (int k = 0; k < 2; ++k) {
                const double sign = ((i + j + k) % 2 == 0) ? -1.0 : 1.0;  // + at (x1, y1, z1)
                s += sign * box_F(i ? x1 : x0, j ? y1 : y0, k ? z1 : z0);
            }
    return s;
}

void SpaceCharge::tabulate() {
    const double hx = dx_ / 10.0, hy = dy_ / 10.0, hz = dz_ / 10.0;
    run_tasks(mx_, [&](int a) {
        for (int bb = 0; bb < my_; ++bb)
            for (int c = 0; c < mz_; ++c) {
                const int s2 = a * a + bb * bb + c * c;
                double g;
                if (!compat_) {
                    // corrected: exact cell average of 1/r for every entry (AVAS.dll: 10^3 midpoints
                    // inside a radius counted in cells, the point value 1/r outside, 0 for the own
                    // cell -- all wrong for elongated cells, e.g. long or dc bunches)
                    g = box_inv_r(a * dx_ - dx_ * 0.5, a * dx_ + dx_ * 0.5, bb * dy_ - dy_ * 0.5, bb * dy_ + dy_ * 0.5,
                                  c * dz_ - dz_ * 0.5, c * dz_ + dz_ * 0.5) / vol_;
                } else if (s2 == 0) {
                    g = 0.0;
                } else if (s2 > r2_) {
                    const double X = a * dx_, Y = bb * dy_, Z = c * dz_;
                    g = 1.0 / std::pow(X * X + Y * Y + Z * Z, 0.5);
                } else {
                    const double x0 = a * dx_ - dx_ * 0.5, y0 = bb * dy_ - dy_ * 0.5, z0 = c * dz_ - dz_ * 0.5;
                    double acc = 0.0;
                    for (int u = 0; u < 10; ++u) {
                        const double x = x0 + (u + 0.5) * hx;
                        for (int v = 0; v < 10; ++v) {
                            const double y = y0 + (v + 0.5) * hy;
                            for (int w = 0; w < 10; ++w) {
                                const double z = z0 + (w + 0.5) * hz;
                                acc = acc + hx * hy * hz * (1.0 / std::pow(x * x + y * y + z * z, 0.5));
                            }
                        }
                    }
                    g = acc / vol_;
                }
                for (int sa : {-1, 1})
                    for (int sb : {-1, 1})
                        for (int sc : {-1, 1}) G_[gat(nx_ + sa * a, ny_ + sb * bb, nz_ + sc * c)] = g;
            }
    });
}

void SpaceCharge::spicnic() {
    tabulate();
    const double thr = negligible_ * rho_[at(nx_ / 2, ny_ / 2, nz_ / 2)];
    const size_t n3 = rho_.size();
    // potential of the octant charge mirrored into the 8 octants, at target (i, j, k)
    auto sym = [&](int i, int j, int k) {
        double S = 0.0;
        const int seg[3][2][2] = {
            {{std::max(0, i - mx_), std::min(std::min(nx_, i + mx_), (nx_ + 1) / 2)}, {std::max(std::max(0, i - mx_), nx_ / 2), std::min(nx_, i + mx_)}},
            {{std::max(0, j - my_), std::min(std::min(ny_, j + my_), (ny_ + 1) / 2)}, {std::max(std::max(0, j - my_), ny_ / 2), std::min(ny_, j + my_)}},
            {{std::max(0, k - mz_), std::min(std::min(nz_, k + mz_), (nz_ + 1) / 2)}, {std::max(std::max(0, k - mz_), nz_ / 2), std::min(nz_, k + mz_)}}};
        for (int si = 0; si < 2; ++si)
            for (int sj = 0; sj < 2; ++sj)
                for (int sk = 0; sk < 2; ++sk)
                    for (int a = seg[0][si][0]; a < seg[0][si][1]; ++a) {
                        const int ra = si == 0 ? nx_ - 1 - a : a;
                        for (int bb = seg[1][sj][0]; bb < seg[1][sj][1]; ++bb) {
                            const int rb = sj == 0 ? ny_ - 1 - bb : bb;
                            for (int c = seg[2][sk][0]; c < seg[2][sk][1]; ++c) {
                                const int rc = sk == 0 ? nz_ - 1 - c : c;
                                const double r = rho_[at(ra, rb, rc)];
                                if (r > thr) S = r * G_[gat(nx_ - i + a, ny_ - j + bb, nz_ - k + c)] + S;
                            }
                        }
                    }
        return S;
    };
    if (compat_) {
        const int nt = static_cast<int>(sTasks_.size());
        std::vector<std::vector<double>> C(nt, std::vector<double>(n3, 0.0));
        run_tasks(nt, [&](int t) {
            const auto& T = sTasks_[t];
            for (int i = T[0]; i <= T[2]; ++i) {
                int j0, j1;
                if (T[0] == T[2]) { j0 = T[1]; j1 = T[3]; }
                else if (i == T[0]) { j0 = T[1]; j1 = ny_; }
                else if (i == T[2]) { j0 = static_cast<int>(nz_ * 0.5); j1 = T[3]; }
                else { j0 = static_cast<int>(nz_ * 0.5); j1 = ny_; }
                for (int j = j0; j < j1; ++j)
                    for (int k = static_cast<int>(nz_ * 0.5); k < nz_; ++k) {
                        const double v = sym(i, j, k) * kE;
                        const double r0 = rho_[at(i, j, k)];
                        for (int I : {nx_ - 1 - i, i})
                            for (int J : {ny_ - 1 - j, j})
                                for (int K : {nz_ - 1 - k, k}) {
                                    phi_[at(I, J, K)] = v;
                                    const double d = rho_[at(I, J, K)] - r0;
                                    if (thr <= d || d <= -thr) {  // assignment: the last one wins
                                        for (int a = 0; a < nx_; ++a)
                                            for (int bb = 0; bb < ny_; ++bb)
                                                for (int c = 0; c < nz_; ++c)
                                                    C[t][at(a, bb, c)] = G_[gat(nx_ + I - a, ny_ + J - bb, nz_ + K - c)] * d * kE;
                                    }
                                }
                    }
            }
        });
        for (size_t e = 0; e < n3; ++e) {
            double v = phi_[e];
            for (int t = 0; t < nt; ++t) v = v + C[t][e];
            phi_[e] = v;
        }
    } else {
        // corrected: every upper-octant target, and the asymmetric part of the charge added
        // (accumulated, not assigned) as sum over the mirror points of (rho(P) - rho_sym) G
        const int hi = nx_ / 2, hj = ny_ / 2, hk = nz_ / 2;
        std::vector<std::array<int, 4>> corr;  // I, J, K, target index
        std::vector<double> dval;
        std::vector<double> symv(n3, 0.0);
        run_tasks(nx_ - hi, [&](int ii) {
            const int i = hi + ii;
            for (int j = hj; j < ny_; ++j)
                for (int k = hk; k < nz_; ++k) symv[at(i, j, k)] = sym(i, j, k) * kE;
        });
        for (int i = hi; i < nx_; ++i)
            for (int j = hj; j < ny_; ++j)
                for (int k = hk; k < nz_; ++k) {
                    const double r0 = rho_[at(i, j, k)];
                    for (int I : {nx_ - 1 - i, i})
                        for (int J : {ny_ - 1 - j, j})
                            for (int K : {nz_ - 1 - k, k}) {
                                phi_[at(I, J, K)] = symv[at(i, j, k)];
                                const double d = rho_[at(I, J, K)] - r0;
                                if (d != 0.0 && (thr <= d || d <= -thr)) {
                                    corr.push_back({I, J, K, 0});
                                    dval.push_back(d);
                                }
                            }
                }
        run_tasks(nx_, [&](int a) {
            for (int bb = 0; bb < ny_; ++bb)
                for (int c = 0; c < nz_; ++c) {
                    double s = 0.0;
                    for (size_t q = 0; q < corr.size(); ++q)
                        s = G_[gat(nx_ + corr[q][0] - a, ny_ + corr[q][1] - bb, nz_ + corr[q][2] - c)] * dval[q] + s;
                    phi_[at(a, bb, c)] = s * kE + phi_[at(a, bb, c)];
                }
        });
    }
    const double hx = 1.0 / (dx_ + dx_), hy = 1.0 / (dy_ + dy_), hz = 1.0 / (dz_ + dz_);
    auto grad = [&](int i, int n, double pm, double p0, double pp, double h) {
        if (i == 0) return 2.0 * (-(pp - p0) * h);
        if (i == n - 1) return 2.0 * (-(p0 - pm) * h);
        return -(pp - pm) * h;
    };
    for (int i = 0; i < nx_; ++i)
        for (int j = 0; j < ny_; ++j)
            for (int k = 0; k < nz_; ++k) {
                const double p0 = phi_[at(i, j, k)];
                ex_[at(i, j, k)] = grad(i, nx_, i > 0 ? phi_[at(i - 1, j, k)] : 0.0, p0, i + 1 < nx_ ? phi_[at(i + 1, j, k)] : 0.0, hx);
                ey_[at(i, j, k)] = grad(j, ny_, j > 0 ? phi_[at(i, j - 1, k)] : 0.0, p0, j + 1 < ny_ ? phi_[at(i, j + 1, k)] : 0.0, hy);
                ez_[at(i, j, k)] = grad(k, nz_, k > 0 ? phi_[at(i, j, k - 1)] : 0.0, p0, k + 1 < nz_ ? phi_[at(i, j, k + 1)] : 0.0, hz);
            }
}

// ---- interpolation back to the particles (PIC::Allocate) ----
void SpaceCharge::allocate(Beam& b) {
    double Q = 0.0;
    for (int k = 0; k < nz_; ++k)
        for (int j = 0; j < ny_; ++j)
            for (int i = 0; i < nx_; ++i) Q = Q + rho_[at(i, j, k)];
    const double* c = b.centroid.data();
    double rs = 0.0, rc = 1.0;
    if (b.rotMode != 0) dll_sincos(b.rotAngle, rs, rc);
    const int n = b.n;
    for (int p = 0; p < n; ++p) {
        if (b.lost[p] == 1) {
            b.scx[p] = b.scy[p] = b.scz[p] = 0.0;
            continue;
        }
        if (out_[p] == 0) {
            const auto& w = w_[p];
            const int i = cell_[p][0], j = cell_[p][1], k = cell_[p][2];
            auto interp = [&](const std::vector<double>& g) {
                return g[at(i, j, k)] * w[0] + g[at(i + 1, j, k)] * w[1] + g[at(i, j + 1, k)] * w[2] +
                       g[at(i + 1, j + 1, k)] * w[3] + g[at(i, j, k + 1)] * w[4] + g[at(i + 1, j, k + 1)] * w[5] +
                       g[at(i, j + 1, k + 1)] * w[6] + g[at(i + 1, j + 1, k + 1)] * w[7];
            };
            const double Ez = interp(ez_), Ey = interp(ey_), Ex = interp(ex_);
            double sx = Ex * igamma_, sy = Ey * igamma_, sz = Ez;
            if (b.rotMode == 1) {
                const double ny = sy * rc - sz * rs, nz = sy * rs + sz * rc;
                sy = ny;
                sz = nz;
            } else if (b.rotMode != 0) {
                const double nx = sx * rc + sz * rs, nz = sz * rc - sx * rs;
                sx = nx;
                sz = nz;
            }
            b.scx[p] = sx;
            b.scy[p] = sy;
            b.scz[p] = sz;
        } else {
            const double ddx = b.rel[0][p] - c[0], ddy = b.rel[2][p] - c[2], ddz = b.rel[4][p] - c[4];
            const double r = std::sqrt(ddx * ddx + ddy * ddy + ddz * ddz);
            const double f = (((Q * kE) / r) / r) / r * igamma_;
            b.scx[p] = ddx * f;
            b.scy[p] = ddy * f;
            b.scz[p] = f * ddz * gamma_;
        }
    }
    if (rec_ == 1) record(b, Q);
}

// recordSCF: rest-frame mesh field (k, j, i order, binary) and one text line per call (appended)
void SpaceCharge::record(const Beam& b, double Q) {
    std::string bin;
    bin.reserve(rho_.size() * 24);
    for (int k = 0; k < nz_; ++k)
        for (int j = 0; j < ny_; ++j)
            for (int i = 0; i < nx_; ++i)
                for (const auto* g : {&ex_, &ey_, &ez_}) bin.append(reinterpret_cast<const char*>(&(*g)[at(i, j, k)]), 8);
    if (!write_file(paths().output + "SpaceChargeSet.plt", bin, true, true)) say("SpaceChargeSet.plt cannot be output             ");
    Text t;
    t.g(b.time).s(" ").g(sync_[4] + b.centroid[4]).s(" ").g(sync_[0] + b.centroid[0]).s(" ").g(sync_[2] + b.centroid[2]).s(" ")
        .i(nx_).s(" ").i(ny_).s(" ").i(nz_).s(" ").g(dx_).s(" ").g(dy_).s(" ").g(dz_).s(" ").g(Q).s(" ").g(gamma_).nl();
    write_file(paths().output + "SpaceChargeMesh.txt", t.str(), true);
}

void hesc_field_pre(Beam& b, const std::vector<std::array<double, 8>>& rows, double tRef, int threads) {
    const int n = b.n;
    if (compat_dll()) {
        // AVAS.dll (FUN_180078390): gamma from gamma*beta_z alone, v = beta c; OpenMP static
        // blocks of cores[2] threads, partial sums added in thread order.  Its second parallel
        // region (FUN_180090940) adds the squares straight into the shared Beam.rms array with
        // no lock or reduction: with cores[2] > 1 updates are lost at random (rms about half,
        // different every run).  That race cannot be reproduced; this is AVAS.dll with
        // cores[2] = 1 (spec 60 2.3, M4-notes).
        const int T = std::max(1, threads);
        double tot[7] = {0, 0, 0, 0, 0, 0, 0};
        const int chunk = n / T, rem = n % T;
        for (int t = 0; t < T; ++t) {
            const int i0 = t < rem ? t * (chunk + 1) : rem + t * chunk;
            const int i1 = i0 + (t < rem ? chunk + 1 : chunk);
            double part[7] = {0, 0, 0, 0, 0, 0, 0};
            for (int i = i0; i < i1; ++i) {
                const auto& r = rows[i];
                if (r[7] != 1.0) continue;
                part[0] = part[0] + 1.0;
                const double dt = r[6] - tRef;
                const double G = r[5] * r[5] + 1.0;
                const double be = std::sqrt(1.0 - 1.0 / G);
                const double cg = (1.0 / std::sqrt(G)) * k::c;
                b.rel[4][i] = be * dt * -k::c;
                b.rel[0][i] = r[0] - dt * r[1] * cg;
                b.rel[2][i] = r[2] - dt * r[3] * cg;
                b.rel[1][i] = r[1];
                b.rel[3][i] = r[3];
                b.rel[5][i] = r[5];
                for (int k = 0; k < 6; ++k) part[1 + k] = part[1 + k] + b.rel[k][i];
            }
            for (int k = 0; k < 7; ++k) tot[k] = tot[k] + part[k];
        }
        for (int k = 0; k < 6; ++k) b.centroid[k] = tot[1 + k] / tot[0];
        const double* c = b.centroid.data();
        const double gc = std::sqrt(c[5] * c[5] + 1.0);
        double q[3] = {0, 0, 0};
        for (int i = 0; i < n; ++i) {
            if (rows[i][7] != 1.0) continue;
            const double ax = b.rel[0][i] - c[0], ay = b.rel[2][i] - c[2], az = (b.rel[4][i] - c[4]) * gc;
            q[0] = ax * ax + q[0];
            q[1] = ay * ay + q[1];
            q[2] = az * az + q[2];
        }
        for (int k = 0; k < 3; ++k) b.rms[k] = std::sqrt(q[k] / tot[0]);
        return;
    }
    double s[6] = {0, 0, 0, 0, 0, 0};
    double cnt = 0.0;
    for (int i = 0; i < n; ++i) {
        const auto& r = rows[i];
        if (r[7] != 1.0) continue;
        const double dt = r[6] - tRef;
        const double g = std::sqrt(r[1] * r[1] + 1.0 + r[3] * r[3] + r[5] * r[5]);
        const double be = std::sqrt(1.0 - (1.0 / g) / g);
        const double xp = r[1] / r[5], yp = r[3] / r[5];
        const double vz = std::sqrt((be * k::c * be * k::c) / (xp * xp + yp * yp + 1.0));
        b.rel[4][i] = -dt * vz;
        b.rel[0][i] = r[0] - ((r[1] * dt) / g) * k::c;
        b.rel[2][i] = r[2] - ((dt * r[3]) / g) * k::c;
        b.rel[1][i] = r[1];
        b.rel[3][i] = r[3];
        b.rel[5][i] = r[5];
        cnt = cnt + 1.0;
        for (int k = 0; k < 6; ++k) s[k] = s[k] + b.rel[k][i];
    }
    for (int k = 0; k < 6; ++k) b.centroid[k] = s[k] / cnt;
    const double* c = b.centroid.data();
    const double gc = 1.0 / std::sqrt(1.0 - std::pow(c[5] / std::sqrt(c[1] * c[1] + 1.0 + c[3] * c[3] + c[5] * c[5]), 2.0));
    double q[3] = {0, 0, 0};
    for (int i = 0; i < n; ++i) {
        if (rows[i][7] != 1.0) continue;
        const double ax = b.rel[0][i] - c[0], ay = b.rel[2][i] - c[2], az = (b.rel[4][i] - c[4]) * gc;
        q[0] = q[0] + ax * ax;
        q[1] = q[1] + ay * ay;
        q[2] = q[2] + az * az;
    }
    for (int k = 0; k < 3; ++k) b.rms[k] = std::sqrt(q[k] / cnt);
}

}  // namespace avas
