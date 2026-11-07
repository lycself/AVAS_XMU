// Text input helpers with the exact semantics of the original kernel (spec 10, 2.2).
#pragma once
#include <string>
#include <vector>

namespace avas {

// Whole-file read; "\r\n" -> "\n", then remaining "\r" -> "\n".  Empty string if unreadable.
std::string normalize_line_endings(const std::string& raw);

// Splits a line at spaces and tabs; '!' ends the line and drops the word it touches
// (so "1000!c" yields nothing for that word, as in the original StringSplit).
std::vector<std::string> split_words(const std::string& line);

// Splits normalised text into lines (handles a last line without "\n").
std::vector<std::string> split_lines(const std::string& text);

// std::stoi semantics (strtol base 10, trailing garbage ignored); throws NumberError("stoi").
int parse_int(const std::string& s);
// std::stod semantics (strtod, C locale); throws NumberError("stod").
double parse_double(const std::string& s);
// strtod without any check (the original uses bare strtod in a few places).
double parse_double_lenient(const std::string& s);

std::string to_lower(std::string s);
// Extension after the last '.', or empty with ok=false when there is no '.'.
std::string extension(const std::string& name, bool& ok);

}  // namespace avas
