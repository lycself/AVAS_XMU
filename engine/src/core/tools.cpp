// AVAS.dll-only tools (spec 80 3-5): sampling, meshing and dataprocess.  They do not touch the
// simulation: they compress TraceWin .dst distributions into weighted macro particles and turn
// pairs of simulation results into binary training files.  compat=dll keeps the original's exits
// (exit(1) / exit(0) end the host process); by default errors are returned as non-zero codes.
#include "core/tools.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/text.h"

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace avas {

namespace {

struct Dst {
    char h0 = 0, h1 = 0, h2 = 0;
    std::int32_t n = 0;
    double ib = 0.0, f = 0.0, mc2 = 0.0;
    std::vector<std::array<double, 6>> p;
};

const char* kOpen = "The initial bunch distribution file cannot be opened, check particleDistributionPath";

struct ToolError {
    int code;
};

[[noreturn]] void fail(const std::string& msg, int code) {
    say(msg);
    if (compat_dll()) compat_exit(code);
    throw ToolError{code == 0 ? 2 : code};
}

bool read_dst(const std::string& path, Dst& d) {
    std::string raw;
    if (!read_file(path, raw) || raw.size() < 23) return false;
    d.h0 = raw[0];
    d.h1 = raw[1];
    std::memcpy(&d.n, raw.data() + 2, 4);
    std::memcpy(&d.ib, raw.data() + 6, 8);
    std::memcpy(&d.f, raw.data() + 14, 8);
    d.h2 = raw[22];
    if (d.n < 0 || raw.size() < 23 + 48 * static_cast<size_t>(d.n)) return false;
    d.p.resize(static_cast<size_t>(d.n));
    for (int i = 0; i < d.n; ++i) std::memcpy(d.p[static_cast<size_t>(i)].data(), raw.data() + 23 + 48 * i, 48);
    if (raw.size() >= 23 + 48 * static_cast<size_t>(d.n) + 8) std::memcpy(&d.mc2, raw.data() + 23 + 48 * d.n, 8);
    return true;
}

// written header: h2, h0, N, Ib, f, h2 (the original drops h1)
void write_dst(const std::string& path, const Dst& d, const std::vector<std::array<double, 6>>& p,
               const std::vector<double>& w) {
    std::string out;
    out.push_back(d.h2);
    out.push_back(d.h0);
    const std::int32_t n = static_cast<std::int32_t>(p.size());
    out.append(reinterpret_cast<const char*>(&n), 4);
    out.append(reinterpret_cast<const char*>(&d.ib), 8);
    out.append(reinterpret_cast<const char*>(&d.f), 8);
    out.push_back(d.h2);
    for (const auto& r : p) out.append(reinterpret_cast<const char*>(r.data()), 48);
    out.append(reinterpret_cast<const char*>(&d.mc2), 8);
    if (!write_file(path, out, false, true)) say("output error");
    std::string wb;
    wb.append(reinterpret_cast<const char*>(&n), 4);
    for (double v : w) wb.append(reinterpret_cast<const char*>(&v), 8);
    if (!write_file(path + "b", wb, false, true)) say("output error");
}

// floor as the original writes it: truncation, minus one for negative non-integers
int ffloor(double v) {
    const long long t = static_cast<long long>(v);
    if (static_cast<double>(t) != v && v < 0.0) return static_cast<int>(t - 1);
    return static_cast<int>(t);
}

std::vector<int> argsort(const std::vector<double>& v) {
    std::vector<int> idx(v.size());
    for (size_t i = 0; i < v.size(); ++i) idx[i] = static_cast<int>(i);
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return v[static_cast<size_t>(a)] < v[static_cast<size_t>(b)]; });
    return idx;
}

std::vector<int> ranks(const std::vector<double>& v) {
    const std::vector<int> o = argsort(v);
    std::vector<int> r(v.size());
    for (size_t k = 0; k < o.size(); ++k) r[static_cast<size_t>(o[k])] = static_cast<int>(k);
    return r;
}

// Twiss groups of spec 80 3.3 from the 13 second moments
std::array<std::array<double, 6>, 7> twiss7(const double* m) {
    std::array<std::array<double, 6>, 7> t{};
    auto set = [&](int g, double det, double cxy, double bnum, double s1, double s2) {
        const double e = std::sqrt(det);
        const double a = -cxy / e;
        const double be = bnum / e;
        t[static_cast<size_t>(g)] = {e, a, be, (a * a + 1.0) / be, s1, s2};
    };
    set(0, m[0] * m[1] - m[2] * m[2], m[2], m[0], std::sqrt(m[0]), std::sqrt(m[1]));
    set(1, m[3] * m[4] - m[5] * m[5], m[5], m[3], std::sqrt(m[3]), std::sqrt(m[4]));
    set(2, m[6] * m[7] - m[8] * m[8], m[8], m[6], std::sqrt(m[6]), std::sqrt(m[7]));
    set(3, m[3] * m[0] - m[9] * m[9], m[9], m[0], std::sqrt(m[0]), std::sqrt(m[3]));
    set(4, m[4] * m[0] - m[10] * m[10], m[10], m[0], std::sqrt(m[0]), std::sqrt(m[4]));
    set(5, m[3] * m[1] - m[11] * m[11], m[11], m[3], std::sqrt(m[3]), std::sqrt(m[1]));
    set(6, m[4] * m[1] - m[12] * m[12], m[12], m[1], std::sqrt(m[1]), std::sqrt(m[4]));
    return t;
}

double halo(double u4, double p4, double u2p2, double up3, double u3p, double u2, double p2, double up) {
    return std::sqrt(3.0 * u4 * p4 + 9.0 * u2p2 * u2p2 - 12.0 * up3 * u3p) / (2.0 * u2 * p2 - 2.0 * up * up) - 2.0;
}

}  // namespace

// ---------------- meshing (spec 80 4) ----------------
int tool_meshing(const std::string& in, const std::string& out, int n, double mag) {
    try {
        Dst d;
        if (!read_dst(in, d)) fail(kOpen, 1);
        say(Text().i(d.n).s(" Meshing......").str());
        const int N = d.n;
        double mu[6] = {0, 0, 0, 0, 0, 0}, sg[6] = {0, 0, 0, 0, 0, 0};
        for (const auto& r : d.p)
            for (int k = 0; k < 6; ++k) mu[k] = mu[k] + r[static_cast<size_t>(k)];
        for (int k = 0; k < 6; ++k) mu[k] = mu[k] / N;
        for (const auto& r : d.p)
            for (int k = 0; k < 6; ++k) {
                const double dv = r[static_cast<size_t>(k)] - mu[k];
                sg[k] = sg[k] + dv * dv;
            }
        double h[6];
        std::vector<double> c[6];
        for (int k = 0; k < 6; ++k) {
            sg[k] = std::sqrt(sg[k] / N);
            h[k] = (sg[k] + sg[k]) * mag * (1.0 / (n - 1));
            c[k].resize(static_cast<size_t>(n));
            for (int i = 0; i < n; ++i) c[k][static_cast<size_t>(i)] = ((i - n * 0.5) + 0.5) * h[k] + mu[k];
        }
        if (n < 2 || std::pow(static_cast<double>(n), 6) > 4e8) fail("meshing: the number of nodes is out of range", 1);
        const size_t nn = static_cast<size_t>(n);
        std::vector<double> G(nn * nn * nn * nn * nn * nn, 0.0);
        const double iv = 1.0 / (h[5] * h[4] * h[3] * h[2] * h[0] * h[1]);
        for (const auto& r : d.p) {
            int ii[6];
            bool ok = true;
            for (int k = 0; k < 6; ++k) {
                ii[k] = ffloor((r[static_cast<size_t>(k)] - c[k][0]) / h[k]);
                if (ii[k] >= n - 1 || ii[k] < 0) ok = false;
            }
            if (!ok) continue;
            for (int o = 0; o < 64; ++o) {
                int bit[6];
                for (int k = 0; k < 6; ++k) bit[k] = (o >> (5 - k)) & 1;
                double w = 1.0;
                for (int k = 0; k < 6; ++k) w = w * std::fabs(r[static_cast<size_t>(k)] - c[k][static_cast<size_t>(ii[k] + bit[k])]);
                w = w * iv;
                size_t at = 0;
                for (int k = 0; k < 6; ++k) at = at * nn + static_cast<size_t>(ii[k] + (1 - bit[k]));
                G[at] = G[at] + w;
            }
        }
        std::vector<std::array<double, 6>> P;
        std::vector<double> W;
        for (size_t at = 0; at < G.size(); ++at) {
            if (G[at] == 0.0) continue;
            size_t rem = at;
            std::array<double, 6> q{};
            for (int k = 5; k >= 0; --k) {
                q[static_cast<size_t>(k)] = c[k][rem % nn];
                rem /= nn;
            }
            P.push_back(q);
            W.push_back(G[at]);
        }
        write_dst(out, d, P, W);
        say("Mesh completed. Number of extracted particles is: " + std::to_string(P.size()));
        return 0;
    } catch (const ToolError& e) {
        return e.code;
    }
}

// ---------------- sampling (spec 80 3) ----------------
namespace {

struct Sample {
    std::vector<int> pos;     // positions in the sorted order
    std::vector<double> w;    // weights
    std::vector<int> cnt;     // count per segment
};

std::vector<double> seg_weights(int S, double M, double ct, bool final) {
    std::vector<double> w(static_cast<size_t>(S));
    w[0] = M;
    if (!final) {
        for (int k = 0; k < S - 2; ++k) w[static_cast<size_t>(k + 1)] = std::pow(M, -1.0 / (S - 2)) * w[static_cast<size_t>(k)];
        w[static_cast<size_t>(S - 1)] = std::pow(M, -1.0 / (S - 2)) * w[static_cast<size_t>(S - 2)] * (1.0 / ct);
    } else {
        for (int k = 0; k < S - 2; ++k) w[static_cast<size_t>(k + 1)] = w[static_cast<size_t>(k)] * (1.0 / std::pow(M, 1.0 / (S - 2)));
        w[static_cast<size_t>(S - 1)] = w[static_cast<size_t>(S - 2)] / (ct * std::pow(M, 1.0 / (S - 2)));
    }
    return w;
}

Sample draw(int N, int S, const std::vector<double>& w, int T, bool check) {
    Sample s;
    std::vector<int> nk(static_cast<size_t>(S), N / S);
    int sum = 0;
    for (int v : nk) sum += v;
    nk[static_cast<size_t>(S - 1)] += N - sum;
    double D = 0.0;
    for (int k = 0; k < S; ++k) D = D + nk[static_cast<size_t>(k)] / (T * w[static_cast<size_t>(k)]);
    double start = 0.0;
    s.cnt.assign(static_cast<size_t>(S), 0);
    for (int k = 0; k < S; ++k) {
        const int K = static_cast<int>(std::round(nk[static_cast<size_t>(k)] / (D * w[static_cast<size_t>(k)])));
        for (int j = 0; j < K; ++j) {
            const int idx = static_cast<int>(std::round((j + 0.5) * w[static_cast<size_t>(k)] * D + start));
            if (check && idx >= N) break;
            s.pos.push_back(idx);
            s.w.push_back(w[static_cast<size_t>(k)]);
            s.cnt[static_cast<size_t>(k)] += 1;
        }
        start = static_cast<int>(start + nk[static_cast<size_t>(k)]);
    }
    return s;
}

}  // namespace

int tool_sampling(const std::string& in, const std::string& out, int p3, int p4, int p5, int p6, int p7, int p8,
                  double p9, double ct) {
    try {
        write_file("./SPTest.txt", "", false);
        say("The inFile Name has been set to " + in);
        say("The outFile Name has been set to " + out);
        Dst d;
        if (!read_dst(in, d)) fail(kOpen, 1);
        const int N = d.n;
        say(Text().i(N).s(" sorting......").str());
        const double c = 299792458.0;
        std::vector<std::array<double, 6>> P = d.p;
        double phiMin = 9999999.0;
        for (const auto& r : d.p) phiMin = std::min(phiMin, r[4]);
        double sum6[6] = {0, 0, 0, 0, 0, 0}, sumW = 0.0;
        for (int i = 0; i < N; ++i) {
            auto& q = P[static_cast<size_t>(i)];
            const auto& r = d.p[static_cast<size_t>(i)];
            const double g = r[5] / d.mc2 + 1.0;
            const double vz = std::sqrt(std::pow(c * std::sqrt(1.0 - 1.0 / (g * g)), 2.0) / (r[1] * r[1] + r[3] * r[3] + 1.0));
            const double dt = (1e-06 / (d.f * 6.283185307179586)) * (r[4] - phiMin);
            q[4] = vz * dt * -1000.0;
            q[5] = vz * 3.3356409519815204e-09 * g;
            q[0] = (r[0] * 0.01 - dt * r[1] * vz) * 1000.0;
            q[2] = (r[2] * 0.01 - dt * r[3] * vz) * 1000.0;
            q[1] = r[1] * 1000.0;
            q[3] = r[3] * 1000.0;
            for (int k = 0; k < 6; ++k) sum6[k] = sum6[k] + q[static_cast<size_t>(k)];
            sumW = sumW + r[5];
        }
        double mean[6];
        for (int k = 0; k < 6; ++k) mean[k] = sum6[k] / N;
        const double W0 = sumW / N;
        for (auto& q : P) {
            q[5] = (q[5] - mean[5]) / mean[5] * 1000.0;
            for (int k = 0; k < 5; ++k) q[static_cast<size_t>(k)] = q[static_cast<size_t>(k)] - mean[k];
        }
        auto moments = [&](const std::vector<int>& sel, const std::vector<double>& ws, bool centred, double* m,
                           double* H, double* means, double& Wsum) {
            Wsum = 0.0;
            double mu[6] = {0, 0, 0, 0, 0, 0};
            for (size_t a = 0; a < sel.size(); ++a) {
                const double w = ws.empty() ? 1.0 : ws[a];
                Wsum = Wsum + w;
                for (int k = 0; k < 6; ++k) mu[k] = mu[k] + w * P[static_cast<size_t>(sel[a])][static_cast<size_t>(k)];
            }
            for (int k = 0; k < 6; ++k) mu[k] = mu[k] / Wsum;
            if (means)
                for (int k = 0; k < 6; ++k) means[k] = mu[k];
            for (int k = 0; k < 13; ++k) m[k] = 0.0;
            double h[3][8] = {};
            for (size_t a = 0; a < sel.size(); ++a) {
                const double w = ws.empty() ? 1.0 : ws[a];
                const auto& q = P[static_cast<size_t>(sel[a])];
                double v[6];
                for (int k = 0; k < 6; ++k) v[k] = centred ? q[static_cast<size_t>(k)] - mu[k] : q[static_cast<size_t>(k)];
                const double pr[13] = {v[0] * v[0], v[1] * v[1], v[0] * v[1], v[2] * v[2], v[3] * v[3], v[2] * v[3],
                                       v[4] * v[4], v[5] * v[5], v[4] * v[5], v[0] * v[2], v[0] * v[3], v[1] * v[2], v[1] * v[3]};
                for (int k = 0; k < 13; ++k) m[k] = m[k] + w * pr[k];
                for (int pl = 0; pl < 3; ++pl) {
                    // halo moments: raw (not centred on the sample mean)
                    const double u = q[static_cast<size_t>(2 * pl)], pp = q[static_cast<size_t>(2 * pl + 1)];
                    const double t[8] = {u * u * u * u, pp * pp * pp * pp, u * u * pp * pp, u * pp * pp * pp,
                                         u * u * u * pp, u * u, pp * pp, u * pp};
                    for (int k = 0; k < 8; ++k) h[pl][k] = h[pl][k] + w * t[k];
                }
            }
            for (int k = 0; k < 13; ++k) m[k] = m[k] / Wsum;
            for (int pl = 0; pl < 3; ++pl) {
                double hh[8];
                for (int k = 0; k < 8; ++k) hh[k] = h[pl][k] / Wsum;
                H[pl] = halo(hh[0], hh[1], hh[2], hh[3], hh[4], hh[5], hh[6], hh[7]);
            }
        };
        std::vector<int> all(static_cast<size_t>(N));
        for (int i = 0; i < N; ++i) all[static_cast<size_t>(i)] = i;
        double m0[13], H0[3], Wtot;
        moments(all, {}, false, m0, H0, nullptr, Wtot);
        const auto T0 = twiss7(m0);
        // 6-D normalised radius order
        std::vector<double> rr(static_cast<size_t>(N));
        const double sgs[6] = {T0[0][4], T0[0][5], T0[1][4], T0[1][5], T0[2][4], T0[2][5]};
        for (int i = 0; i < N; ++i) {
            double s = 0.0;
            for (int k = 0; k < 6; ++k) {
                const double v = P[static_cast<size_t>(i)][static_cast<size_t>(k)] / sgs[k];
                s = s + v * v;
            }
            rr[static_cast<size_t>(i)] = std::sqrt(s);
        }
        const std::vector<int> I = argsort(rr);
        // original 3-D density mesh
        say("Meshing......");
        const int n = p8;
        const double sx = std::sqrt(m0[0]), sy = std::sqrt(m0[3]), sz = std::sqrt(m0[6]);
        const double hx = 2.0 * sx * p9 * (1.0 / (n - 1)), hy = 2.0 * sy * p9 * (1.0 / (n - 1)), hz = 2.0 * sz * p9 * (1.0 / (n - 1));
        std::vector<double> cx(static_cast<size_t>(n)), cy(static_cast<size_t>(n)), cz(static_cast<size_t>(n));
        for (int i = 0; i < n; ++i) {
            cx[static_cast<size_t>(i)] = ((i - n * 0.5) + 0.5) * hx;
            cy[static_cast<size_t>(i)] = ((i - n * 0.5) + 0.5) * hy;
            cz[static_cast<size_t>(i)] = ((i - n * 0.5) + 0.5) * hz;
        }
        const size_t nn = static_cast<size_t>(n);
        std::vector<std::array<int, 3>> cell(static_cast<size_t>(N), std::array<int, 3>{-1, -1, -1});
        std::vector<std::array<double, 8>> cw(static_cast<size_t>(N), std::array<double, 8>{});
        std::vector<double> G0(nn * nn * nn, 0.0);
        const double ivol = 1.0 / (hy * hx * hz);
        for (int i = 0; i < N; ++i) {
            const auto& q = P[static_cast<size_t>(i)];
            const int ix = ffloor((q[0] - cx[0]) * (1.0 / hx)), iy = ffloor((q[2] - cy[0]) * (1.0 / hy)),
                      iz = ffloor((q[4] - cz[0]) * (1.0 / hz));
            if (ix < 0 || iy < 0 || iz < 0 || ix > n - 2 || iy > n - 2 || iz > n - 2) continue;
            cell[static_cast<size_t>(i)] = {ix, iy, iz};
            for (int kk = 0; kk < 8; ++kk) {
                const double g = std::fabs(q[0] - cx[static_cast<size_t>(ix + (1 - (kk & 1)))]) *
                                 std::fabs(q[2] - cy[static_cast<size_t>(iy + ((kk & 3) < 2 ? 1 : 0))]) *
                                 std::fabs(q[4] - cz[static_cast<size_t>(iz + (kk < 4 ? 1 : 0))]) * ivol;
                cw[static_cast<size_t>(i)][static_cast<size_t>(kk)] = g;
                const int ox = kk & 1, oy = (kk >> 1) & 1, oz = kk >> 2;
                const size_t at = (static_cast<size_t>(ix + ox) * nn + static_cast<size_t>(iy + oy)) * nn + static_cast<size_t>(iz + oz);
                G0[at] = G0[at] + g;
            }
        }
        say("Searching......");
        const int T = p3 - 2;
        struct Row {
            double S, M, o, mis, mse, dmean, dhalo;
        };
        std::vector<Row> rows;
        for (int S = p4; S >= p5; --S) {
            for (int M = p6; M >= p7; --M) {
                const std::vector<double> w = seg_weights(S, M, ct, false);
                Sample s = draw(N, S, w, T, true);
                for (int k = 0; k < S; ++k)
                    if (s.cnt[static_cast<size_t>(k)] == 0) {
                        fail("Error! The Magnification is too large " + std::to_string(S) + " " + std::to_string(M) + " " +
                                 std::to_string(k),
                             0);
                    }
                const double A = std::min(0.8 * s.pos.front() - 3.0, 0.8 * (N - s.pos.back()) - 3.0);
                std::vector<int> offs;
                if (A >= 1.0) {
                    const int ra = static_cast<int>(std::round(A));
                    for (int o = -ra; o <= ra; ++o) offs.push_back(o);
                } else {
                    offs.push_back(0);
                }
                std::vector<double> vm, vs, vd, vh;
                for (int o : offs) {
                    std::vector<int> sel;
                    std::vector<double> ws = s.w;
                    for (int pz : s.pos) sel.push_back(I[static_cast<size_t>(pz + o)]);
                    sel.push_back(I[0]);
                    sel.push_back(I[static_cast<size_t>(N - 1)]);
                    ws.push_back(w[static_cast<size_t>(S - 1)]);
                    ws.push_back(w[static_cast<size_t>(S - 1)]);
                    double ms[13], Hs[3], mu[6], Ws;
                    moments(sel, ws, true, ms, Hs, mu, Ws);
                    const auto Ts = twiss7(ms);
                    double Mk[7];
                    for (int g = 0; g < 7; ++g) {
                        const double R = (Ts[static_cast<size_t>(g)][1] - T0[static_cast<size_t>(g)][1]) *
                                             (Ts[static_cast<size_t>(g)][1] - T0[static_cast<size_t>(g)][1]) -
                                         (Ts[static_cast<size_t>(g)][2] - T0[static_cast<size_t>(g)][2]) *
                                             (Ts[static_cast<size_t>(g)][3] - T0[static_cast<size_t>(g)][3]);
                        Mk[g] = std::sqrt((R + std::sqrt(R * (R + 4.0))) * 0.5 + 1.0) - 1.0;
                    }
                    const double mis = (Mk[0] + Mk[1] + Mk[2] + Mk[3] + Mk[4] + Mk[5] + Mk[6]) / 9.0 + Mk[2] * 2.0 / 9.0;
                    std::vector<double> Gs(G0.size(), 0.0);
                    for (size_t a = 0; a < sel.size(); ++a) {
                        const int pi = sel[a];
                        const auto& ce = cell[static_cast<size_t>(pi)];
                        if (ce[0] < 0) continue;
                        const double f = ws[a] * (static_cast<double>(N) / Ws);
                        for (int kk = 0; kk < 8; ++kk) {
                            const int ox = kk & 1, oy = (kk >> 1) & 1, oz = kk >> 2;
                            const size_t at = (static_cast<size_t>(ce[0] + ox) * nn + static_cast<size_t>(ce[1] + oy)) * nn +
                                              static_cast<size_t>(ce[2] + oz);
                            Gs[at] = Gs[at] + f * cw[static_cast<size_t>(pi)][static_cast<size_t>(kk)];
                        }
                    }
                    double se = 0.0;
                    for (size_t at = 0; at < G0.size(); ++at) se = se + (Gs[at] - G0[at]) * (Gs[at] - G0[at]);
                    const double mse = std::sqrt(se / static_cast<double>(nn * nn * nn));
                    double wbar = 0.0;
                    for (size_t a = 0; a < sel.size(); ++a) wbar = wbar + ws[a] * d.p[static_cast<size_t>(sel[a])][5];
                    wbar = wbar / Ws;
                    double dm = std::fabs(wbar - W0);
                    for (int k = 0; k < 6; ++k) dm = dm + std::fabs(mu[k]);
                    double dh = 0.0;
                    for (int pl = 0; pl < 3; ++pl) dh = dh + std::fabs(Hs[pl] - H0[pl]) / H0[pl];
                    vm.push_back(mis);
                    vs.push_back(mse);
                    vd.push_back(dm);
                    vh.push_back(dh);
                }
                const std::vector<int> r1 = ranks(vm), r2 = ranks(vs), r3 = ranks(vd), r4 = ranks(vh);
                double best = 9999999.0;
                size_t bo = 0;
                for (size_t a = 0; a < offs.size(); ++a) {
                    const double sr = static_cast<double>(r1[a] + r2[a] + r3[a] + r4[a]);
                    if (sr < best) {
                        best = sr;
                        bo = a;
                    }
                }
                rows.push_back({static_cast<double>(S), static_cast<double>(M), static_cast<double>(offs[bo]), vm[bo], vs[bo],
                                vd[bo], vh[bo]});
                Text t;
                t.i(S).s("   ").i(M).s(" ").fixed(ct, 12).s("   ").fixed(static_cast<double>(offs[bo]), 12).s("   ").fixed(vm[bo], 12).s("   ")
                    .fixed(vs[bo], 12).s("  ").fixed(vd[bo], 12).s(" ").fixed(vh[bo], 12).s(" ").fixed(A, 12).nl();
                write_file("./SPTest.txt", t.str(), true);
            }
        }
        if (rows.empty()) fail("sampling: the search range of the segments or magnification is empty", 1);
        std::vector<double> a1, a2, a3, a4;
        for (const Row& r : rows) {
            a1.push_back(r.mis);
            a2.push_back(r.mse);
            a3.push_back(r.dmean);
            a4.push_back(r.dhalo);
        }
        const std::vector<int> q1 = ranks(a1), q2 = ranks(a2), q3 = ranks(a3), q4 = ranks(a4);
        double best = 999999.0;
        size_t br = 0;
        for (size_t a = 0; a < rows.size(); ++a) {
            const double sr = static_cast<double>(q1[a] + q2[a] + q3[a] + q4[a]);
            if (sr < best) {
                best = sr;
                br = a;
            }
        }
        const Row& R = rows[br];
        {
            Text t;
            t.s("segments: ").g(R.S).s(" Magnification: ").g(R.M).s(" offset: ").g(R.o).s(" rankMismatch: ").i(q1[br])
                .s(" rankMse: ").i(q2[br]).s(" rankMean: ").i(q3[br]).s(" rankeHalo: ").i(q4[br]).s(" Mismatch: ").g(R.mis)
                .s(" MSE: ").g(R.mse).s(" DeltaMean: ").g(R.dmean).s(" DeltaHalo: ").g(R.dhalo);
            say(t.str());
            Text f;
            f.s("segments: ").fixed(R.S, 12).s(" Magnification: ").fixed(R.M, 12).s(" offset: ").fixed(R.o, 12)
                .s("terminalCoefficient: ").fixed(ct, 12).s(" rankMismatch: ").i(q1[br]).s(" rankMse: ").i(q2[br])
                .s(" rankMean: ").i(q3[br]).s(" rankeHalo: ").i(q4[br]).s(" Mismatch: ").fixed(R.mis, 12).s(" MSE: ")
                .fixed(R.mse, 12).s(" DeltaMean: ").fixed(R.dmean, 12).s(" DeltaHalo: ").fixed(R.dhalo, 12).nl();
            write_file("./SPTest.txt", f.str(), true);
        }
        const int S = static_cast<int>(std::round(R.S));
        const double M = std::round(R.M);
        const int o = static_cast<int>(R.o);
        const std::vector<double> w = seg_weights(S, M, ct, true);
        const Sample s = draw(N, S, w, T, !compat_dll());
        std::vector<std::array<double, 6>> outP;
        std::vector<double> outW;
        for (size_t a = 0; a < s.pos.size(); ++a) {
            const int pz = s.pos[a] + o;
            if (pz < 0 || pz >= N) {
                if (compat_dll()) continue;
                continue;
            }
            outP.push_back(d.p[static_cast<size_t>(I[static_cast<size_t>(pz)])]);
            outW.push_back(s.w[a]);
        }
        outP.push_back(d.p[static_cast<size_t>(I[0])]);
        outW.push_back(w[static_cast<size_t>(S - 1)]);
        outP.push_back(d.p[static_cast<size_t>(I[static_cast<size_t>(N - 1)])]);
        outW.push_back(w[static_cast<size_t>(S - 1)]);
        write_dst(out, d, outP, outW);
        say(Text().s("Search completed, mismatch is: ").g(R.mis).str());
        return 0;
    } catch (const ToolError& e) {
        return e.code;
    }
}

// ---------------- dataprocess (spec 80 5) ----------------
namespace {

std::vector<std::string> list_dst(const std::string& dir) {
    std::vector<std::string> out;
#if defined(_WIN32)
    _finddata_t fd;
    const intptr_t h = _findfirst((dir + "*.dst").c_str(), &fd);
    if (h != -1) {
        do {
            const std::string n = fd.name;
            if (n != "." && n != "..") out.push_back(n);
        } while (_findnext(h, &fd) == 0);
        _findclose(h);
    }
#else
    if (DIR* d = opendir(dir.c_str())) {
        while (dirent* e = readdir(d)) {
            const std::string n = e->d_name;
            if (n.size() > 4 && n.compare(n.size() - 4, 4, ".dst") == 0) out.push_back(n);
        }
        closedir(d);
    }
    std::sort(out.begin(), out.end());
#endif
    return out;
}

double file_z(const std::string& name) {
    const size_t u = name.find('_');
    if (u == std::string::npos) return 0.0;
    return std::strtod(name.substr(u + 1, 8).c_str(), nullptr);
}

void process_index(const std::string& root, int i, int nF, int nO, double mag, int useW) {
    const std::string dF = root + "/OutputFile_" + std::to_string(i) + "/FiveCube/";
    const std::string dO = root + "/OutputFile_" + std::to_string(i) + "/original/";
    std::vector<std::string> files = list_dst(dO);
    std::vector<double> zs;
    for (const auto& f : files) zs.push_back(file_z(f));
    // selection sort by z (the original)
    for (size_t a = 0; a + 1 < files.size(); ++a) {
        size_t m = a;
        for (size_t b = a + 1; b < files.size(); ++b)
            if (zs[b] < zs[m]) m = b;
        std::swap(zs[a], zs[m]);
        std::swap(files[a], files[m]);
    }
    const std::string outPath = root + "/preprocessing_dataset/OutputFile_" + std::to_string(i) + ".ppj";
    std::string out;
    auto put = [&](const void* p, size_t n) { out.append(reinterpret_cast<const char*>(p), n); };
    const std::int32_t nf32 = static_cast<std::int32_t>(files.size()), nF32 = nF, nO32 = nO;
    put(&nf32, 4);
    put(&nF32, 4);
    put(&nO32, 4);
    put(&mag, 8);
    if (files.empty()) {
        if (!write_file(outPath, out, false, true)) fail(outPath + " open failed, Check permissions", 1);
        return;
    }
    Dst d0;
    if (!read_dst(dO + files[0], d0)) fail(std::string(kOpen) + ": " + dO + files[0], 1);
    std::string wraw;
    if (!read_file(dF + files[0] + "b", wraw) || wraw.size() < 4)
        fail("The initial bunch distribution b file cannot be opened, check particleDistributionPath: " + dF + files[0] + "b", 1);
    std::int32_t nw = 0;
    std::memcpy(&nw, wraw.data(), 4);
    double sw0 = 0.0;
    for (int k = 0; k < nw && 4 + 8 * static_cast<size_t>(k) + 8 <= wraw.size(); ++k) {
        double v;
        std::memcpy(&v, wraw.data() + 4 + 8 * k, 8);
        sw0 = sw0 + v;
    }
    const double sF = static_cast<double>(d0.n) / sw0;
    for (size_t j = 0; j < files.size(); ++j) {
        double sig[6] = {0, 0, 0, 0, 0, 0};
        for (int ds = 0; ds < 2; ++ds) {
            const bool five = ds == 0;
            const std::string path = (five ? dF : dO) + files[j];
            Dst d;
            if (!read_dst(path, d)) fail(std::string(kOpen) + ": " + path, 1);
            std::vector<double> w(static_cast<size_t>(d.n), 1.0);
            if (five && useW == 1) {
                std::string wr;
                if (!read_file(path + "b", wr) || wr.size() < 4)
                    fail("The initial bunch distribution b file cannot be opened, check particleDistributionPath: " + path + "b", 1);
                std::int32_t m = 0;
                std::memcpy(&m, wr.data(), 4);
                w.assign(static_cast<size_t>(m), 1.0);
                for (int k = 0; k < m && 4 + 8 * static_cast<size_t>(k) + 8 <= wr.size(); ++k)
                    std::memcpy(&w[static_cast<size_t>(k)], wr.data() + 4 + 8 * k, 8);
            }
            const int np = static_cast<int>(std::min(w.size(), d.p.size()));
            double W = 0.0;
            for (int k = 0; k < np; ++k) W = W + w[static_cast<size_t>(k)];
            const double s = five ? sF : 1.0;
            std::vector<double> vz(static_cast<size_t>(np)), gam(static_cast<size_t>(np));
            double swp = 0.0, swW = 0.0, swv = 0.0;
            std::vector<std::array<double, 6>> q(static_cast<size_t>(np));
            for (int k = 0; k < np; ++k) {
                const auto& r = d.p[static_cast<size_t>(k)];
                const double wk = w[static_cast<size_t>(k)];
                const double g = r[5] / d.mc2 + 1.0;
                gam[static_cast<size_t>(k)] = g;
                vz[static_cast<size_t>(k)] = (299792458.0 / std::sqrt(r[1] * r[1] + r[3] * r[3] + 1.0)) * std::sqrt(1.0 - 1.0 / (g * g));
                swp = swp + wk * r[4];
                swW = swW + wk * r[5];
                swv = swv + wk * vz[static_cast<size_t>(k)];
                q[static_cast<size_t>(k)] = {r[0] * 10.0, r[1] * 1000.0, r[2] * 10.0, r[3] * 1000.0, 0.0, 0.0};
            }
            const double Wb = swW * (1.0 / W);
            const double gb = Wb / d.mc2 + 1.0;
            const double bb = std::sqrt(1.0 - 1.0 / (gb * gb));
            const double phib = (1.0 / W) * swp;
            const double K = gb * (swv * (1.0 / W));  // AVAS.dll scales the sum first
            for (int k = 0; k < np; ++k) {
                const auto& r = d.p[static_cast<size_t>(k)];
                q[static_cast<size_t>(k)][4] = (0.001 / (d.f * 6.283185307179586)) * (phib - r[4]) * vz[static_cast<size_t>(k)];
                q[static_cast<size_t>(k)][5] = (vz[static_cast<size_t>(k)] * gam[static_cast<size_t>(k)] - K) * (1.0 / K) * 100.0;
            }
            double mu[6] = {0, 0, 0, 0, 0, 0};
            for (int k = 0; k < np; ++k)
                for (int c = 0; c < 6; ++c) mu[c] = mu[c] + w[static_cast<size_t>(k)] * q[static_cast<size_t>(k)][static_cast<size_t>(c)];
            for (double& v : mu) v = v / W;
            double m2[3][3] = {}, mxy = 0.0;
            for (int k = 0; k < np; ++k) {
                const double wk = w[static_cast<size_t>(k)];
                for (int pl = 0; pl < 3; ++pl) {
                    const double u = q[static_cast<size_t>(k)][static_cast<size_t>(2 * pl)] - mu[2 * pl];
                    const double up = q[static_cast<size_t>(k)][static_cast<size_t>(2 * pl + 1)] - mu[2 * pl + 1];
                    m2[pl][0] = u * u * wk + m2[pl][0];
                    m2[pl][1] = up * up * wk + m2[pl][1];
                    m2[pl][2] = up * u * wk + m2[pl][2];
                }
                mxy = (q[static_cast<size_t>(k)][2] - mu[2]) * (q[static_cast<size_t>(k)][0] - mu[0]) * wk + mxy;
            }
            for (auto& r : m2)
                for (double& v : r) v = v / W;
            mxy = mxy / W;
            const double Nd = d.n;
            for (int pl = 0; pl < 3; ++pl) {
                const double e = std::sqrt(m2[pl][0] * m2[pl][1] - m2[pl][2] * m2[pl][2]);
                const double v[9] = {bb * e * gb, (-1.0 / e) * m2[pl][2], m2[pl][0] / e, mu[2 * pl], mu[2 * pl + 1],
                                     std::sqrt(m2[pl][0]), std::sqrt(m2[pl][1]), Wb, Nd};
                put(v, sizeof v);
            }
            {
                const double e = std::sqrt(m2[0][0] * m2[1][0] - mxy * mxy);
                const double v[9] = {bb * e * gb, (-1.0 / e) * mxy, m2[0][0] / e, mu[0], mu[2], std::sqrt(m2[0][0]), std::sqrt(m2[1][0]), Wb, Nd};
                put(v, sizeof v);
            }
            if (five) {
                for (int pl = 0; pl < 3; ++pl) {
                    sig[2 * pl] = std::sqrt(m2[pl][0]);
                    sig[2 * pl + 1] = std::sqrt(m2[pl][1]);
                }
            }
            const double zj = zs[j];
            put(&zj, 8);
            put(&d.ib, 8);
            const int nd = five ? nF : nO;
            const int pairs[4][2] = {{0, 1}, {2, 3}, {4, 5}, {0, 2}};
            for (const auto& pr : pairs) {
                const int a = pr[0], b = pr[1];
                const double hu = (2.0 * sig[a]) * mag * (1.0 / (nd - 1)), hv = (2.0 * sig[b]) * mag * (1.0 / (nd - 1));
                std::vector<double> U(static_cast<size_t>(nd)), V(static_cast<size_t>(nd));
                for (int ii = 0; ii < nd; ++ii) {
                    U[static_cast<size_t>(ii)] = (ii - (nd * 0.5 - 0.5)) * hu + mu[a];
                    V[static_cast<size_t>(ii)] = (ii - (nd * 0.5 - 0.5)) * hv + mu[b];
                }
                std::vector<double> g(static_cast<size_t>(nd) * static_cast<size_t>(nd), 0.0);
                const double iv = 1.0 / (hv * hu);
                for (int k = 0; k < np; ++k) {
                    const double u = q[static_cast<size_t>(k)][static_cast<size_t>(a)], v = q[static_cast<size_t>(k)][static_cast<size_t>(b)];
                    const int ix = ffloor((u - U[0]) * (1.0 / hu)), iy = ffloor((v - V[0]) * (1.0 / hv));
                    if (ix < 0 || iy < 0 || ix > nd - 2 || iy > nd - 2) continue;
                    const double wk = w[static_cast<size_t>(k)];
                    auto at = [&](int x, int y) -> double& { return g[static_cast<size_t>(x) * static_cast<size_t>(nd) + static_cast<size_t>(y)]; };
                    at(ix, iy) += std::fabs((v - V[static_cast<size_t>(iy + 1)]) * (u - U[static_cast<size_t>(ix + 1)])) * iv * wk * s;
                    at(ix + 1, iy) += std::fabs((v - V[static_cast<size_t>(iy + 1)]) * (u - U[static_cast<size_t>(ix)])) * iv * wk * s;
                    at(ix, iy + 1) += std::fabs((v - V[static_cast<size_t>(iy)]) * (u - U[static_cast<size_t>(ix + 1)])) * iv * wk * s;
                    at(ix + 1, iy + 1) += std::fabs((v - V[static_cast<size_t>(iy)]) * (u - U[static_cast<size_t>(ix)])) * iv * wk * s;
                }
                put(g.data(), g.size() * 8);
                // second moments of the grid about its centre (AVAS.dll: offsets without the mean,
                // products before the weight, divided by multiplying with 1 / (s W))
                double su = 0, sv = 0, suv = 0;
                for (int x = 0; x < nd; ++x) {
                    const double du = (x - (nd * 0.5 - 0.5)) * hu;
                    for (int y = 0; y < nd; ++y) {
                        const double gv = g[static_cast<size_t>(x) * static_cast<size_t>(nd) + static_cast<size_t>(y)];
                        const double dv = (y - (nd * 0.5 - 0.5)) * hv;
                        su = du * du * gv + su;
                        sv = dv * dv * gv + sv;
                        suv = du * dv * gv + suv;
                    }
                }
                const double rn = 1.0 / (s * W);
                su = rn * su;
                sv = rn * sv;
                suv = rn * suv;
                const double e = std::sqrt(sv * su - suv * suv);
                const double v5[5] = {bb * e * gb, (-1.0 / e) * suv, su / e, std::sqrt(su), std::sqrt(sv)};
                put(v5, sizeof v5);
            }
        }
    }
    if (!write_file(outPath, out, false, true)) fail(outPath + " open failed, Check permissions", 1);
}

}  // namespace

int tool_dataprocess(const std::string& root, int i0, int i1, double g0, double g1, double mag, int useW) {
    try {
        say("The inFile Name has been set to " + root);
        const std::string pd = root + "/preprocessing_dataset";
        if (!file_exists(pd) && !make_dir(pd)) fail("Error output folder creation failure", 1);
        const int nF = static_cast<int>(std::round(g0)), nO = static_cast<int>(std::round(g1));
        const int step = static_cast<int>(std::ceil((i1 - i0 + 1) * 0.25));
        std::vector<int> b{i0};
        if (step > 0)
            while (b.back() + step < i1 + 1) b.push_back(b.back() + step);
        b.push_back(i1 + 1);
        say("NumofThreads: " + std::to_string(b.size() - 1));
        std::vector<std::thread> th;
        std::mutex mx;
        int err = 0;
        for (size_t s = 0; s + 1 < b.size(); ++s) {
            th.emplace_back([&, s] {
                for (int i = b[s]; i < b[s + 1]; ++i) {
                    try {
                        process_index(root, i, nF, nO, mag, useW);
                        std::lock_guard<std::mutex> g(mx);
                        say("Inedx " + std::to_string(i) + " complete.");
                    } catch (const ToolError& e) {
                        std::lock_guard<std::mutex> g(mx);
                        err = e.code;
                    }
                }
            });
        }
        for (auto& t : th) t.join();
        if (err) return err;
        say("PostProcess complete. index: [" + std::to_string(i0) + ", " + std::to_string(i1) + "]");
        return 0;
    } catch (const ToolError& e) {
        return e.code;
    }
}

}  // namespace avas
