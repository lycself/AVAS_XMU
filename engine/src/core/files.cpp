#include "core/files.h"

#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#endif

namespace avas {

Paths& paths() {
    static Paths p;
    return p;
}

#if defined(_WIN32)
static std::wstring widen(const std::string& s) {
    if (s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), nullptr, 0);
    UINT cp = CP_UTF8;
    DWORD flags = MB_ERR_INVALID_CHARS;
    if (n <= 0) {  // not UTF-8 (e.g. compat=dll path bytes): use the ANSI code page
        cp = CP_ACP;
        flags = 0;
        n = MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), nullptr, 0);
    }
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), &w[0], n);
    return w;
}
#endif

std::FILE* open_file(const std::string& path, const char* mode) {
#if defined(_WIN32)
    std::wstring wmode(mode, mode + std::char_traits<char>::length(mode));
    return _wfopen(widen(path).c_str(), wmode.c_str());
#else
    return std::fopen(path.c_str(), mode);
#endif
}

bool file_exists(const std::string& path) {
    std::FILE* f = open_file(path, "rb");
    if (!f) return false;
    std::fclose(f);
    return true;
}

bool read_file(const std::string& path, std::string& out) {
    std::FILE* f = open_file(path, "rb");
    if (!f) return false;
    out.clear();
    char buf[1 << 16];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) out.append(buf, n);
    std::fclose(f);
    return true;
}

bool write_file(const std::string& path, const std::string& data, bool append, bool binary) {
    std::FILE* f = open_file(path, binary ? (append ? "ab" : "wb") : (append ? "a" : "w"));
    if (!f) return false;
    if (!data.empty()) std::fwrite(data.data(), 1, data.size(), f);
    std::fclose(f);
    return true;
}

bool make_dir(const std::string& path) {
#if defined(_WIN32)
    return _wmkdir(widen(path).c_str()) == 0;
#else
    return mkdir(path.c_str(), 0775) == 0;
#endif
}

bool AppendFile::open(const std::string& path, bool binary) {
    close();
    f_ = open_file(path, binary ? "ab" : "a");
    return f_ != nullptr;
}

void AppendFile::write(const std::string& data) {
    if (!f_) return;
    std::fwrite(data.data(), 1, data.size(), f_);
    std::fflush(f_);
}

void AppendFile::close() {
    if (f_) std::fclose(f_);
    f_ = nullptr;
}

}  // namespace avas
