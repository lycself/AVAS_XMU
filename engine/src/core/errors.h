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
// Ends the process like an uncaught non-string exception in the original kernel.  On Windows the
// exception leaves the DLL as the MSVC C++ exception 0xE06D7363 (ctypes: OSError "Windows Error
// 0xe06d7363"); elsewhere the libstdc++ terminate message and abort().
[[noreturn]] void compat_abort(const std::string& what);
// Ends the process like the original kernel's read through a null pointer (Windows: access
// violation exception, ctypes: OSError "access violation reading 0x0000000000000000"; elsewhere
// SIGSEGV).
[[noreturn]] void compat_crash(const std::string& what);

}  // namespace avas
