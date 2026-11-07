#include "core/fieldmap.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/text.h"

namespace avas {

namespace {

// Whitespace token reader over a text buffer (equivalent of istream >> for these files).
struct TextReader {
    const std::string& s;
    size_t pos = 0;
    explicit TextReader(const std::string& text) : s(text) {}
    bool token(std::string& out) {
        while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) ++pos;
        if (pos >= s.size()) return false;
        size_t b = pos;
        while (pos < s.size() && !std::isspace(static_cast<unsigned char>(s[pos]))) ++pos;
        out.assign(s, b, pos - b);
        return true;
    }
    double number() {
        while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) ++pos;
        const char* p = s.c_str() + pos;
        char* end = nullptr;
        double v = std::strtod(p, &end);
        if (end == p) return 0.0;  // extraction failure leaves 0 (C++11 istream behaviour)
        pos += static_cast<size_t>(end - p);
        return v;
    }
    int integer() {
        while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) ++pos;
        const char* p = s.c_str() + pos;
        char* end = nullptr;
        long v = std::strtol(p, &end, 10);
        if (end == p) return 0;
        pos += static_cast<size_t>(end - p);
        return static_cast<int>(v);
    }
    // std::getline: rest of the current line.
    std::string line() {
        size_t nl = s.find('\n', pos);
        std::string out = s.substr(pos, nl == std::string::npos ? std::string::npos : nl - pos);
        pos = nl == std::string::npos ? s.size() : nl + 1;
        if (!out.empty() && out.back() == '\r') out.pop_back();
        return out;
    }
};

[[noreturn]] void cannot_open(const std::string& file) {
    throw EngineError("Element field file cannot be opened : " + file);
}

void load_text_3d(FieldMap& m, const std::string& name, const char* const* exts, int ncomp) {
    for (int c = 0; c < ncomp; ++c) {
        const std::string file = name + exts[c];
        std::string raw;
        if (!read_file(file, raw)) cannot_open(file);
        TextReader r(raw);
        int norm = 1;
        if (c == 0) {
            m.nz = r.integer();
            m.zlen = r.number();
            m.nx = r.integer();
            m.xmin = r.number();
            m.xmax = r.number();
            m.ny = r.integer();
            m.ymin = r.number();
            m.ymax = r.number();
            m.dx = (m.xmax - m.xmin) / static_cast<double>(m.nx);
            m.dy = (m.ymax - m.ymin) / static_cast<double>(m.ny);
            m.dz = m.zlen / static_cast<double>(m.nz);
            m.data.assign(static_cast<size_t>(m.nx + 1) * (m.ny + 1) * (m.nz + 1) * ncomp, 0.0);
            r.line();
            norm = parse_int(r.line());
        } else {
            for (int k = 0; k < 3; ++k) r.line();
            norm = parse_int(r.line());
        }
        const double scale = 1.0 / static_cast<double>(norm);
        const size_t nodes = static_cast<size_t>(m.nx + 1) * (m.ny + 1) * (m.nz + 1);
        for (size_t n = 0; n < nodes; ++n) m.data[n * ncomp + c] = r.number() * scale;
    }
}

bool load_text_1d(FieldMap& m, const std::string& name) {
    const std::string file = name + ".edz";
    std::string raw;
    if (!read_file(file, raw)) cannot_open(file);
    TextReader r(raw);
    m.nz = r.integer();
    m.zlen = r.number();
    r.line();
    const std::string markLine = r.line();
    if (parse_int(markLine) != 1) say("mark error : " + file + "  " + markLine);
    m.nx = m.ny = 2;
    m.xmin = m.ymin = -1.0;
    m.xmax = m.ymax = 1.0;
    m.dx = m.dy = 1.0;
    m.dz = m.zlen / static_cast<double>(m.nz);
    m.data.assign(static_cast<size_t>(9) * (m.nz + 1) * 6, 0.0);
    for (int iz = 0; iz <= m.nz; ++iz) {
        const double ez = r.number();
        for (int j = 0; j < 9; ++j) m.data[(static_cast<size_t>(iz) * 9 + j) * 6 + 2] = ez;
    }
    return true;
}

void load_binary(FieldMap& m, const std::string& name, const char* const* exts, int ncomp) {
    for (int c = 0; c < ncomp; ++c) {
        const std::string file = name + exts[c];
        std::string raw;
        if (!read_file(file, raw)) cannot_open(file);
        size_t pos = 0;
        auto get = [&](void* dst, size_t n) {
            if (pos + n > raw.size()) throw EngineError("Element field file is truncated : " + file);
            std::memcpy(dst, raw.data() + pos, n);
            pos += n;
        };
        std::int32_t nz, nx, ny;
        double zlen, xmin, xmax, ymin, ymax, norm;
        get(&nz, 4); get(&zlen, 8); get(&nx, 4); get(&xmin, 8); get(&xmax, 8);
        get(&ny, 4); get(&ymin, 8); get(&ymax, 8); get(&norm, 8);
        m.nz = nz; m.zlen = zlen; m.nx = nx; m.xmin = xmin; m.xmax = xmax;
        m.ny = ny; m.ymin = ymin; m.ymax = ymax;
        if (c == 0) {
            m.dx = (m.xmax - m.xmin) / static_cast<double>(m.nx);
            m.dy = (m.ymax - m.ymin) / static_cast<double>(m.ny);
            m.dz = m.zlen / static_cast<double>(m.nz);
            m.data.assign(static_cast<size_t>(m.nx + 1) * (m.ny + 1) * (m.nz + 1) * ncomp, 0.0);
        }
        const double scale = 1.0 / norm;
        const size_t nodes = m.data.size() / ncomp;
        for (size_t n = 0; n < nodes; ++n) {
            float v;
            get(&v, 4);
            m.data[n * ncomp + c] = static_cast<double>(v) * scale;
        }
    }
}

}  // namespace

int FieldLibrary::load(const std::string& name, int type) {
    for (size_t i = 0; i < maps.size(); ++i)
        if (maps[i].name == name) return static_cast<int>(i);
    static const char* const rf[] = {".edx", ".edy", ".edz", ".bdx", ".bdy", ".bdz"};
    static const char* const es[] = {".esx", ".esy", ".esz"};
    static const char* const bs[] = {".bsx", ".bsy", ".bsz"};
    const char* const* exts = nullptr;
    int ncomp = 0;
    if (type == 1) { exts = rf; ncomp = 6; }
    else if (type == 2) { exts = es; ncomp = 3; }
    else if (type == 3) { exts = bs; ncomp = 3; }
    else return -1;

    FieldMap m;
    m.name = name;
    m.ncomp = ncomp;
    const bool binary = !name.empty() && name.back() == 'b';
    if (binary) {
        load_binary(m, name, exts, ncomp);
    } else if (type == 1 && !file_exists(name + ".edx")) {
        say("The one-dimensional field has been expanded into a three-dimensional field.          ");
        load_text_1d(m, name);
    } else {
        load_text_3d(m, name, exts, ncomp);
    }
    maps.push_back(std::move(m));
    return static_cast<int>(maps.size()) - 1;
}

}  // namespace avas
