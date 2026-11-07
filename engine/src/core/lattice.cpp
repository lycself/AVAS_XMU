#include "core/lattice.h"

#include <cmath>

#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/options.h"
#include "core/physics.h"
#include "core/text.h"

namespace avas {

namespace {

// Keywords the original kernel knows but the new engine does not implement yet.
bool is_unimplemented(const std::string& k) {
    static const char* const names[] = {
        "superpose", "multipole", "quad", "solenoid", "steerer", "bend", "edge", "rfq",
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

class LatticeReader {
public:
    LatticeReader(Lattice& lat, Params& p) : lat_(lat), p_(p) {}

    void run() {
        std::string raw;
        if (!read_file(paths().input + "lattice.txt", raw)) {
            throw EngineError("File \"lattice.txt\" cannot be opened, Check the inputFile routing and lattice.txt.");
        }
        say("Initialising lattice...");
        bool skip = true;
        bool ended = false;
        for (const std::string& line : split_lines(normalize_line_endings(raw))) {
            std::vector<std::string> w = split_words(line);
            if (w.empty()) continue;
            if (skip && w[0] != "start") continue;
            skip = false;
            if (w[0] == "end") {
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
        } else if (k == "field") {
            add_field(w);
        } else if (k == "spacechargecomp") {
            // AVAS.dll only: space-charge compensation factor; space charge is not in M1.
        } else if (is_unimplemented(k)) {
            throw EngineError("Lattice keyword \"" + k + "\" is not implemented in the new engine yet");
        } else if (!is_ignored(k) && !compat_dll()) {
            say("lattice.txt: unknown keyword \"" + k + "\" ignored");
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

    void add_drift(const std::vector<std::string>& w) {
        if (w.size() < 4 && !compat_dll()) {
            throw EngineError("Error! parameter not enough in element : drift");
        }
        if (w.size() < 2) throw EngineError("Error! parameter not enough in element : drift");
        Element& e = push_element(w);
        e.kind = ElemKind::Drift;
        if (w.size() < 4) {
            // AVAS.dll: message, element stays with its default values (L = R = zStart = 0).
            print_not_enough(w);
            return;
        }
        set_basic(e, w);
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
        const int type = parse_int(w[4]);
        if (type == 4) throw EngineError("Field type 4 (superpose) is not implemented in the new engine yet");
        Element& e = push_element(w);
        const int idx = lat_.fm.load(w[9], type);
        w[9] = std::to_string(idx);
        e.kind = ElemKind::Field;
        set_basic(e, w);
        e.fieldType = type;
        e.frequency = parse_double(w[5]);
        e.phaseRad = parse_double(w[6]) / 180.0 * k::pi;
        e.ke = parse_double(w[7]);
        e.kb = parse_double(w[8]);
        e.mapIndex = idx;
        if (type == 1) {
            if (e.frequency == 0.0) {
                throw EngineError("RF cavity frequency cannot be set to 0, Check the parameters of the RF cavity elements");
            }
            e.cavIndex = parse_int(w.at(14));
        } else if (type != 0 && type != 2 && type != 3) {
            say("Field type error!");
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
        lat_.spOut.push_back({0, 0, 0, 0, 0, 0, L});
        lat_.spOutCount = 0;
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
