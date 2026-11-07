#include "core/lattice.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

#include "core/dllmath.h"
#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/physics.h"
#include "core/text.h"

namespace avas {

namespace {

// Keywords the original kernel knows but the new engine does not implement yet.
// err_* keywords read by the kernel (spec 70 2.2); the coupled err_*_cpl_* lines are ignored.
bool is_error_keyword(const std::string& k) {
    static const char* const names[] = {
        "err_beam_stat", "err_beam_dyn", "err_cav_ncpl_stat", "err_cav_ncpl_dyn", "err_quad_ncpl_stat",
        "err_quad_ncpl_dyn", "err_step", "err_beam_stat_on", "err_beam_dyn_on", "err_quad_stat_on",
        "err_quad_dyn_on", "err_cav_stat_on", "err_cav_dyn_on"};
    for (const char* n : names)
        if (k == n) return true;
    return false;
}

// Keywords both original kernels ignore silently (GUI / linear model only).
bool is_ignored(const std::string& k) {
    static const char* const names[] = {"start", "superposeend", "lattice", "lattice_end", "adjust",
                                        "diag_energy", "diag_size", "diag_position", "err_quad_cpl_stat",
                                        "err_quad_cpl_dyn", "err_cav_cpl_stat", "err_cav_cpl_dyn"};
    for (const char* n : names)
        if (k == n) return true;
    return false;
}

void print_not_enough(const std::vector<std::string>& w) {
    std::string s = "Error! parameter not enough in element : \n";
    for (const auto& x : w) s += x + "\t";
    say(s);
}

struct AutoRow {
    double first, spacing, span;
};

// Number formatting of the synthesised superpose rows: "%f" in the original (rounds lengths and
// positions to 1e-6 m), round-trip "%.17g" in the corrected engine.
std::string num(double v) {
    char buf[64];
    std::snprintf(buf, sizeof buf, compat_dll() ? "%f" : "%.17g", v);
    return buf;
}

class LatticeReader {
public:
    LatticeReader(Lattice& lat, Params& p) : lat_(lat), p_(p) {}

    void run() {
        std::string raw;
        if (!read_file(paths().input + "lattice.txt", raw)) {
            throw EngineError("File \"lattice.txt\" cannot be opened, Check the inputFile routing and lattice.txt.");
        }
        say("Initialising lattice...");
        lines_ = split_lines(normalize_line_endings(raw));
        bool skip = true;
        bool ended = false;
        std::vector<std::string> w;
        while (next_line(w)) {
            if (skip && w[0] != "start") continue;
            skip = false;
            if (w[0] == "end") {
                lat_.err.nElements = static_cast<int>(lat_.elements.size());
                if (lat_.err.beamVals.empty()) lat_.err.flags[0] = lat_.err.flags[1] = 0;
                finish();
                ended = true;
                break;
            }
            dispatch(w);
        }
        if (!ended) {
            // The original stops without finishing the lattice (undefined behaviour later on).
            say("Warning: lattice.txt has no end line; the lattice is closed at its last element");
            finish();
        }
    }

private:
    Lattice& lat_;
    Params& p_;
    int cav_ = 0;
    std::vector<AutoRow> auto_;
    std::vector<std::string> lines_;
    size_t pos_ = 0;

    bool next_line(std::vector<std::string>& w) {
        while (pos_ < lines_.size()) {
            w = split_words(lines_[pos_++]);
            if (!w.empty()) return true;
        }
        return false;
    }
    std::vector<std::string> block_line() {
        std::vector<std::string> w;
        // the original loops forever when a superpose block reaches the end of the file
        if (!next_line(w)) throw EngineError("A superpose block is not closed before the end of lattice.txt");
        return w;
    }

    double current_end() const { return lat_.elemZEnd.empty() ? 0.0 : lat_.elemZEnd.back(); }

    void dispatch(std::vector<std::string>& w) {
        const std::string& k = w[0];
        if (k == "drift") {
            add_drift(w);
        } else if (k == "automaticoutput") {
            if (w.size() < 4) throw EngineError("keyword: automaticoutput, Insufficient parameters");
            const double first = (lat_.elemZEnd.empty() ? parse_double(w[1]) : current_end() + parse_double(w[1]));
            auto_.push_back({first, parse_double(w[2]), parse_double(w[3])});
        } else if (k == "outputplane") {
            if (w.size() < 2) throw EngineError("keyword: outputplane, Insufficient parameters");
            p_.outputPlanes.push_back(lat_.elemZEnd.empty() ? parse_double(w[1]) : current_end() + parse_double(w[1]));
        } else if (k == "superpose") {
            superpose_block(w);
        } else if (k == "multipole") {
            multipole_to_field(w);
            add_field(w);
        } else if (k == "quad") {
            add_quad(w);
        } else if (k == "solenoid") {
            add_matrix(w, ElemKind::Solenoid, 5);
        } else if (k == "steerer") {
            add_matrix(w, ElemKind::Steerer, 7);
        } else if (k == "bend") {
            add_matrix(w, ElemKind::Bend, 5);
        } else if (k == "edge") {
            add_matrix(w, ElemKind::Edge, 5);
        } else if (k == "rfq") {
            add_rfq();
        } else if (k == "field") {
            add_field(w);
        } else if (k == "spacechargecomp") {
            // AVAS.dll only: compensation factor V from here on (space charge x (1 - V) for V < 1).
            if (w.size() < 2) throw EngineError("keyword: spacechargecomp, Insufficient parameters");
            double v = parse_double(w[1]);
            if (!compat_dll() && (v < 0.0 || v > 1.0)) {
                // corrected engine: the space charge factor 1 - V is limited to [0, 1] (AVAS.dll
                // ignores V >= 1 silently and amplifies the space charge for V < 0)
                say(Text().s("lattice.txt: spacechargecomp ").g(v).s(" outside [0, 1], limited to ")
                        .g(v < 0.0 ? 0.0 : 1.0).str());
                v = v < 0.0 ? 0.0 : 1.0;
            }
            lat_.spaceChargeComp.push_back({current_end(), 1.0, v});
        } else if (is_error_keyword(k)) {
            parse_error_line(w);
        } else if (!is_ignored(k) && !compat_dll()) {
            say("lattice.txt: unknown keyword \"" + k + "\" ignored");
        }
    }

    // err_* keywords (spec 70 2.3); E = number of elements built so far
    void parse_error_line(const std::vector<std::string>& w) {
        ErrorParameter& er = lat_.err;
        const std::string& k = w[0];
        const double E = static_cast<double>(lat_.elements.size());
        // AVAS.dll: up to 30 entries -> padded to 30, otherwise to 60 (spec 70 3.3)
        auto pad_switches = [&] {
            const size_t n = er.beamSwitches.size();
            if (n < 31) er.beamSwitches.resize(30, 0);
            else if (n < 60) er.beamSwitches.resize(60, 0);
        };
        if (k == "err_step") {
            er.a = parse_int(w.at(1));
            er.b = parse_int(w.at(2));
        } else if (k == "err_beam_stat" || k == "err_beam_dyn") {
            er.beamVals.push_back(k == "err_beam_dyn" ? 1.0 : 0.0);
            auto& line = er.beamLine[k == "err_beam_dyn" ? 1 : 0];
            line.clear();
            for (size_t i = 1; i < w.size(); ++i) {
                er.beamVals.push_back(parse_double(w[i]));
                line.push_back(parse_double(w[i]));
            }
            pad_switches();
        } else if (k == "err_beam_stat_on" || k == "err_beam_dyn_on") {
            const bool dyn = k == "err_beam_dyn_on";
            er.beamSwitches.push_back(dyn ? 1 : 0);
            bool on = false;
            er.beamSw[dyn ? 1 : 0].clear();
            for (size_t i = 1; i < w.size(); ++i) {
                const int v = parse_int(w[i]);
                if (v != 0) on = true;
                er.beamSwitches.push_back(v);
                er.beamSw[dyn ? 1 : 0].push_back(v);
            }
            pad_switches();
            if (on) er.flags[dyn ? 1 : 0] = 1;
        } else if (k.find("_ncpl_") != std::string::npos) {
            const double cls = k.rfind("err_cav", 0) == 0 ? 1.0 : 0.0;
            const double mode = k.compare(k.size() - 4, 4, "_dyn") == 0 ? 1.0 : 0.0;
            std::vector<double> d{cls, mode, E};
            for (size_t i = 1; i < w.size(); ++i) d.push_back(parse_double(w[i]));
            if (d.size() < 20) d.resize(20, 0.0);
            er.elemDefs.push_back(d);
        } else {  // err_quad/cav_stat/dyn_on
            const int cls = k.rfind("err_cav", 0) == 0 ? 1 : 0;
            const int mode = k.find("_dyn_on") != std::string::npos ? 1 : 0;
            std::vector<int> sw{cls, mode};
            bool on = false;
            for (size_t i = 1; i < w.size(); ++i) {
                const int v = parse_int(w[i]);
                if (v != 0) on = true;
                sw.push_back(v);
            }
            if (sw.size() < 20) sw.resize(20, 0);
            er.elemSwitches.push_back(sw);
            if (on) er.flags[mode ? 3 : 2] = 1;
        }
    }

    // Lattice::SetElement: accumulate z and append.
    Element& push_element(const std::vector<std::string>& w) {
        const double zStart = current_end();
        const double L = parse_double(w.at(1));
        lat_.elemZStart.push_back(zStart);
        lat_.elemLength.push_back(L);
        lat_.elemZEnd.push_back(zStart + L);
        lat_.elements.emplace_back();
        Element& e = lat_.elements.back();
        e.index = static_cast<int>(lat_.elements.size()) - 1;
        return e;
    }

    void set_basic(Element& e, const std::vector<std::string>& w) {
        e.words = w;
        e.zStart = lat_.elemZStart.back();
        e.typeName = w[0];
        e.length = parse_double(w[1]);
        e.radius = parse_double(w.at(2));
        e.zEnd = e.zStart + e.length;
    }

    // "Error! parameter not enough" that only prints in the original: the element keeps its
    // default values (typeName "drift", L = R = 0).  The corrected engine stops instead.
    bool too_few(const std::vector<std::string>& w, size_t need) {
        if (w.size() >= need) return false;
        if (!compat_dll()) throw EngineError("Error! parameter not enough in element : " + w[0]);
        print_not_enough(w);
        return true;
    }

    void add_drift(const std::vector<std::string>& w) {
        if (w.size() < 2) throw EngineError("Error! parameter not enough in element : drift");
        Element& e = push_element(w);
        e.kind = ElemKind::Drift;
        if (too_few(w, 4)) return;
        set_basic(e, w);
    }

    void add_quad(const std::vector<std::string>& w) {
        if (w.size() < 4) throw EngineError("Error! parameter not enough in element : quad");
        if (w.size() < 5) throw std::out_of_range("vector::_M_range_check");
        Element& e = push_element(w);
        e.kind = ElemKind::Quad;
        set_basic(e, w);
        e.strength = parse_double(w[4]);
        if (e.strength == 0.0) {
            throw EngineError("The gradient of the quadrupole cannot be set to 0, Drift can be used to replace "
                              "quadrupole with a field gradient of 0");
        }
    }

    void add_matrix(const std::vector<std::string>& w, ElemKind kind, size_t need) {
        if (w.size() < 2) throw EngineError("Error! parameter not enough in element : " + w[0]);
        Element& e = push_element(w);
        e.kind = kind;
        if (too_few(w, need)) return;
        set_basic(e, w);
        switch (kind) {
            case ElemKind::Solenoid:
                e.strength = parse_double(w[4]);
                break;
            case ElemKind::Steerer:
                e.steerKind = static_cast<int>(parse_double(w[6]));
                e.steerX = parse_double(w[4]);
                e.steerY = parse_double(w[5]);
                e.steerMax = parse_double(w.at(7));
                break;
            case ElemKind::Bend:
                if (w.size() < 8) throw std::out_of_range("vector::_M_range_check");
                e.hv = static_cast<int>(parse_double(w[7]));
                e.alpha = parse_double(w[4]) * k::deg_to_rad;
                e.rho = parse_double(w[5]);
                e.fieldIndex = parse_double(w[6]);
                e.arc = std::fabs(e.alpha) * e.rho;
                e.h = e.alpha / e.arc;
                e.kx = std::sqrt((1.0 - e.fieldIndex) * e.h * e.h);
                e.ky = std::sqrt(e.h * e.fieldIndex * e.h);
                break;
            case ElemKind::Edge: {
                if (w.size() < 10) throw std::out_of_range("vector::_M_range_check");
                e.poleAngle = parse_double(w[4]) * k::deg_to_rad;
                e.rho = std::fabs(parse_double(w[5]));
                e.gap = parse_double(w[6]);
                e.fringeK1 = parse_double(w[7]);
                e.fringeK2 = parse_double(w[8]);
                e.hv = parse_int(w[9]);
                // TRANSPORT fringe-field correction angle, evaluated as AVAS.dll does
                double s, c;
                dll_sincos(e.poleAngle, s, c);
                const double ir = 1.0 / e.rho;
                const double t = std::tan(e.poleAngle);
                e.psi = ((s * s + 1.0) / c * e.gap) *
                        ((1.0 - t * e.fringeK2 * (ir * e.gap * e.fringeK1)) * ir * e.fringeK1);
                break;
            }
            default:
                break;
        }
    }

    // ReadRFQ: converts rfqinputAnn.txt / rfqinputV.txt into rfqinput.txt and E0-z.txt (in the
    // input folder, as the original).  The RFQ element itself is not available: AVAS.dll crashes
    // on it and libAVAS.so never reads its cell file (see M2-notes).
    void add_rfq() {
        read_rfq();
        throw EngineError("Lattice keyword \"rfq\" is not implemented in the new engine yet (the original "
                          "kernels cannot run it either)");
    }

    void read_rfq() {
        auto load = [](const std::string& name, size_t ncol, const char* err, std::vector<std::vector<float>>& rows) {
            std::string raw;
            if (!read_file(paths().input + name, raw)) {
                say(err);
                return;
            }
            for (const std::string& line : split_lines(raw)) {
                std::string l = line;
                if (!l.empty() && l.back() == '\r') l.pop_back();
                const std::vector<std::string> wd = split_words(l);
                if (wd.size() < ncol) throw std::out_of_range("vector::_M_range_check");
                std::vector<float> r(ncol);
                for (size_t i = 0; i < ncol; ++i) r[i] = static_cast<float>(parse_double_lenient(wd[i]));
                rows.push_back(r);
            }
        };
        std::vector<std::vector<float>> ann, vv;
        load("rfqinputAnn.txt", 13, "RFQ Structure path Ann ERROR!", ann);
        load("rfqinputV.txt", 14, "RFQ Structure path V ERROR!", vv);
        Text out, e0;
        for (size_t i = 0; i < vv.size(); ++i) {
            const auto& v = vv[i];
            if (i >= ann.size()) throw EngineError("rfqinputAnn.txt has fewer rows than rfqinputV.txt");
            const auto& a = ann[i];
            out.g(v[0]).s(" ").g(v[12]).s(" ").g(v[1]).s(" ").g(v[7]).s(" ").g(v[8]).s(" ").g(v[6]);
            for (int c : {6, 5, 8, 7, 10, 12, 9, 11}) out.s(" ").fixed(a[c], 6);
            out.nl();
            const float e = v[13] / 100.0f;
            e0.g(e).s(" ").g(v[4]).nl();
        }
        if (!write_file(paths().input + "rfqinput.txt", out.str(), false)) say("output error");
        write_file(paths().input + "E0-z.txt", e0.str(), false);
    }

    // Initial::MultipoleFieldMapGenerator + Converter: writes <name>.bsx/.bsy/.bsz into the field
    // folder and turns the row into a static magnetic field row.
    void multipole_to_field(std::vector<std::string>& w) {
        if (w.size() < 10) throw EngineError("Error! parameter not enough in element : multipole");
        const int poles = parse_int(w[4]);
        const double L = parse_double(w[1]);
        const int n = parse_int(w[5]);
        const double G = parse_double(w[6]);
        const double R = parse_double(w[2]);
        double kk;
        if (poles == 3) kk = (G + G) / (R * R);
        else if (poles == 4) kk = (G / (R * R * R)) * 6.0;
        else if (poles == 5) kk = (G / (R * R * R * R)) * 24.0;
        else throw EngineError("Abnormal setting of the number of magnetic poles");
        const double d = (R + R) / n;
        Text hdr;
        hdr.s("  ").i(2).s("  ").g(L).nl();
        for (int k = 0; k < 2; ++k) hdr.s("  ").i(n).s("  ").g(-R).s("  ").g(R).nl();
        hdr.s("  ").i(1).nl();
        Text fx = hdr, fy = hdr, fz = hdr;
        for (int s = 0; s < 3; ++s) {
            for (int j = 0; j <= n; ++j) {
                for (int i = 0; i <= n; ++i) {
                    const double x = i * d - R, y = j * d - R;
                    double bx, by;
                    if (poles == 3) {
                        bx = y * x * (kk + kk);
                        by = (x * x - y * y) * kk;
                    } else if (poles == 4) {
                        bx = (x * 3.0 * x * y - y * y * y) * kk;
                        by = (x * x * x - x * 3.0 * y * y) * kk;
                    } else {
                        const double x3 = x * x * x;
                        bx = (x3 * y - x * y * y * y) * kk * 4.0;
                        by = ((x3 * x - y * y * x * x * 6.0) + y * y * y * y) * kk;
                    }
                    fx.s("  ").g(bx).nl();
                    fy.s("  ").g(by).nl();
                    fz.s("  ").g(0.0).nl();
                }
            }
        }
        const std::string base = paths().field + w[9];
        write_file(base + ".bsx", fx.str(), false);
        write_file(base + ".bsy", fy.str(), false);
        write_file(base + ".bsz", fz.str(), false);
        // AVAS.dll multiplies the map, already scaled by G, by Kb = G again (field ~ G^2).
        w[0] = "field";
        w[8] = compat_dll() ? w[6] : std::string("1");
        w[3] = "0";
        w[4] = "3";
        w[5] = "0";
        w[6] = "0";
        w[7] = "1";
    }

    void add_field(std::vector<std::string> w) {
        if (w.size() < 10) {
            if (!compat_dll()) throw EngineError("Error! parameter not enough in element : field");
            if (w.size() < 6 || parse_double(w[5]) == 0.0) {  // undefined behaviour, then exit(1)
                print_not_enough(w);
                compat_exit(1);
            }
            // AVAS.dll: the path prefix is not applied, word 9 becomes "invalid" below.
        } else {
            w[9] = paths().field + w[9];
        }
        const double f = parse_double(w[5]);
        if (f != 0.0) {
            while (w.size() < 14) w.push_back("invalid");
            if (parse_int(w[3]) == 0) p_.needSynPhase = 1;
            w.push_back(std::to_string(cav_));
            cav_ += 1;
        }
        if (parse_int(w[4]) == 4) throw EngineError("Field type 4 is only produced by superpose blocks");
        add_field_row(w);
    }

    // Lattice::SetElement + Field::SetParameter for a (possibly synthesised) field row.
    void add_field_row(std::vector<std::string> w) {
        const int type = parse_int(w[4]);
        Element& e = push_element(w);
        int idx = -1;
        if (type == 4) {
            const size_t m = w.size() / 20;
            for (size_t off = 20; off + 20 <= 20 * m; off += 20) {
                if (w[off] == "field") {
                    const int si = lat_.fm.load(w[off + 9], parse_int(w[off + 4]));
                    w[off + 9] = std::to_string(si);
                }
            }
        } else {
            idx = lat_.fm.load(w[9], type);
            w[9] = std::to_string(idx);
        }
        e.kind = ElemKind::Field;
        set_basic(e, w);
        e.fieldType = type;
        e.frequency = parse_double(w[5]);
        e.phaseRad = parse_double(w[6]) / 180.0 * k::pi;
        e.ke = parse_double(w[7]);
        e.kb = parse_double(w[8]);
        e.mapIndex = type == 4 ? parse_int(w[9]) : idx;
        if (type == 1) {
            if (e.frequency == 0.0) {
                throw EngineError("RF cavity frequency cannot be set to 0, Check the parameters of the RF cavity elements");
            }
            e.cavIndex = parse_int(w.at(14));
        } else if (type == 4) {
            if (e.frequency != 0.0) e.cavIndex = parse_int(w.at(14));
            parse_superpose_subs(e, w);
        } else if (type != 0 && type != 2 && type != 3) {
            say("Field type error!");
        }
    }

    // Field::SetParameter, type 4: sub-blocks of 20 words after the 20-word head.
    void parse_superpose_subs(Element& e, const std::vector<std::string>& w) {
        const size_t m = w.size() / 20;
        if (m < 2) return;
        const double head12 = parse_double(w[12]);
        for (size_t j = 0; j + 2 <= m; ++j) {
            const size_t b = 20 + 20 * j;
            auto common = [&](int st, double ke, double kb, int map, bool place) {
                e.subType.push_back(st);
                e.subKe.push_back(ke);
                e.subKb.push_back(kb);
                e.subMap.push_back(map);
                e.subZStart.push_back(e.zStart + (parse_double(w[b + 12]) - head12));
                e.subRadius.push_back(parse_double(w[b + 2]));
                e.subLength.push_back(parse_double(w[b + 1]));
                std::array<double, 6> pl{};
                if (place) {
                    pl = {parse_double(w[b + 14]), parse_double(w[b + 15]), parse_double(w[b + 16]),
                          parse_double(w[b + 17]) * k::pi / 180.0, parse_double(w[b + 18]) * k::pi / 180.0,
                          parse_double(w[b + 19]) * k::pi / 180.0};
                }
                e.subPlace.push_back(pl);
            };
            if (w[b] == "field") {
                const int t = parse_int(w[b + 4]);
                if (t == 1) {
                    e.frequency = parse_double(w[b + 5]);
                    e.phaseRad = parse_double(w[b + 6]) / 180.0 * k::pi;
                    common(2, parse_double(w[b + 7]), parse_double(w[b + 8]), parse_int(w[b + 9]), true);
                } else if (t == 3) {
                    common(1, 0.0, parse_double(w[b + 8]), parse_int(w[b + 9]), true);
                } else if (t == 2) {
                    common(3, parse_double(w[b + 7]), 0.0, parse_int(w[b + 9]), true);
                } else {
                    say("Superposed field contains a field that cannot be superposed");
                }
            } else {
                // AVAS.dll tests words[15 + 15 j] (wrong stride), so a drift is practically never
                // recognised; the corrected engine registers it (aperture only).
                const size_t at = compat_dll() ? 15 + 15 * j : b;
                if (at < w.size() && w[at] == "drift") common(0, 0.0, 0.0, 0, !compat_dll());
            }
        }
    }

    // A superpose ... superposeend / superposeout block becomes one or more type-4 field rows.
    void superpose_block(std::vector<std::string> w) {
        struct Sub {
            std::vector<std::string> words;
            double z0, zEnd;
            std::array<double, 6> sp;
        };
        std::vector<Sub> subs;
        int mode = 0;
        std::array<double, 7> out{};
        double rmin = 1.0;
        std::vector<std::string> terminator;
        while (true) {
            if (w[0] != "superpose") {
                terminator = w;  // dropped by the original
                break;
            }
            Sub s;
            s.z0 = parse_double_lenient(w.at(1));
            if (w.size() < 3) {
                s.sp = {parse_double(w[1]), 0, 0, 0, 0, 0};
            } else if (w.size() < 7) {
                throw EngineError("Abnormal number of parameters for keyword superpose");
            } else {
                for (int i = 0; i < 6; ++i) s.sp[i] = parse_double_lenient(w[1 + i]);
            }
            w = block_line();
            if (w[0] == "err_cav_ncpl_stat" || w[0] == "err_cav_ncpl_dyn" || w[0] == "err_quad_ncpl_stat" ||
                w[0] == "err_quad_ncpl_dyn") {
                throw EngineError("Error lines inside a superpose block are not implemented in the new engine yet");
            }
            if (w[0] == "multipole") multipole_to_field(w);
            if (w.size() < 3) throw EngineError("Error! parameter not enough in element : " + w[0]);
            const double r = parse_double_lenient(w[2]);
            if (r < rmin) rmin = parse_double(w[2]);
            s.zEnd = s.z0 + parse_double_lenient(w[1]);
            s.words = w;
            subs.push_back(s);
            w = block_line();
            if (w[0] == "superposeout") {
                if (w.size() < 7) throw EngineError("Abnormal number of parameters for keyword superposeout");
                out = {parse_double(w[1]), parse_double(w[2]), parse_double(w[3]), parse_double(w[4]) * k::pi / 180.0,
                       parse_double(w[5]) * k::pi / 180.0, parse_double(w[6]) * k::pi / 180.0, 0.0};
                // AVAS.dll InitLattice: 1 = turned about x only (beam frame turns in y-z), else 2
                mode = (parse_double(w[6]) == 0.0 && parse_double(w[5]) != 0.0) ? 1 : 2;
                break;
            }
        }
        int nRf = 0, kRf = -1;
        for (size_t i = 0; i < subs.size(); ++i) {
            auto& sw = subs[i].words;
            if (sw[0] != "field") continue;
            if (sw.size() < 10) throw EngineError("Error! parameter not enough in element : field");
            sw[9] = paths().field + sw[9];
            if (parse_double(sw[5]) != 0.0) {
                ++nRf;
                kRf = static_cast<int>(i);
                if (parse_int(sw[3]) == 0) p_.needSynPhase = 1;
            }
        }
        if (nRf > 0 && mode != 0) throw EngineError("RF cavity is temporarily not allowed in curve transmission");
        if (nRf > 1) throw EngineError("Only one RF cavity is allowed to be used in a set of superimposed fields");

        double minZ0 = subs[0].z0, maxZEnd = subs[0].zEnd;
        for (const Sub& s : subs) {
            minZ0 = std::min(minZ0, s.z0);
            maxZEnd = std::max(maxZEnd, s.zEnd);
        }
        auto subBlock = [&](const Sub& s, std::vector<std::string>& row) {
            std::vector<std::string> sw = s.words;
            while (sw.size() < 12) sw.push_back("invalid");
            row.insert(row.end(), sw.begin(), sw.end());
            row.push_back(num(s.z0));
            row.push_back(num(s.zEnd));
            for (double v : s.sp) row.push_back(num(v));
        };
        auto staticHead = [&](double len, const std::string& c10, const std::string& c11, const std::string& c12,
                              const std::string& c13) {
            std::vector<std::string> h = {"field", num(len), num(rmin), "0", "4", "0", "0", "0", "0", "0",
                                          c10, c11, c12, c13};
            for (int i = 0; i < 6; ++i) h.push_back("invalid");
            return h;
        };
        if (nRf == 0) {
            std::vector<std::string> row = staticHead(maxZEnd - minZ0, std::to_string(mode), "invalid", "0",
                                                      num(maxZEnd - minZ0));
            for (const Sub& s : subs) subBlock(s, row);
            add_field_row(row);
            if (mode != 0) {
                out[6] = lat_.elemZEnd.back();
                lat_.spOut.push_back(out);
                lat_.elements.back().inSuperposeOut = mode;
            }
        } else {
            const Sub& rf = subs[kRf];
            const auto& rw = rf.words;
            auto rfHead = [&](const std::string& c12, const std::string& c13, bool caseA) {
                // AVAS.dll writes one "invalid" too many when the cavity starts the block, which
                // shifts every sub-block of the cavity piece: the cavity gets no field at all.
                std::vector<std::string> h = {"field", rw[1], num(rmin), (compat_dll() && !caseA) ? "0" : rw[3], "4",
                                              rw[5], rw[6], rw[7], rw[8], "0", "invalid", "invalid", c12, c13,
                                              std::to_string(cav_)};
                const int pad = (caseA && compat_dll()) ? 6 : 5;
                for (int i = 0; i < pad; ++i) h.push_back("invalid");
                cav_ += 1;
                return h;
            };
            auto piece = [&](std::vector<std::string> row, auto keep) {
                for (const Sub& s : subs)
                    if (keep(s)) subBlock(s, row);
                add_field_row(row);
            };
            const double z0k = rf.z0, zEk = rf.zEnd;
            if (z0k == 0.0) {
                piece(rfHead("0", rw[1], true), [&](const Sub& s) { return s.z0 < zEk; });
                piece(staticHead(maxZEnd - zEk, "invalid", "invalid", num(zEk), num(maxZEnd)),
                      [&](const Sub& s) { return zEk < s.zEnd; });
            } else if (zEk == maxZEnd) {
                piece(staticHead(z0k, "invalid", "invalid", "0", num(z0k)), [&](const Sub& s) { return s.z0 < z0k; });
                piece(rfHead(num(z0k), num(zEk), false), [&](const Sub& s) { return z0k < s.zEnd; });
            } else {
                piece(staticHead(z0k, "invalid", "invalid", "0", num(z0k)), [&](const Sub& s) { return s.z0 < z0k; });
                piece(rfHead(num(z0k), num(zEk), false),
                      [&](const Sub& s) { return z0k < s.zEnd && s.z0 < zEk; });
                piece(staticHead(maxZEnd - zEk, "invalid", "invalid", num(zEk), num(maxZEnd)),
                      [&](const Sub& s) { return zEk < s.zEnd; });
            }
        }
        // AVAS.dll drops the line that ended the block, so an element written right after a block
        // without superposeend disappears; the corrected engine keeps it.
        if (!terminator.empty() && terminator[0] != "superposeend" && !compat_dll()) {
            say("lattice.txt: superpose block not closed by superposeend before \"" + terminator[0] + "\"");
            if (terminator[0] == "end") {
                --pos_;  // let the main loop see the end line
            } else {
                dispatch(terminator);
            }
        }
    }

    void finish() {
        if (lat_.elements.empty()) throw EngineError("The lattice contains no element");
        const double L = current_end();
        auto& planes = p_.outputPlanes;
        planes.push_back(L);
        if (!auto_.empty()) {
            planes.clear();
            for (const AutoRow& r : auto_) {
                if (!(r.spacing > 0.0)) throw EngineError("Output plane anomaly, check automaticoutput");
                for (double z = r.first; z <= r.span + r.first; z += r.spacing) planes.push_back(z);
            }
            if (planes.empty() || planes.back() != L) planes.push_back(L);
        }
        for (size_t i = 0; i + 1 < planes.size(); ++i)
            if (planes[i + 1] < planes[i]) throw EngineError("Output plane anomaly, check automaticoutput");
        if (lat_.spOut.empty()) {
            lat_.spOut.push_back({0, 0, 0, 0, 0, 0, L});
            lat_.spOutCount = 0;
        } else {
            lat_.spOutCount = static_cast<int>(lat_.spOut.size());
            if (L > lat_.spOut.back()[6]) lat_.spOut.push_back({0, 0, 0, 0, 0, 0, L});
        }
        Text t;
        t.s("Lattice initialisation complete, total length ").g(L).s(" m");
        say(t.str());
        say("A total of " + std::to_string(planes.size()) + " output planes");
        say("  ");
    }
};

}  // namespace

void init_lattice(Lattice& lat, Params& p) {
    LatticeReader(lat, p).run();
}

}  // namespace avas
