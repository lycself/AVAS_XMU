#include "core/options.h"

#include <cstdlib>
#include <mutex>
#include <thread>

namespace avas {

Options& options() {
    static Options opts;
    return opts;
}

int set_option(const std::string& key, const std::string& value) {
    if (key == "compat") {
        if (value == "none" || value == "default" || value.empty()) {
            options().compat = Compat::None;
        } else if (value == "dll") {
            options().compat = Compat::Dll;
        } else {
            return -2;
        }
        return 0;
    }
    if (key == "threads") {
        char* end = nullptr;
        long n = std::strtol(value.c_str(), &end, 10);
        if (end == value.c_str() || n < 0) return -2;
        options().threads = static_cast<int>(n);
        return 0;
    }
    if (key == "device") {
        if (value == "cpu" || value.empty()) options().device = Device::Cpu;
        else if (value == "cuda" || value == "gpu") options().device = Device::Cuda;
        else if (value == "auto") options().device = Device::Auto;
        else if (value == "emu") options().device = Device::Emu;
        else return -2;
        return 0;
    }
    if (key == "precision") {
        if (value == "auto" || value.empty()) options().precision = Precision::Auto;
        else if (value == "fp64") options().precision = Precision::Fp64;
        else if (value == "fp32") options().precision = Precision::Fp32;
        else return -2;
        return 0;
    }
    if (key == "gpus") {
        if (!value.empty() && value != "all") {
            for (char ch : value)
                if (!((ch >= '0' && ch <= '9') || ch == ',' || ch == ' ')) return -2;
        }
        options().gpus = value;
        return 0;
    }
    if (key == "gpu_atomic") {  // measurement only (not deterministic)
        options().gpuAtomic = value == "1";
        return 0;
    }
    return -1;
}

void init_options_from_env() {
    static std::once_flag once;
    std::call_once(once, [] {
        if (const char* c = std::getenv("AVAS_ENGINE_COMPAT")) set_option("compat", c);
        if (const char* t = std::getenv("AVAS_ENGINE_THREADS")) set_option("threads", t);
        if (const char* d = std::getenv("AVAS_ENGINE_DEVICE")) set_option("device", d);
        if (const char* q = std::getenv("AVAS_ENGINE_PRECISION")) set_option("precision", q);
        if (const char* g = std::getenv("AVAS_ENGINE_GPUS")) set_option("gpus", g);
        if (const char* a = std::getenv("AVAS_ENGINE_GPU_ATOMIC")) set_option("gpu_atomic", a);
    });
}

int omp_max_threads() {
    if (const char* s = std::getenv("OMP_NUM_THREADS")) {
        int n = std::atoi(s);
        if (n > 0) return n;
    }
    unsigned n = std::thread::hardware_concurrency();
    return n ? static_cast<int>(n) : 1;
}

}  // namespace avas
