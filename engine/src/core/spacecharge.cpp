#include "core/spacecharge.h"
#include "core/text.h"

#include <cstdio>

#include <cstring>
#include <cstdint>

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
#include "core/latview.h"
#include "core/reduce.h"
#include "core/sc_kernels.h"
#include "core/options.h"
#include "core/physics.h"
#include "core/workers.h"
#include "third_party/pocketfft/pocketfft_hdronly.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace avas {

namespace {

constexpr double kE = 8987551787.368176;  // 1/(4 pi eps0)

// Integer add from several threads (the fixed-point deposit; integer sums do not depend on the order).
inline void atomic_add(long long& x, long long v) {
#if defined(_MSC_VER)
    _InterlockedExchangeAdd64(reinterpret_cast<volatile __int64*>(&x), v);
#else
    __atomic_fetch_add(&x, v, __ATOMIC_RELAXED);
#endif
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
    } else if (s[0] == 1.0 || s[0] == 2.0) {
        // PICNIC (libPIC.so has no defaults: unset values are uninitialised there) takes the mesh
        // defaults of SPICNIC -- the mesh means the same in both (mean +- meshrms * rms)
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

// FFT normalisation 1/((4 Nx + 4)(Ny + 1) Nz).  AVAS.dll multiplies the denominator in 32-bit int
// (wraps for meshes of about 1024^3); the corrected engine in 64 bits.
double fft_norm(int nx, int ny, int nz, bool compat) {
    if (compat) {
        const std::uint32_t a = static_cast<std::uint32_t>(4 * nx + 4) * static_cast<std::uint32_t>(ny + 1) *
                                static_cast<std::uint32_t>(nz);
        return 1.0 / static_cast<double>(static_cast<std::int32_t>(a));
    }
    return 1.0 / (static_cast<double>(4LL * nx + 4) * (ny + 1) * nz);
}

SpaceCharge::SpaceCharge(const Params& p, int nParticles) : compat_(compat_dll()) {
    method_ = static_cast<int>(p.scPar[0]);
    nx_ = p.numOfGrid[0];
    ny_ = p.numOfGrid[1];
    nz_ = p.numOfGrid[2];
    for (int i = 0; i < 3; ++i) mr_[i] = p.scPar[1 + i];
    negligible_ = p.scPar[8];
    lp_ = p.lpBoundary;
    periodic_ = compat_ ? 0 : p.lpPeriodic;
    rec_ = p.recScField;
    imp_ = p.impScField == 1;
    if (imp_) {
        // PIC::Initial: index of the recorded solves (SpaceChargeMesh.txt of the input folder)
        std::string txt;
        if (!read_file(paths().input + "SpaceChargeMesh.txt", txt))
            throw EngineError("impscfield 1: SpaceChargeMesh.txt cannot be opened in the input folder");
        for (const std::string& ln : split_lines(normalize_line_endings(txt))) {
            const std::vector<std::string> w = split_words(ln);
            if (w.size() < 12) continue;
            itime_.push_back(parse_double(w[0]));
            izc_.push_back(parse_double(w[1]));
            ixy_.push_back({parse_double(w[2]), parse_double(w[3])});
            inx_ = parse_int(w[4]);
            iny_ = parse_int(w[5]);
            inz_ = parse_int(w[6]);
            id_.push_back({parse_double(w[7]), parse_double(w[8]), parse_double(w[9])});
            iQ_.push_back(parse_double(w[10]));
            igam_.push_back(parse_double(w[11]));
        }
        if (itime_.empty()) throw EngineError("impscfield 1: SpaceChargeMesh.txt has no record");
    }
    // AVAS.dll: lpboundary 1 asks for FFT, but scmethod overwrites it right after (no effect);
    // the corrected engine honours it
    if (lp_ == 1 && !compat_) method_ = 0;
    if (periodic_ == 1) {
        if (method_ != 0) say("lpboundary V1 1: the periodic longitudinal box uses the FFT solver");
        method_ = 0;
    }
    pool_ = static_cast<int>(p.scPar[9]) == 1;
    for (int k = 0; k < 10; ++k) cores_[k] = pool_ ? static_cast<int>(p.scPar[10 + k]) : 1;
    if (nx_ < 2 || ny_ < 2 || nz_ < 2) throw EngineError("numofgrid must be at least 2 in every direction");
    if (method_ == 1 && (nx_ % 2 || ny_ % 2 || nz_ % 2)) {
        // AVAS.dll loops forever on the mirror step 0 of an odd mesh (spec 60 2.7.4)
        throw EngineError("SPICNIC needs an even numofgrid in every direction");
    }
    mx_ = my_ = mz_ = static_cast<int>(std::round(nx_ * 3 * 0.25));
    if (!compat_ || method_ == 2) {
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
    if (method_ == 1 || method_ == 2) G_.assign(static_cast<size_t>(2 * nx_ + 1) * (2 * ny_ + 1) * (2 * nz_ + 1), 0.0);
    nThreads_ = std::max(1, options().threads > 0 ? options().threads : static_cast<int>(std::thread::hardware_concurrency()));
    if (nThreads_ > 1) workers_ = std::make_unique<WorkerPool>(nThreads_);
}

SpaceCharge::~SpaceCharge() = default;

void SpaceCharge::par(int n, const std::function<void(int)>& task) const {
    if (workers_ && n > 1) {
        workers_->run(n, task);
        return;
    }
    for (int i = 0; i < n; ++i) task(i);
}

void SpaceCharge::par_range(int n, const std::function<void(int, int)>& task) const {
    const int chunks = std::min(n, workers_ ? nThreads_ : 1);
    if (chunks <= 1) {
        if (n > 0) task(0, n);
        return;
    }
    par(chunks, [&](int c) {
        task(static_cast<int>(static_cast<long long>(n) * c / chunks), static_cast<int>(static_cast<long long>(n) * (c + 1) / chunks));
    });
}

bool SpaceCharge::begin(const Beam& b, const double* sync) {
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
    return true;
}

bool SpaceCharge::solve(Beam& b, double factor, const double* sync) {
    if (!begin(b, sync)) return false;
    weight(b, factor);
    if (method_ == 0) fft();
    else if (method_ == 2) picnic();
    else spicnic();
    allocate(b);
    return true;
}

// ---- mesh and CIC deposit (PIC::Weight) ----
void SpaceCharge::mesh(const Beam& b) {
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
    double cz = lp_ == 1 ? 0.0 : c[4];
    if (periodic_ == 1) {
        // M10 D7: the box is one RF period of the synchronous particle in the beam rest frame,
        // gamma_s beta_s lambda = lambda (gamma beta_z)_s, the stretch gamma_s about the synchronous particle
        const double* s = sync_;
        gamma_ = std::sqrt(s[1] * s[1] + s[3] * s[3] + s[5] * s[5] + 1.0);
        lpLen_ = (k::c / b.frequency) * s[5];
        if (!(lpLen_ > 0.0)) throw EngineError("lpboundary V1 1: the synchronous particle does not move forward");
        dz_ = lpLen_ / static_cast<double>(nz_);
        cz = 0.0;
    }
    const double cx = lp_ == 1 ? 0.0 : c[0], cy = lp_ == 1 ? 0.0 : c[2];
    for (int i = 0; i < nx_; ++i) xs_[i] = ((i - nx_ * 0.5) + 0.5) * dx_ + cx;
    for (int j = 0; j < ny_; ++j) ys_[j] = ((j - ny_ * 0.5) + 0.5) * dy_ + cy;
    for (int k = 0; k < nz_; ++k) zs_[k] = ((k - nz_ * 0.5) + 0.5) * dz_ + cz;
    vol_ = dy_ * dx_ * dz_;
    igamma_ = 1.0 / gamma_;
}

kern::MeshV SpaceCharge::mesh_view(const Beam& b) const {
    kern::MeshV M{};
    M.nx = nx_;
    M.ny = ny_;
    M.nz = nz_;
    M.dx = dx_;
    M.dy = dy_;
    M.dz = dz_;
    M.vol = vol_;
    M.gamma = gamma_;
    M.igamma = igamma_;
    M.c0 = b.centroid[0];
    M.c2 = b.centroid[2];
    M.c4 = b.centroid[4];
    M.lpRel = (lp_ == 1 && (!compat_ || (pool_ && cores_[4] != 1))) || periodic_ == 1 ? 1 : 0;
    M.lpc = lp_ == 1 && (!compat_ || (pool_ && cores_[9] != 1)) ? 1 : 0;
    M.lpWrap = periodic_;
    M.lpLen = lpLen_;
    M.compat = compat_ ? 1 : 0;
    M.xs = xs_.data();
    M.ys = ys_.data();
    M.zs = zs_.data();
    M.Q = 0.0;
    M.rotMode = b.rotMode;
    M.rs = 0.0;
    M.rc = 1.0;
    if (b.rotMode != 0) dll_sincos(b.rotAngle, M.rs, M.rc);
    M.fixScaleExp = fixExp_;
    return M;
}

// Fixed-point scale of the corrected deposit: |sum| <= n max|q| <= 2^61 (charges never change).
void SpaceCharge::init_fix(const Beam& b) {
    if (fixInit_) return;
    double mq = 0.0;
    for (int p = 0; p < b.n; ++p) {
        const double a = std::fabs(b.charge[p]);
        if (mq < a) mq = a;
    }
    const double B = mq * static_cast<double>(b.n);
    int e = 0;
    std::frexp(B, &e);  // B < 2^e
    fixExp_ = B > 0.0 ? 61 - e : 0;
    fixInit_ = true;
}

void SpaceCharge::set_rho(const std::vector<long long>& acc, double factor, bool threaded) {
    const int e3 = static_cast<int>(-fixExp_);
    auto convert = [&](int e0, int e1) {
        for (int e = e0; e < e1; ++e) rho_[e] = pm::scale2(static_cast<double>(acc[e]), e3);
        if (factor != 1.0)
            for (int e = e0; e < e1; ++e) rho_[e] = rho_[e] * factor;
    };
    if (threaded) par_range(static_cast<int>(rho_.size()), convert);
    else convert(0, static_cast<int>(rho_.size()));
}

double SpaceCharge::total_charge() const {
    double Q = 0.0;
    for (int k = 0; k < nz_; ++k)
        for (int j = 0; j < ny_; ++j)
            for (int i = 0; i < nx_; ++i) Q = Q + rho_[at(i, j, k)];
    return Q;
}

void SpaceCharge::weight(Beam& b, double factor) {
    mesh(b);
    const double* c = b.centroid.data();
    const int n = b.n;
    if (!compat_) {
        // corrected engine: every deposit is an exact 64-bit integer (charge * 2^e), so the sum
        // does not depend on the order -- the same on any thread count, GPU or number of GPUs
        init_fix(b);
        fix_.resize(rho_.size());
        par_range(static_cast<int>(fix_.size()), [&](int e0, int e1) { std::fill(fix_.begin() + e0, fix_.begin() + e1, 0LL); });
        const kern::MeshV M = mesh_view(b);
        const kern::PartV P = beam_view(b);
        // threads: private integer meshes when the mesh is small against the deposits, else atomic adds
        // into the one mesh (integer sums: the same result either way)
        const bool threaded = workers_ && n >= 2048;
        const int tasks = threaded ? std::min(nThreads_, (n + 1023) / 1024) : 1;
        const bool privateMeshes = threaded && fix_.size() * static_cast<size_t>(tasks) <= static_cast<size_t>(n) * 8;
        auto deposit_into = [&](int p0, int p1, long long* mesh, bool atomic) {
            for (int p = p0; p < p1; ++p) {
                if (b.lost[p] == 1) continue;
                int i, j, k;
                double w[8];
                if (!kern::mesh_cell(M, P, p, i, j, k, w)) continue;
                const double q = b.charge[p];
                for (int m = 0; m < 8; ++m) {
                    long long& cell = mesh[kern::mesh_corner(M, i, j, k, m)];
                    const long long v = kern::fix_charge(w[m] * q, fixExp_);
                    if (atomic) atomic_add(cell, v);
                    else cell += v;
                }
            }
        };
        if (privateMeshes) {
            if (fixLoc_.size() < static_cast<size_t>(tasks)) fixLoc_.resize(static_cast<size_t>(tasks));
            par(tasks, [&](int t) {
                std::vector<long long>& loc = fixLoc_[static_cast<size_t>(t)];
                loc.assign(fix_.size(), 0LL);
                deposit_into(static_cast<int>(static_cast<long long>(n) * t / tasks), static_cast<int>(static_cast<long long>(n) * (t + 1) / tasks),
                             loc.data(), false);
            });
            par_range(static_cast<int>(fix_.size()), [&](int e0, int e1) {
                for (int e = e0; e < e1; ++e) {
                    long long s = 0;
                    for (int t = 0; t < tasks; ++t) s += fixLoc_[static_cast<size_t>(t)][static_cast<size_t>(e)];
                    fix_[static_cast<size_t>(e)] = s;
                }
            });
            set_rho(fix_, factor, true);
            return;
        }
        if (threaded) par_range(n, [&](int p0, int p1) { deposit_into(p0, p1, fix_.data(), true); });
        else deposit_into(0, n, fix_.data(), false);
        set_rho(fix_, factor, true);
        return;
    }
    const bool lpRel = lp_ == 1 && (!compat_ || (pool_ && cores_[4] != 1));
    auto deposit = [&](int i0, int i1, std::vector<double>& g) {
        for (int p = i0; p < i1; ++p) {
            if (b.lost[p] == 1) continue;
            const double x = b.rel[0][p], y = b.rel[2][p];
            // lpboundary 1: AVAS.dll stretches about the synchronous particle only in the threaded
            // deposit (Calculate_Weight, cores[4] > 1), the serial one stretches about the centroid
            const double zr = lpRel ? b.rel[4][p] * gamma_ + c[4] : (b.rel[4][p] - c[4]) * gamma_ + c[4];
            const int i = static_cast<int>(std::floor((x - xs_[0]) / dx_));
            const int j = static_cast<int>(std::floor((y - ys_[0]) / dy_));
            const int k = static_cast<int>(std::floor((zr - zs_[0]) / dz_));
            if (0 <= i && i < nx_ - 1 && 0 <= j && j < ny_ - 1 && 0 <= k && k < nz_ - 1) {
                double w[8];
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
            }
        }
    };
    std::fill(rho_.begin(), rho_.end(), 0.0);
    // AVAS.dll: particle segments of cores[4] tasks into private meshes (serial when
    // multithreading 0 or one task)
    const std::vector<int> bounds = (pool_ && cores_[4] != 1) ? wBounds_ : std::vector<int>{0, n};
    const int nb = static_cast<int>(bounds.size()) - 1;
    if (nb == 1) {
        deposit(0, n, rho_);
    } else {
        if (loc_.size() != static_cast<size_t>(nb)) loc_.assign(static_cast<size_t>(nb), std::vector<double>());
        par(nb, [&](int t) {
            loc_[t].assign(rho_.size(), 0.0);
            deposit(bounds[t], bounds[t + 1], loc_[t]);
        });
        par_range(static_cast<int>(rho_.size()), [&](int e0, int e1) {
            for (int e = e0; e < e1; ++e) {
                double s = 0.0;
                for (int t = 0; t < nb; ++t) s = s + loc_[t][e];
                rho_[e] = s;
            }
        });
    }
    if (factor < 0.999999999)
        par_range(static_cast<int>(rho_.size()), [&](int e0, int e1) {
            for (int e = e0; e < e1; ++e) rho_[e] = rho_[e] * (1.0 - factor);
        });
}

// ---- FFT Poisson solver: DST-I in x, y (grounded walls), periodic FFT in z ----
void SpaceCharge::fft() {
    // Every transform is split into independent lines (DST in x, y for each z index; DFT in z for each
    // (x, y)) and every loop into independent nodes: the same arithmetic as one thread, bit for bit.
    const size_t n3 = rho_.size();
    fa_.resize(n3);
    fb_.resize(n3);
    fc1_.resize(n3);
    fc2_.resize(n3);
    double* a = fa_.data();
    double* bb = fb_.data();
    std::complex<double>* c1 = fc1_.data();
    std::complex<double>* c2 = fc2_.data();
    const size_t plane = static_cast<size_t>(ny_) * nz_;
    auto nodes = [&](const std::function<void(size_t, size_t)>& f) {
        par_range(nx_, [&](int i0, int i1) { f(static_cast<size_t>(i0) * plane, static_cast<size_t>(i1) * plane); });
    };
    const double s = ((1.0 / dx_) / dy_) / dz_;
    nodes([&](size_t e0, size_t e1) {
        for (size_t e = e0; e < e1; ++e) a[e] = rho_[e] * s;
    });
    using pocketfft::shape_t;
    using pocketfft::stride_t;
    const stride_t sd{static_cast<std::ptrdiff_t>(sizeof(double) * ny_ * nz_), static_cast<std::ptrdiff_t>(sizeof(double) * nz_),
                      static_cast<std::ptrdiff_t>(sizeof(double))};
    const stride_t sc{static_cast<std::ptrdiff_t>(sizeof(std::complex<double>) * ny_ * nz_),
                      static_cast<std::ptrdiff_t>(sizeof(std::complex<double>) * nz_),
                      static_cast<std::ptrdiff_t>(sizeof(std::complex<double>))};
    auto dst_xy = [&](const double* in, double* out) {
        par_range(nz_, [&](int k0, int k1) {
            const shape_t sh{static_cast<size_t>(nx_), static_cast<size_t>(ny_), static_cast<size_t>(k1 - k0)};
            pocketfft::dst<double>(sh, sd, sd, {0, 1}, 1, in + k0, out + k0, 1.0, false, 1);
        });
    };
    auto dft_z = [&](const std::complex<double>* in, std::complex<double>* out, bool forward) {
        par_range(nx_, [&](int i0, int i1) {
            const shape_t sh{static_cast<size_t>(i1 - i0), static_cast<size_t>(ny_), static_cast<size_t>(nz_)};
            const size_t o = static_cast<size_t>(i0) * plane;
            pocketfft::c2c<double>(sh, sc, sc, {2}, forward, in + o, out + o, 1.0, 1);
        });
    };
    dst_xy(a, bb);
    nodes([&](size_t e0, size_t e1) {
        for (size_t e = e0; e < e1; ++e) c1[e] = {bb[e], 0.0};
    });
    dft_z(c1, c2, true);
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
    par(nx_, [&](int i) {
        for (int j = 0; j < ny_; ++j)
            for (int k = 0; k < nz_; ++k) {
                const double f = 112940906683.72818 / ((lx[i] + ly[j]) + lz[k]);
                auto& v = c2[at(i, j, k)];
                v = {v.real() * f, v.imag() * f};
            }
    });
    dft_z(c2, c1, false);
    nodes([&](size_t e0, size_t e1) {
        for (size_t e = e0; e < e1; ++e) a[e] = c1[e].real();
    });
    dst_xy(a, bb);
    const double norm = fft_norm(nx_, ny_, nz_, compat_);
    nodes([&](size_t e0, size_t e1) {
        for (size_t e = e0; e < e1; ++e) phi_[e] = bb[e] * norm;
    });
    const double hx = 1.0 / (dx_ + dx_), hy = 1.0 / (dy_ + dy_), hz = 1.0 / (dz_ + dz_);
    par(nx_, [&](int i) {
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
    });
}

// ---- SPICNIC ----
// Integral of 1/r over the box [x0,x1]x[y0,y1]x[z0,z1] (closed form, inclusion-exclusion over the
// corners of the antiderivative F = yz ln(x+r) + zx ln(y+r) + xy ln(z+r)
//   - x^2/2 atan(yz/(xr)) - y^2/2 atan(zx/(yr)) - z^2/2 atan(xy/(zr)), sck::box_F).
void SpaceCharge::tabulate() {
    if (!compat_) {
        // corrected: exact cell average of 1/r for every entry (AVAS.dll: 10^3 midpoints inside a
        // radius counted in cells, the point value 1/r outside, 0 for the own cell -- all wrong for
        // elongated cells, e.g. long or dc bunches).  The antiderivative is evaluated once at the
        // (mx+1)(my+1)(mz+1) cell corners (q - 1/2) h and shared by the neighbouring cells.
        const int ax = mx_ + 1, ay = my_ + 1, az = mz_ + 1;
        const sck::SpGrid g{nx_, ny_, nz_, mx_, my_, mz_};
        std::vector<double> F(static_cast<size_t>(ax) * ay * az);
        par(ax, [&](int q) {
            for (int r = 0; r < ay; ++r)
                for (int t = 0; t < az; ++t)
                    F[(static_cast<size_t>(q) * ay + r) * az + t] = sck::green_corner(q, r, t, dx_, dy_, dz_);
        });
        par(mx_, [&](int a) {
            for (int bb = 0; bb < my_; ++bb)
                for (int c = 0; c < mz_; ++c) sck::green_cell(g, F.data(), vol_, a, bb, c, G_.data());
        });
        return;
    }
    const double hx = dx_ / 10.0, hy = dy_ / 10.0, hz = dz_ / 10.0;
    par(mx_, [&](int a) {
        for (int bb = 0; bb < my_; ++bb)
            for (int c = 0; c < mz_; ++c) {
                const int s2 = a * a + bb * bb + c * c;
                double g;
                if (s2 == 0) {
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

// SPICNIC host part of the corrected engine: Green table, threshold, and the mirror points whose
// charge differs from the octant-symmetric charge (I, J, K, d), in the order of the targets.
void SpaceCharge::spicnic_prepare(std::vector<int>& corr, std::vector<double>& dval, double& thr) {
    tabulate();
    thr = negligible_ * rho_[at(nx_ / 2, ny_ / 2, nz_ / 2)];
    corr.clear();
    dval.clear();
    // the targets and mirror points in the order of the device code (sck::corr_value)
    const sck::SpGrid g{nx_, ny_, nz_, mx_, my_, mz_};
    const long long nt = g.n_targets();
    for (long long t = 0; t < nt; ++t)
        for (int m = 0; m < 8; ++m) {
            int I, J, K;
            const double d = sck::corr_value(g, rho_.data(), thr, t, m, I, J, K);
            if (d != 0.0) {
                corr.push_back(I);
                corr.push_back(J);
                corr.push_back(K);
                dval.push_back(d);
            }
        }
}

// PICNIC (spec 65): the potential of the whole mesh charge by direct summation at every node
// (sck::picnic_node, shared with the GPU back ends), then the field by central differences.
void SpaceCharge::picnic() {
    tabulate();
    const sck::SpGrid g{nx_, ny_, nz_, mx_, my_, mz_};
    par(nx_, [&](int i) {
        for (int j = 0; j < ny_; ++j)
            for (int k = 0; k < nz_; ++k) phi_[at(i, j, k)] = sck::picnic_node<double>(g, rho_.data(), G_.data(), i, j, k);
    });
    const double hx = 1.0 / (dx_ + dx_), hy = 1.0 / (dy_ + dy_), hz = 1.0 / (dz_ + dz_);
    par(nx_, [&](int i) {
        for (int j = 0; j < ny_; ++j)
            for (int k = 0; k < nz_; ++k)
                sck::grad_spicnic_node(g, phi_.data(), hx, hy, hz, i, j, k, ex_.data(), ey_.data(), ez_.data());
    });
}

void SpaceCharge::fft_tables(std::vector<double>& lx, std::vector<double>& ly, std::vector<double>& lz, double& scale,
                             double& norm) const {
    scale = ((1.0 / dx_) / dy_) / dz_;
    lx.resize(nx_);
    ly.resize(ny_);
    lz.resize(nz_);
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
    norm = fft_norm(nx_, ny_, nz_, compat_);
}

void SpaceCharge::spicnic() {
    std::vector<int> corr;
    std::vector<double> dval;
    double thr = 0.0;
    if (compat_) {
        tabulate();
        thr = negligible_ * rho_[at(nx_ / 2, ny_ / 2, nz_ / 2)];
    } else {
        spicnic_prepare(corr, dval, thr);
    }
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
        par(nt, [&](int t) {
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
        std::vector<double> symv(n3, 0.0);
        par(nx_ - hi, [&](int ii) {
            const int i = hi + ii;
            for (int j = hj; j < ny_; ++j)
                for (int k = hk; k < nz_; ++k) symv[at(i, j, k)] = sym(i, j, k) * kE;
        });
        for (int i = hi; i < nx_; ++i)
            for (int j = hj; j < ny_; ++j)
                for (int k = hk; k < nz_; ++k)
                    for (int I : {nx_ - 1 - i, i})
                        for (int J : {ny_ - 1 - j, j})
                            for (int K : {nz_ - 1 - k, k}) phi_[at(I, J, K)] = symv[at(i, j, k)];
        const size_t nc = dval.size();
        par(nx_, [&](int a) {
            for (int bb = 0; bb < ny_; ++bb)
                for (int c = 0; c < nz_; ++c) {
                    double s = 0.0;
                    for (size_t q = 0; q < nc; ++q)
                        s = G_[gat(nx_ + corr[3 * q] - a, ny_ + corr[3 * q + 1] - bb, nz_ + corr[3 * q + 2] - c)] * dval[q] + s;
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
    par(nx_, [&](int i) {
        for (int j = 0; j < ny_; ++j)
            for (int k = 0; k < nz_; ++k) {
                const double p0 = phi_[at(i, j, k)];
                ex_[at(i, j, k)] = grad(i, nx_, i > 0 ? phi_[at(i - 1, j, k)] : 0.0, p0, i + 1 < nx_ ? phi_[at(i + 1, j, k)] : 0.0, hx);
                ey_[at(i, j, k)] = grad(j, ny_, j > 0 ? phi_[at(i, j - 1, k)] : 0.0, p0, j + 1 < ny_ ? phi_[at(i, j + 1, k)] : 0.0, hy);
                ez_[at(i, j, k)] = grad(k, nz_, k > 0 ? phi_[at(i, j, k - 1)] : 0.0, p0, k + 1 < nz_ ? phi_[at(i, j, k + 1)] : 0.0, hz);
            }
    });
}

// ---- interpolation back to the particles (PIC::Allocate) ----
void SpaceCharge::allocate(Beam& b) {
    const double Q = total_charge();
    kern::MeshV M = mesh_view(b);
    M.Q = Q;
    const kern::PartV P = beam_view(b);
    auto interp = [&](int p0, int p1) {
        for (int p = p0; p < p1; ++p) kern::mesh_interp(M, P, p, ex_.data(), ey_.data(), ez_.data(), kE);
    };
    if (b.n >= 4096) par_range(b.n, interp);
    else interp(0, b.n);
    if (rec_ == 1) record(b, Q);
}

// importSCF (spec 60 2.9.3): the recorded rest-frame field, shifted by whole RF periods in time
void SpaceCharge::import_scf(Beam& b) {
    const int n = b.n;
    const double T = 1.0 / b.frequency;
    double t = b.time;
    auto zero = [&](int p) { b.scx[p] = b.scy[p] = b.scz[p] = 0.0; };
    if (t < itime_[0]) {
        for (int p = 0; p < n; ++p) zero(p);
        return;
    }
    while (T < t - itime_[0]) {
        t = t - T;
        b.time = t;  // AVAS.dll folds the simulation time back into the first period
    }
    std::vector<double> q;
    for (double v = t; v < itime_.back(); v = v + T) q.push_back(v);
    if (q.empty()) {
        if (compat_) compat_abort("Vector is empty!");
        throw EngineError("impscfield 1: the simulation time is past the last recorded space-charge field");
    }
    // monotonicSearch: times[a] < q <= times[a + 1], continued from the previous position
    std::vector<double> zq;
    std::vector<int> rr;
    size_t a = 0;
    for (double v : q) {
        int ia = -1, ib = -1;
        double ta = 0.0, tb = 0.0;
        while (a + 1 < itime_.size()) {
            if (itime_[a] < v && v <= itime_[a + 1]) {
                ia = static_cast<int>(a);
                ib = ia + 1;
                ta = itime_[a];
                tb = itime_[a + 1];
                break;
            }
            ++a;
        }
        if (ia < 0) {
            // AVAS.dll reads index -1 here (t equal to the first record time)
            if (!compat_) throw EngineError("impscfield 1: no recorded field around the simulation time");
            ia = ib = 0;
        }
        zq.push_back(((v - ta) / (tb - ta)) * (izc_[static_cast<size_t>(ib)] - izc_[static_cast<size_t>(ia)]) +
                     izc_[static_cast<size_t>(ia)]);
        rr.push_back(tb - v < v - ta ? ib : ia);
    }
    std::string set;
    if (!read_file(paths().input + "SpaceChargeSet.plt", set))
        throw EngineError("Unable to open file: " + paths().input + "SpaceChargeSet.plt");
    const long long nnode = static_cast<long long>(inx_) * iny_ * inz_;
    for (int p = 0; p < n; ++p) {
        if (b.lost[p] == 1) {
            zero(p);
            continue;
        }
        const double z = b.z[p];
        // findClosest: lower_bound, the previous entry when it is not farther
        size_t m = static_cast<size_t>(std::lower_bound(zq.begin(), zq.end(), z) - zq.begin());
        double zc;
        if (zq.size() == 1 || m == 0) {
            m = 0;
            zc = zq[0];
        } else if (m == zq.size()) {
            m = zq.size() - 1;
            zc = zq.back();
        } else {
            zc = zq[m];
            if (std::fabs(zq[m - 1] - z) <= std::fabs(zq[m] - z)) {
                m -= 1;
                zc = zq[m];
            }
        }
        const size_t r = static_cast<size_t>(rr[m]);
        const auto& d = id_[r];
        const double g = igam_[r];
        const double x0 = ixy_[r][0] - (inx_ * 0.5 - 0.5) * d[0];
        const double y0 = ixy_[r][1] - (iny_ * 0.5 - 0.5) * d[1];
        const double z0 = zc - (inz_ * 0.5 - 0.5) * d[2];
        const double dzs = (z - zc) * g;
        const double zr = dzs + zc;
        const int i = static_cast<int>(std::floor((b.x[p] - x0) / d[0]));
        const int j = static_cast<int>(std::floor((b.y[p] - y0) / d[1]));
        const int k = static_cast<int>(std::floor((zr - z0) / d[2]));
        if (i < inx_ - 1 && j < iny_ - 1 && k < inz_ - 1 && i > -1 && j > -1 && k > -1) {
            const double iv = 1.0 / (d[1] * d[0] * d[2]);
            double w[8];
            for (int mm = 0; mm < 8; ++mm) {
                const int kk = k == inz_ - 1 ? (mm > 3 ? 0 : -k) : (mm < 4 ? 1 : 0);
                const int jj = (mm & 3) < 2 ? 1 : 0;
                const int ii = (~mm) & 1;
                w[mm] = std::fabs((b.y[p] - (static_cast<double>(jj + j) * d[1] + y0)) *
                                  (b.x[p] - (static_cast<double>(ii + i) * d[0] + x0)) *
                                  (zr - (static_cast<double>(k + kk) * d[2] + z0))) * iv;
            }
            const int node[8][3] = {{i, j, k}, {i + 1, j, k}, {i, j + 1, k}, {i + 1, j + 1, k},
                                    {i, j, k + 1}, {i + 1, j, k + 1}, {i, j + 1, k + 1}, {i + 1, j + 1, k + 1}};
            double ex = 0.0, ey = 0.0, ez = 0.0;
            for (int mm = 0; mm < 8; ++mm) {
                const long long off =
                    ((static_cast<long long>((node[mm][2] * iny_ + node[mm][1]) * inx_ + node[mm][0]) * 3) +
                     nnode * static_cast<long long>(r) * 3) * 8;
                double f[3] = {0, 0, 0};
                if (off >= 0 && off + 24 <= static_cast<long long>(set.size())) {
                    std::memcpy(f, set.data() + off, 24);
                } else {
                    std::fprintf(stderr, "Read failed: Location %lld\n", off);
                }
                ex = ex + w[mm] * f[0];
                ey = ey + w[mm] * f[1];
                ez = ez + f[2] * w[mm];
            }
            b.scx[p] = ex / g;
            b.scy[p] = ey / g;
            b.scz[p] = ez;
        } else {
            const double dx = b.x[p] - ixy_[r][0], dy = b.y[p] - ixy_[r][1];
            const double r2 = dy * dy + dx * dx + dzs * dzs;
            const double f = (kE / (std::sqrt(r2) * r2 * g)) * iQ_[r];
            b.scx[p] = f * dx;
            b.scy[p] = f * dy;
            b.scz[p] = f * dzs * g;
        }
    }
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
    // corrected engine: sums in the fixed order of reduce.h
    kern::PartV P = beam_view(b);
    P.rec = const_cast<double*>(rows.empty() ? nullptr : rows[0].data());
    auto serial = [](int ng, const std::function<void(int)>& fn) {
        for (int g = 0; g < ng; ++g) fn(g);
    };
    double s[kern::kHescSums];
    fixed_sum<kern::kHescSums>(n, serial, [&](int i, double* c) { kern::hesc_pass1(P, i, tRef, c); }, s);
    const double cnt = s[6];
    for (int k = 0; k < 6; ++k) b.centroid[k] = s[k] / cnt;
    const double* c = b.centroid.data();
    const double gc = 1.0 / std::sqrt(1.0 - std::pow(c[5] / std::sqrt(c[1] * c[1] + 1.0 + c[3] * c[3] + c[5] * c[5]), 2.0));
    double q[3];
    fixed_sum<3>(n, serial, [&](int i, double* cc) { kern::hesc_pass2(P, i, c, gc, cc); }, q);
    for (int k = 0; k < 3; ++k) b.rms[k] = std::sqrt(q[k] / cnt);
}

}  // namespace avas
