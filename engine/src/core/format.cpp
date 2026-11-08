#include "core/format.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>

namespace avas {

namespace {

bool append_special(std::string& out, double v) {
    if (std::isnan(v)) {
        std::uint64_t bits;
        std::memcpy(&bits, &v, sizeof bits);
        const bool neg = (bits >> 63) != 0;
        const std::uint64_t payload = bits & 0x000FFFFFFFFFFFFFull;
        if (neg && payload == 0x0008000000000000ull) {
            out += "-nan(ind)";
        } else if (payload & 0x0008000000000000ull) {
            out += neg ? "-nan" : "nan";
        } else {
            out += neg ? "-nan(snan)" : "nan(snan)";
        }
        return true;
    }
    if (std::isinf(v)) {
        out += v < 0 ? "-inf" : "inf";
        return true;
    }
    return false;
}

}  // namespace

void append_g(std::string& out, double v, int precision) {
    if (append_special(out, v)) return;
    char buf[64];
    int n = std::snprintf(buf, sizeof buf, "%.*g", precision, v);
    out.append(buf, static_cast<size_t>(n));
}

void append_fixed(std::string& out, double v, int precision) {
    if (append_special(out, v)) return;
    char buf[400];
    int n = std::snprintf(buf, sizeof buf, "%.*f", precision, v);
    if (n >= static_cast<int>(sizeof buf)) n = static_cast<int>(sizeof buf) - 1;
    out.append(buf, static_cast<size_t>(n));
}

std::string fmt_g(double v, int precision) {
    std::string s;
    append_g(s, v, precision);
    return s;
}

std::string fmt_fixed(double v, int precision) {
    std::string s;
    append_fixed(s, v, precision);
    return s;
}

static bool g_quiet = false;
void set_console_quiet(bool q) { g_quiet = q; }

void say(const std::string& line) {
    if (g_quiet) return;
    std::fwrite(line.data(), 1, line.size(), stdout);
    std::fputc('\n', stdout);
    std::fflush(stdout);
}

void say_raw(const std::string& text) {
    if (g_quiet) return;
    std::fwrite(text.data(), 1, text.size(), stdout);
    std::fflush(stdout);
}

std::string time_stamp() {
    std::time_t t = std::time(nullptr);
    std::tm tmv{};
#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[64];
    std::strftime(buf, sizeof buf, "%Y/%m/%d %H:%M:%S  ", &tmv);
    return buf;
}

}  // namespace avas
