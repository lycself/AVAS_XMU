// Number formatting identical to the std::ostream output of the original kernel (MSVC).
//
// Default stream format = printf "%.<p>g"; std::fixed = "%.<p>f".  NaN and infinity are printed
// the MSVC way ("inf", "-inf", "nan", "-nan(ind)"), because the reference outputs come from AVAS.dll.
#pragma once
#include <cstdint>
#include <string>

namespace avas {

void append_g(std::string& out, double v, int precision = 6);
void append_fixed(std::string& out, double v, int precision);
std::string fmt_g(double v, int precision = 6);
std::string fmt_fixed(double v, int precision);

// Small append-only text builder used by all output writers.
class Text {
public:
    Text& g(double v, int precision = 6) { append_g(s_, v, precision); return *this; }
    Text& fixed(double v, int precision) { append_fixed(s_, v, precision); return *this; }
    Text& i(long long v) { s_ += std::to_string(v); return *this; }
    Text& s(const std::string& t) { s_ += t; return *this; }
    Text& s(const char* t) { s_ += t; return *this; }
    Text& nl() { s_ += '\n'; return *this; }
    const std::string& str() const { return s_; }
    std::string& str() { return s_; }
    void clear() { s_.clear(); }
private:
    std::string s_;
};

// Console output (stdout, flushed): say() appends "\n"; say_raw() writes as is.
void say(const std::string& line);
// MPI ranks other than 0 print nothing.
void set_console_quiet(bool q);
void say_raw(const std::string& text);

// "%Y/%m/%d %X  " time stamp (local time) used by ErrorLog.txt and synParticle.txt.
std::string time_stamp();

}  // namespace avas
