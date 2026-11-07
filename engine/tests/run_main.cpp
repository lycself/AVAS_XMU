// Command line driver: avas_engine_run INPUT_DIR OUTPUT_DIR FIELD_DIR [compat]
#include <cstdio>
#include <cstdlib>
#include <string>

#include "avas_engine.h"

#if defined(_WIN32)
#include <windows.h>
static std::wstring wide(const char* s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s, -1, &w[0], n);
    w.resize(static_cast<size_t>(n > 0 ? n - 1 : 0));
    return w;
}
#endif

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "usage: %s INPUT_DIR OUTPUT_DIR FIELD_DIR [none|dll]\n", argv[0]);
        return 64;
    }
    if (argc > 4 && avas_set_option("compat", argv[4]) != 0) {
        std::fprintf(stderr, "invalid compat value %s\n", argv[4]);
        return 64;
    }
#if defined(_WIN32)
    path(wide(argv[1]).c_str(), wide(argv[2]).c_str(), wide(argv[3]).c_str());
#else
    path(argv[1], argv[2], argv[3]);
#endif
    int state = 1;
    return main_agent(&state);
}
