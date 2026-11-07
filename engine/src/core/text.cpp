#include "core/text.h"

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdlib>

#include "core/errors.h"

namespace avas {

std::string normalize_line_endings(const std::string& raw) {
    std::string s;
    s.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i) {
        char c = raw[i];
        if (c == '\r') {
            s.push_back('\n');
            if (i + 1 < raw.size() && raw[i + 1] == '\n') ++i;
        } else {
            s.push_back(c);
        }
    }
    return s;
}

std::vector<std::string> split_words(const std::string& s) {
    std::vector<std::string> out;
    const size_t n = s.size();
    size_t start = 0;
    for (size_t i = 0; i < n; ++i) {
        const char c = s[i];
        if (c == '!') break;
        if (c == ' ' || c == '\t') {
            if (start == i) {
                start = i + 1;
            } else {
                out.push_back(s.substr(start, i - start));
                start = i + 1;
            }
        } else if (i == n - 1) {
            out.push_back(s.substr(start, n - start));
        }
    }
    return out;
}

std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> lines;
    size_t pos = 0;
    while (pos < text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string::npos) {
            lines.push_back(text.substr(pos));
            break;
        }
        lines.push_back(text.substr(pos, nl - pos));
        pos = nl + 1;
    }
    return lines;
}

int parse_int(const std::string& s) {
    const char* p = s.c_str();
    char* end = nullptr;
    errno = 0;
    long v = std::strtol(p, &end, 10);
    if (end == p) throw NumberError("stoi");
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) throw NumberError("stoi");
    return static_cast<int>(v);
}

double parse_double(const std::string& s) {
    const char* p = s.c_str();
    char* end = nullptr;
    errno = 0;
    double v = std::strtod(p, &end);
    if (end == p) throw NumberError("stod");
    if (errno == ERANGE) throw NumberError("stod");
    return v;
}

double parse_double_lenient(const std::string& s) { return std::strtod(s.c_str(), nullptr); }

std::string to_lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string extension(const std::string& name, bool& ok) {
    size_t dot = name.rfind('.');
    ok = dot != std::string::npos;
    return ok ? name.substr(dot + 1) : std::string();
}

}  // namespace avas
