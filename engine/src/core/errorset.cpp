#include "core/errorset.h"

#include <cmath>
#include <string>

#include "core/dllmath.h"
#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/text.h"

namespace avas {

namespace {

// GenerateGaussian of AVAS.dll (FUN_18001e1a0): Box-Muller with a cached second value, only sigma.
struct DllGaussian {
    bool generate = false;
    double z1 = 0.0;
    double next(MsvcRand& rng, double sigma) {
        generate = !generate;
        if (!generate) return z1 * sigma;
        double u1;
        int k2;
        do {
            u1 = static_cast<double>(rng.rand()) * 3.051850947599719e-05;
            k2 = rng.rand();
        } while (u1 <= 2.2250738585072014e-308);
        const double r = std::sqrt(std::log(u1) * -2.0);
        double s, c;
        dll_sincos(static_cast<double>(k2) * 0.00019175345033660654, s, c);
        z1 = r * s;
        return (r * c) * sigma;
    }
};

const char* kUnknownElem = "Unknown error distribution type, check the error distribution type";
const char* kUnknownBeam = "Specify an unknown error distribution and check the type of error distribution";

bool is_field_type(const Element& e, int t) {
    return e.typeName == "field" && e.words.size() > 4 && parse_int(e.words[4]) == t;
}

int element_row_size(const Element& e) {
    if (e.typeName == "field" && e.words.size() > 4 && parse_int(e.words[4]) == 4)
        return (static_cast<int>(e.words.size()) / 15 - 1) * 12;
    return 12;
}

}  // namespace

std::vector<ErrorSample> generate_error_set(const Lattice& lat, MsvcRand& rng, std::uint64_t seed) {
    const ErrorParameter& er = lat.err;
    const bool compat = compat_dll();
    const int a = er.a, b = er.b, ns = a * b;
    const bool f0 = er.flags[0], f1 = er.flags[1], f2 = er.flags[2], f3 = er.flags[3];
    if (ns < 1) throw EngineError("err_step needs positive numbers");
    // templates (spec 70 3.2)
    auto make_t = [&](int beamLen) {
        std::vector<std::vector<double>> t;
        t.emplace_back(beamLen, 0.0);
        for (int e = 0; e < er.nElements; ++e) t.emplace_back(element_row_size(lat.elements[e]), 0.0);
        return t;
    };
    const int bl = (f0 || f1) ? 26 : 1;
    std::vector<std::vector<double>> dynT, statT;
    if (f2 && f3) {
        dynT = statT = make_t(bl);
    } else if (f2) {
        if (f1) dynT = {std::vector<double>(26, 0.0)};
        statT = make_t((f0 || f1) ? 26 : 1);
    } else if (f3) {
        dynT = make_t((f0 || f1) ? 26 : 1);
        if (f0) statT = {std::vector<double>(26, 0.0)};
    } else if (f0 || f1) {
        dynT = statT = {std::vector<double>(26, 0.0)};
    }
    if (!compat) {
        // corrected: a static beam error without element errors gets its own beam row
        if (f0 && dynT.empty()) dynT = {std::vector<double>(26, 0.0)};
    }
    std::vector<ErrorSample> S(ns);
    for (auto& s : S) {
        s.stat = statT;
        s.dyn = dynT;
        for (int i = 0; i < 4; ++i) s.flags[i] = er.flags[i];
    }
    Philox4x32 ph{{static_cast<std::uint32_t>(seed), static_cast<std::uint32_t>(seed >> 32)}};
    DllGaussian gauss;
    double lastStat = 0.0, lastDyn = 0.0;  // AVAS.dll keeps the previous value for unknown r
    // corrected engine: r 0 fixed (static) / equal steps (dynamic), -1 equal steps, 1 uniform on
    // [-A, A), 2 Gaussian with sigma A; one counter per (sample, row, column, static/dynamic)
    auto def_value = [&](int k, int row, int c, int mode, double r, double x, int j) {
        const double A = ((j + 1) * x) / b;
        double u[2];
        ph.uniform2(static_cast<std::uint32_t>(k), 100u + static_cast<std::uint32_t>(row),
                    static_cast<std::uint32_t>(c * 2 + mode), u);
        if (r == 0.0) return mode == 0 ? x : A;
        if (r == -1.0) return A;
        if (r == 1.0) return (2.0 * u[0] - 1.0) * A;
        if (r == 2.0) {
            double g1, g2;
            p_gauss2(u[0], u[1], g1, g2);
            return g1 * A;
        }
        throw EngineError(kUnknownElem);
    };
    // AVAS.dll (/fp:fast) multiplies by 0.0001 instead of dividing by 10000
    auto uni = [&] { return static_cast<double>(rng.rand() % 10000) * 0.0001; };

    // ---- beam errors (row 0) ----
    const auto& sw = er.beamSwitches;
    const auto& v = er.beamVals;
    auto vget = [&](size_t i) { return i < v.size() ? v[i] : 0.0; };
    for (int mode = 0; mode < 2; ++mode) {
        if (!er.flags[mode]) continue;
        size_t blk = 0, vo = 0;
        if (compat) {
            blk = mode == 0 ? (sw.size() > 0 && sw[0] == 0 ? 0 : 30) : (sw.size() > 0 && sw[0] == 1 ? 0 : 30);
            vo = mode == 0 ? (vget(0) == 0.0 ? 0 : 30) : (vget(0) == 1.0 ? 0 : 30);
        }
        for (int is = 1; is < 30; ++is) {
            // corrected: the switch and value lines of this mode, wherever they were written
            const std::vector<int>& bs = er.beamSw[mode];
            const std::vector<double>& bv = er.beamLine[mode];
            if (compat ? (blk + is >= sw.size() || sw[blk + is] != 1)
                       : (static_cast<size_t>(is - 1) >= bs.size() || bs[is - 1] != 1))
                continue;
            const int c = is;
            const double r = compat ? vget(vo + 1) : (bv.empty() ? 0.0 : bv[0]);
            const double x = compat ? vget(vo + 1 + c) : (static_cast<size_t>(c) < bv.size() ? bv[c] : 0.0);
            for (int j = 0; j < b; ++j)
                for (int i = 0; i < a; ++i) {
                    const int k = j * a + i;
                    double val;
                    if (compat) {
                        const double A = ((j + 1) * x) / b;
                        // AVAS.dll reads r past the end of beamVals when both err_beam_stat and
                        // err_beam_dyn are given (offset 30, not padded): measured as an unknown type
                        const bool rPast = vo + 1 >= v.size();
                        if (rPast) {
                            say(mode == 0 ? kUnknownBeam : std::string("\xD6\xB8") + kUnknownBeam);
                            compat_exit(1);
                        }
                        if (r == 0.0) val = mode == 0 ? x : A;
                        else if (r == 1.0) val = uni() * A;
                        else if (r == 2.0) val = 0.0;
                        else {
                            say(mode == 0 ? kUnknownBeam : std::string("\xD6\xB8") + kUnknownBeam);
                            compat_exit(1);
                        }
                    } else {
                        val = def_value(k, 0, c, mode, r, x, j);
                    }
                    auto& rows = mode == 0 ? S[k].stat : S[k].dyn;
                    if (rows.empty() || static_cast<size_t>(c) >= rows[0].size()) continue;
                    rows[0][c] = val;
                    rows[0][0] = 1.0;
                }
        }
    }
    // ---- element errors ----
    for (int mode = 0; mode < 2; ++mode) {
        for (int cls = 0; cls < 2; ++cls) {
            const std::vector<int>* swr = nullptr;
            for (const auto& s : er.elemSwitches)
                if (s[0] == cls && s[1] == mode) swr = &s;  // the last switch line counts
            if (!swr) continue;
            for (size_t s = 2; s < swr->size(); ++s) {
                if ((*swr)[s] != 1) continue;
                const int c = static_cast<int>(s) - 2;
                for (const auto& d : er.elemDefs) {
                    if (d[0] == 2.0 || d[0] != cls || d[1] != mode) continue;
                    const int E = static_cast<int>(d[2]);
                    const int N = static_cast<int>(d[3]);
                    const double r = d[4];
                    const double x = 5 + c < static_cast<int>(d.size()) ? d[5 + c] : 0.0;
                    for (int e = E; e < std::min(E + N, er.nElements); ++e) {
                        const Element& el = lat.elements[e];
                        bool target;
                        if (cls == 0 && mode == 0) target = is_field_type(el, 3);
                        else if (cls == 1) target = is_field_type(el, 1);
                        else target = is_field_type(el, 3) || el.typeName == "quad" || el.typeName == "solenoid" ||
                                      el.typeName == "bend" || el.typeName == "edge";
                        if (!target) continue;
                        for (int j = 0; j < b; ++j)
                            for (int i = 0; i < a; ++i) {
                                const int k = j * a + i;
                                double val;
                                if (compat) {
                                    const double A = ((j + 1) * x) / b;
                                    if (mode == 0) {
                                        if (r == 0.0) val = x;
                                        else if (r == 1.0) val = uni() * A;
                                        else if (r == 2.0 || r == 3.0) { say(kUnknownElem); compat_exit(1); }
                                        else val = lastStat;
                                        lastStat = val;
                                    } else {
                                        if (r == 0.0) val = A;
                                        else if (r == 1.0) { const double dd = (uni() - 0.5) * A; val = dd + dd; }
                                        else if (r == 2.0) val = gauss.next(rng, A);
                                        else if (r == 3.0) { say(kUnknownElem); compat_exit(1); }
                                        else val = lastDyn;
                                        lastDyn = val;
                                    }
                                } else {
                                    val = def_value(k, e + 1, c, mode, r, x, j);
                                }
                                auto& rows = mode == 0 ? S[k].stat : S[k].dyn;
                                if (rows.size() <= static_cast<size_t>(e + 1) || rows[e + 1].size() <= static_cast<size_t>(c + 1)) continue;
                                rows[e + 1][c + 1] = val;
                                rows[e + 1][0] = 1.0;
                            }
                    }
                }
                // superpose records (spec 70 3.6): element rec[2], sub-field rec[19]; AVAS.dll draws
                // only the dynamic ones, the corrected engine also the static ones
                if (mode == 1 || !compat) {
                    for (const auto& d : er.elemDefs) {
                        if (d[0] != 2.0) continue;
                        for (size_t o = 0; o + 20 <= d.size(); o += 20) {
                            const double* rec = &d[o];
                            if (rec[1] != static_cast<double>(mode) || static_cast<int>(rec[18]) != cls) continue;
                            const int e = static_cast<int>(rec[2]);
                            const int sub = static_cast<int>(rec[19]);
                            const double r = rec[4];
                            const double x = 5 + c < 18 ? rec[5 + c] : 0.0;
                            for (int j = 0; j < b; ++j)
                                for (int i = 0; i < a; ++i) {
                                    const int k = j * a + i;
                                    double val;
                                    if (compat) {
                                        const double A = ((j + 1) * x) / b;
                                        if (r == 0.0) val = A;
                                        else if (r == 1.0) { const double dd = (uni() - 0.5) * A; val = dd + dd; }
                                        else if (r == 2.0) val = gauss.next(rng, A);
                                        else if (r == 3.0) { say(kUnknownElem); compat_exit(1); }
                                        else val = lastDyn;
                                        lastDyn = val;
                                    } else {
                                        val = def_value(k, 1000 + e * 64 + sub, c, mode, r, x, j);
                                    }
                                    auto& rows = mode == 0 ? S[k].stat : S[k].dyn;
                                    const size_t col = static_cast<size_t>(sub) * 12 + static_cast<size_t>(c) + 1;
                                    if (rows.size() <= static_cast<size_t>(e + 1) || rows[e + 1].size() <= col) {
                                        if (!compat) throw EngineError("An error line inside a superpose block has no sub-field to act on");
                                        continue;
                                    }
                                    rows[e + 1][col] = val;
                                    rows[e + 1][static_cast<size_t>(sub) * 12] = 1.0;
                                    rows[e + 1][0] = 1.0;  // AVAS.dll marks the element row as well
                                }
                        }
                    }
                }
            }
        }
    }
    if (compat) {
        // main_agent copies dyn only when f1 or f3 and stat only when f0 or f2 (spec 70 3.2)
        for (auto& s : S) {
            if (!(f1 || f3)) s.dyn.clear();
            if (!(f0 || f2)) s.stat.clear();
        }
    }
    return S;
}

void set_errors_in_elements(Lattice& lat, const ErrorSample& es) {
    const double deg = 0.017453292519943295;  // AVAS.dll folds pi / 180
    const bool f2 = es.flags[2], f3 = es.flags[3];
    for (size_t i = 0; i < lat.elements.size(); ++i) {
        Element& e = lat.elements[i];
        const std::vector<double>* D = (es.dyn.size() > i + 1) ? &es.dyn[i + 1] : nullptr;
        const std::vector<double>* S = (es.stat.size() > i + 1) ? &es.stat[i + 1] : nullptr;
        auto d = [&](int c) { return (D && static_cast<size_t>(c) < D->size()) ? (*D)[c] : 0.0; };
        auto st = [&](int c) { return (S && static_cast<size_t>(c) < S->size()) ? (*S)[c] : 0.0; };
        if (e.kind == ElemKind::Quad || e.kind == ElemKind::Solenoid || e.kind == ElemKind::Bend || e.kind == ElemKind::Edge) {
            if (!f3) continue;
            e.errDx = d(1) * 0.001 + e.errDx;
            e.errDy = d(2) * 0.001 + e.errDy;
            e.errDz = d(7) * 0.001 + e.errDz;
            e.errRotX = d(3) * deg + e.errRotX;
            e.errRotY = d(4) * deg + e.errRotY;
            e.errRotZ = d(5) * deg + e.errRotZ;
            e.strength = (d(6) * 0.01 + 1.0) * e.strength;
            e.fieldErr = e.strength;
        } else if (e.kind == ElemKind::Field && e.fieldType == 4) {
            // spec 70 4.2: per sub-field, from its 12 slots of the row (only dynamic ones are drawn)
            const size_t ns = e.subType.size();
            e.subErr.assign(ns, std::array<double, 12>{});
            for (size_t q = 0; q < ns; ++q) {
                auto& se = e.subErr[q];
                const int t = e.subType[q];
                if (t == 1) se[7] = e.subKb[q];
                else if (t == 2) { se[7] = e.subKb[q]; se[8] = e.subKe[q]; }
                else if (t == 3) se[8] = e.subKe[q];
                auto slot = [&](const std::vector<double>* R, int c) {
                    const size_t idx = q * 12 + static_cast<size_t>(c);
                    return (R && idx < R->size()) ? (*R)[idx] : 0.0;
                };
                if (compat_dll()) {
                    const std::vector<double>* R = f3 ? D : S;
                    if (!R || slot(R, 0) != 1.0) continue;
                    se[0] = 1.0;
                    se[1] += slot(R, 1) * 0.001;
                    se[2] += slot(R, 2) * 0.001;
                    se[3] += slot(R, 7) * 0.001;
                    se[4] += slot(R, 3) * deg;
                    se[5] += slot(R, 4) * deg;
                    if (t == 1 || t == 3) {
                        se[6] += slot(R, 5) * deg;
                        const double f = slot(R, 6) * 0.01 + 1.0;
                        if (t == 1) se[7] = f * e.subKb[q];
                        else se[8] = f * e.subKe[q];
                    } else if (t == 2) {
                        double p5 = slot(R, 5);
                        if (f2 && f3) p5 = p5 + slot(S, 5);
                        se[7] = (p5 * 0.01 + 1.0) * e.subKb[q];
                        se[8] = (p5 * 0.01 + 1.0) * e.subKe[q];
                        se[9] += slot(R, 6);
                    }
                    continue;
                }
                // corrected: static and dynamic slots add up
                const bool hs = S && slot(S, 0) == 1.0, hd = D && slot(D, 0) == 1.0;
                if (!hs && !hd) continue;
                auto sum = [&](int c) { return (hs ? slot(S, c) : 0.0) + (hd ? slot(D, c) : 0.0); };
                se[0] = 1.0;
                se[1] = sum(1) * 0.001;
                se[2] = sum(2) * 0.001;
                se[3] = sum(7) * 0.001;
                se[4] = sum(3) * deg;
                se[5] = sum(4) * deg;
                if (t == 1 || t == 3) {
                    se[6] = sum(5) * deg;
                    const double f = sum(6) * 0.01 + 1.0;
                    if (t == 1) se[7] = f * e.subKb[q];
                    else se[8] = f * e.subKe[q];
                } else if (t == 2) {
                    const double f = sum(5) * 0.01 + 1.0;
                    se[7] = f * e.subKb[q];
                    se[8] = f * e.subKe[q];
                    se[9] = sum(6);
                }
            }
        } else if (e.kind == ElemKind::Field && (e.fieldType == 1 || e.fieldType == 2 || e.fieldType == 3)) {
            e.errDx = e.errDy = e.errDz = e.errRotX = e.errRotY = e.errRotZ = 0.0;
            e.keErr = e.kbErr = e.phaseErr = 0.0;
            const int ampCol = e.fieldType == 1 ? 5 : 6;
            if (f2) {
                e.errDx = st(1) * 0.001; e.errDy = st(2) * 0.001; e.errDz = st(7) * 0.001;
                e.errRotX = st(3) * deg; e.errRotY = st(4) * deg;
                if (e.fieldType != 1) e.errRotZ = st(5) * deg;
                if (e.fieldType == 1) e.phaseErr = st(6);
            }
            if (f3) {
                e.errDx += d(1) * 0.001; e.errDy += d(2) * 0.001; e.errDz += d(7) * 0.001;
                e.errRotX += d(3) * deg; e.errRotY += d(4) * deg;
                if (e.fieldType != 1) e.errRotZ += d(5) * deg;
                if (e.fieldType == 1) e.phaseErr += d(6);
            }
            double factor;
            if (f2 && f3) factor = (st(ampCol) + d(ampCol)) * 0.01 + 1.0;
            else if (f3) factor = d(ampCol) * 0.01 + 1.0;
            else factor = st(ampCol) * 0.01 + 1.0;
            if (e.fieldType == 1) { e.keErr = factor * e.ke; e.kbErr = factor * e.kb; }
            else if (e.fieldType == 2) e.keErr = factor * e.ke;
            else e.kbErr = factor * e.kb;
        }
    }
}

void write_error_sample(const ErrorSample& es) {
    auto dump = [](const std::vector<std::vector<double>>& rows, const char* name) {
        Text t;
        for (const auto& r : rows) {
            for (double v : r) t.g(v).s(" ");
            t.nl();
        }
        write_file(paths().output + name, t.str(), false);
    };
    if (es.flags[0] || es.flags[2]) dump(es.stat, "StaticErrorData.txt");
    if (es.flags[1] || es.flags[3]) dump(es.dyn, "DynamicErrorData.txt");
}

}  // namespace avas
