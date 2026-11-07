// Directory prefixes set by path() and small file helpers.
//
// Paths are kept as byte strings exactly like the original (prefix + "/" + name, no
// normalisation), so messages that show a path are identical.  On Windows the strings are UTF-8
// and are converted to UTF-16 when a file is opened (compat=dll keeps the original's lossy
// wide-to-narrow conversion, see capi.cpp).
#pragma once
#include <cstdio>
#include <string>

namespace avas {

struct Paths {
    std::string input = "./inputFile/";
    std::string field = "./inputFile/";
    std::string output = "./outputFile/";
};
Paths& paths();

std::FILE* open_file(const std::string& path, const char* mode);
bool file_exists(const std::string& path);
bool read_file(const std::string& path, std::string& out);
// Truncates (append=false) or appends; returns false when the file cannot be opened.
// Text mode (binary=false) writes "\r\n" line ends on Windows, like the original's ofstream.
bool write_file(const std::string& path, const std::string& data, bool append, bool binary = false);
bool make_dir(const std::string& path);

// A text file kept open for appending; every write() is flushed (live readers follow it).
class AppendFile {
public:
    AppendFile() = default;
    ~AppendFile() { close(); }
    AppendFile(const AppendFile&) = delete;
    AppendFile& operator=(const AppendFile&) = delete;
    bool open(const std::string& path, bool binary = false);
    void write(const std::string& data);
    void close();
    bool is_open() const { return f_ != nullptr; }
private:
    std::FILE* f_ = nullptr;
};

}  // namespace avas
