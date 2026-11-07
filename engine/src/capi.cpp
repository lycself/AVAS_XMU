// C interface of the engine (see include/avas_engine.h).
#include "avas_engine.h"

#include <string>

#include "core/engine.h"
#include "core/options.h"
#include "core/tools.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {

#if defined(_WIN32)
std::string narrow(const wchar_t* w) {
    if (!w) return std::string();
    if (avas::compat_dll()) {
        // AVAS.dll copies every non-zero byte of the UTF-16 string (non-ASCII paths break).
        const unsigned char* p = reinterpret_cast<const unsigned char*>(w);
        std::string s;
        for (size_t i = 0;; ++i) {
            if (p[i] == 0 && p[i + 1] == 0 && (i % 2 == 0)) break;
            if (p[i] != 0) s.push_back(static_cast<char>(p[i]));
        }
        return s;
    }
    int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    std::string s(static_cast<size_t>(n > 0 ? n - 1 : 0), '\0');
    if (n > 1) WideCharToMultiByte(CP_UTF8, 0, w, -1, &s[0], n, nullptr, nullptr);
    return s;
}
#endif

}  // namespace

extern "C" {

#if defined(_WIN32)
AVAS_API int path(const wchar_t* input, const wchar_t* output, const wchar_t* field) {
    avas::init_options_from_env();
    avas::set_paths(narrow(input), narrow(output), narrow(field));
    return 0;
}
#else
AVAS_API int path(const char* input, const char* output, const char* field) {
    avas::init_options_from_env();
    avas::set_paths(input ? input : "", output ? output : "", field ? field : "");
    return 0;
}
#endif

AVAS_API int main_agent(int* state) { return avas::run_main_agent(state); }

#if defined(_WIN32)
#define AVAS_STR(s) narrow(s)
#define AVAS_CH wchar_t
#else
#define AVAS_STR(s) std::string((s) ? (s) : "")
#define AVAS_CH char
#endif

AVAS_API int sampling(const AVAS_CH* in, const AVAS_CH* out, int total, int segFrom, int segTo, int magFrom, int magTo,
                      int nodes, double halfWidth, double terminalCoefficient) {
    avas::init_options_from_env();
    return avas::tool_sampling(AVAS_STR(in), AVAS_STR(out), total, segFrom, segTo, magFrom, magTo, nodes, halfWidth,
                               terminalCoefficient);
}

AVAS_API int meshing(const AVAS_CH* in, const AVAS_CH* out, int nodes, double halfWidth) {
    avas::init_options_from_env();
    return avas::tool_meshing(AVAS_STR(in), AVAS_STR(out), nodes, halfWidth);
}

AVAS_API int dataprocess(const AVAS_CH* root, int i0, int i1, double nodesFiveCube, double nodesOriginal, double halfWidth,
                         int useWeights) {
    avas::init_options_from_env();
    return avas::tool_dataprocess(AVAS_STR(root), i0, i1, nodesFiveCube, nodesOriginal, halfWidth, useWeights);
}

AVAS_API int avas_set_option(const char* key, const char* value) {
    avas::init_options_from_env();
    if (!key) return -1;
    return avas::set_option(key, value ? value : "");
}

AVAS_API const char* avas_engine_version(void) { return "avas-engine M5"; }

}  // extern "C"
