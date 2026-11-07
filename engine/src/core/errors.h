// Error kinds of the engine.
//
// The original kernel has three ways to end abnormally (spec 10, 2.1.2):
//   * a std::string exception -> ErrorLog.txt + return code 1 (initialisation) or 2 (simulation);
//   * a non-string exception (stoi/stod failure, vector::at) -> std::terminate (process abort);
//   * exit(1) after printing a message.
// EngineError is the first kind.  The other two are reproduced only in "compat=dll" mode
// (compat_abort / compat_exit); by default they are reported as EngineError as well.
#pragma once
#include <stdexcept>
#include <string>

namespace avas {

class EngineError : public std::runtime_error {
public:
    explicit EngineError(const std::string& msg) : std::runtime_error(msg) {}
};

// Thrown by number parsing (stoi/stod semantics); see text.h.
class NumberError : public std::runtime_error {
public:
    explicit NumberError(const std::string& what) : std::runtime_error(what) {}
};

// Ends the process like the original kernel's exit(code).  Flushes stdout first.
[[noreturn]] void compat_exit(int code);
// Ends the process like an uncaught non-string exception in the original kernel.
[[noreturn]] void compat_abort(const std::string& what);

}  // namespace avas
