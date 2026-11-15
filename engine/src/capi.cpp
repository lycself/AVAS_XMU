// C interface of the engine (see include/avas_engine.h).
#include "avas_engine.h"

#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include "core/device.h"
#include "core/engine.h"
#include "core/format.h"
#include "core/options.h"
#include "core/tools.h"

#if __has_include("avas_source_hash.h")
#include "avas_source_hash.h"
#endif
#if __has_include("avas_build_config.h")
#include "avas_build_config.h"
#endif
#ifndef AVAS_ENGINE_SOURCE_HASH
#define AVAS_ENGINE_SOURCE_HASH "unknown"
#endif
#ifndef AVAS_BUILD_CUDA_ARCHS
#define AVAS_BUILD_CUDA_ARCHS ""
#endif
#ifndef AVAS_BUILD_CXX_COMPILER
#define AVAS_BUILD_CXX_COMPILER ""
#endif
#ifndef AVAS_BUILD_CUDA_COMPILER
#define AVAS_BUILD_CUDA_COMPILER ""
#endif
#ifndef AVAS_BUILD_TYPE
#define AVAS_BUILD_TYPE ""
#endif

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

// ---- JSON text of the query functions ----

std::string jstr(const std::string& s) {
    std::string o = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default:
                if (c < 0x20) {
                    char b[8];
                    std::snprintf(b, sizeof b, "\\u%04x", c);
                    o += b;
                } else {
                    o += static_cast<char>(c);
                }
        }
    }
    return o + "\"";
}

std::string jnum(double v) {
    char b[40];
    std::snprintf(b, sizeof b, "%.6g", v);
    return b;
}

std::string jbool(bool b) { return b ? "true" : "false"; }

std::string jints(const std::vector<int>& v) {
    std::string o = "[";
    for (size_t i = 0; i < v.size(); ++i) o += (i ? "," : "") + std::to_string(v[i]);
    return o + "]";
}

std::string short_hash() { return std::string(AVAS_ENGINE_SOURCE_HASH).substr(0, 16); }

std::string device_name(avas::Device d) {
    switch (d) {
        case avas::Device::Cpu: return "cpu";
        case avas::Device::Cuda: return "cuda";
        case avas::Device::Auto: return "auto";
        case avas::Device::Emu: return "emu";
        case avas::Device::Metal: return "metal";
    }
    return "cpu";
}

std::string precision_name(avas::Precision p) {
    return p == avas::Precision::Fp64 ? "fp64" : p == avas::Precision::Fp32 ? "fp32" : "auto";
}

std::string plan_json(const avas::DevicePlan& p) {
    if (!p.error.empty())
        return "{\"device\":null,\"precision\":null,\"gpus\":[],\"reason\":null,\"reason_code\":null,\"error\":" +
               jstr(p.error) + "}";
    return "{\"device\":" + jstr(p.device) + ",\"precision\":" + jstr(p.precision) + ",\"gpus\":" + jints(p.gpus) +
           ",\"reason\":" + jstr(p.reason) + ",\"reason_code\":" + jstr(p.code) +
           (p.cpuBelowParticlesPerCell > 0.0 ? ",\"cpu_below_particles_per_cell\":" + avas::fmt_g(p.cpuBelowParticlesPerCell) : std::string()) +
           (p.cpuBelowParticles > 0 ? ",\"cpu_below_particles\":" + std::to_string(p.cpuBelowParticles) : std::string()) +
           ",\"error\":null}";
}

// {"sc": plan, "nosc": plan} for the current options (device replaced by *dev* when given)
std::string plans_json(const avas::Device* dev) {
    avas::Options& o = avas::options();
    const avas::Device saved = o.device;
    if (dev) o.device = *dev;
    const std::string sc = plan_json(avas::plan_device(0, true));
    const std::string nosc = plan_json(avas::plan_device(0, false));
    o.device = saved;
    return "{\"sc\":" + sc + ",\"nosc\":" + nosc + "}";
}

std::string platform_name() {
#if defined(_WIN32)
    std::string os = "windows";
#elif defined(__APPLE__)
    std::string os = "macos";
#elif defined(__linux__)
    std::string os = "linux";
#else
    std::string os = "other";
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
    return os + "-arm64";
#elif defined(__x86_64__) || defined(_M_X64)
    return os + "-x86_64";
#else
    return os;
#endif
}

std::string trimmed(std::string s) {
    while (!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

std::string compiler_name() {
#if defined(__clang__)
    return trimmed(std::string("clang ") + __clang_version__);
#elif defined(_MSC_VER)
    return trimmed("MSVC " + std::to_string(_MSC_FULL_VER));
#elif defined(__GNUC__)
    return trimmed(std::string("gcc ") + __VERSION__);
#else
    return "unknown";
#endif
}

bool mpi_compiled() {
#if defined(AVAS_WITH_MPI)
    return true;
#else
    return false;
#endif
}

std::string info_json() {
    std::string archs = AVAS_BUILD_CUDA_ARCHS;
    std::string list = "[";
    size_t start = 0;
    bool first = true;
    while (start < archs.size()) {
        size_t end = archs.find(',', start);
        if (end == std::string::npos) end = archs.size();
        if (end > start) {
            list += (first ? "" : ",") + jstr(archs.substr(start, end - start));
            first = false;
        }
        start = end + 1;
    }
    list += "]";
    return "{\"version\":" + jstr(short_hash()) + ",\"source_hash\":" + jstr(AVAS_ENGINE_SOURCE_HASH) +
           ",\"build\":{\"cuda\":" + jbool(avas::cuda_compiled()) + ",\"metal\":" + jbool(avas::metal_compiled()) +
           ",\"mpi\":" + jbool(mpi_compiled()) + ",\"cuda_archs\":" + list +
           ",\"build_type\":" + jstr(AVAS_BUILD_TYPE) + "},\"compiler\":" + jstr(compiler_name()) +
           ",\"cmake_compiler\":" + jstr(AVAS_BUILD_CXX_COMPILER) + ",\"cuda_compiler\":" + jstr(AVAS_BUILD_CUDA_COMPILER) +
           ",\"platform\":" + jstr(platform_name()) + "}";
}

std::string devices_json() {
    std::string why;
    const std::vector<avas::CudaDeviceInfo> cuda = avas::cuda_devices(why);
    std::string cl = "[";
    for (size_t i = 0; i < cuda.size(); ++i) {
        const avas::CudaDeviceInfo& d = cuda[i];
        cl += std::string(i ? "," : "") + "{\"index\":" + std::to_string(d.index) + ",\"name\":" + jstr(d.name) +
              ",\"compute\":" + jstr(std::to_string(d.major) + "." + std::to_string(d.minor)) +
              ",\"fp64_full\":" + jbool(d.fp64Full) + ",\"memory_gb\":" + jnum(d.memoryGB) + "}";
    }
    cl += "]";
    const std::string cudaNote = cuda.empty() ? jstr(why) : "null";
    std::string name, mwhy;
    const bool metalOk = avas::metal_probe(name, mwhy);
    std::string ml = "[]";
    if (!name.empty()) ml = "[{\"name\":" + jstr(name) + ",\"usable\":" + jbool(metalOk) + "}]";
    const std::string metalNote = metalOk ? "null" : jstr(mwhy);
    const avas::Options& o = avas::options();
    const unsigned hw = std::thread::hardware_concurrency();
    const avas::Device autoDev = avas::Device::Auto;
    return "{\"cuda\":{\"compiled\":" + jbool(avas::cuda_compiled()) + ",\"devices\":" + cl + ",\"note\":" + cudaNote +
           "},\"metal\":{\"compiled\":" + jbool(avas::metal_compiled()) + ",\"devices\":" + ml + ",\"note\":" + metalNote +
           "},\"cpu\":{\"threads\":" + std::to_string(hw ? hw : 1) + "}" +
           ",\"options\":{\"device\":" + jstr(device_name(o.device)) + ",\"precision\":" + jstr(precision_name(o.precision)) +
           ",\"gpus\":" + jstr(o.gpus) + ",\"compat\":" + jstr(o.compat == avas::Compat::Dll ? "dll" : "none") +
           ",\"threads\":" + std::to_string(o.threads) + "}" +
           ",\"auto\":" + plans_json(&autoDev) + ",\"selected\":" + plans_json(nullptr) + "}";
}

std::string last_run_json() {
    const avas::LastRun& r = avas::last_run();
    if (!r.ran)
        return "{\"ran\":false,\"device\":null,\"precision\":null,\"gpus\":null,\"compat\":null,\"threads\":null,"
               "\"space_charge\":null,\"reason\":null,\"reason_code\":null,\"seconds\":null,\"status\":null}";
    const bool tracked = !r.device.empty();
    return "{\"ran\":true,\"device\":" + (tracked ? jstr(r.device) : std::string("null")) +
           ",\"precision\":" + (tracked ? jstr(r.precision) : std::string("null")) + ",\"gpus\":" + jints(r.gpus) +
           ",\"compat\":" + jstr(r.compat) + ",\"threads\":" + std::to_string(r.threads) +
           ",\"space_charge\":" + (r.spaceCharge < 0 ? std::string("null") : jbool(r.spaceCharge == 1)) +
           ",\"reason\":" + (tracked ? jstr(r.reason) : std::string("null")) +
           ",\"reason_code\":" + (tracked ? jstr(r.reasonCode) : std::string("null")) +
           ",\"seconds\":" + jnum(r.seconds) + ",\"status\":" + std::to_string(r.status) + "}";
}

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

// The returned text lives in a buffer of the calling thread: valid until that thread calls the same
// function again (see include/avas_engine.h).
AVAS_API const char* avas_engine_version(void) {
    static const std::string v = short_hash();
    return v.c_str();
}

AVAS_API const char* avas_engine_info(void) {
    thread_local std::string buf;
    buf = info_json();
    return buf.c_str();
}

AVAS_API const char* avas_engine_devices(void) {
    avas::init_options_from_env();
    thread_local std::string buf;
    buf = devices_json();
    return buf.c_str();
}

AVAS_API const char* avas_engine_last_run(void) {
    thread_local std::string buf;
    buf = last_run_json();
    return buf.c_str();
}

}  // extern "C"
