// Run-time options of the engine (C API avas_set_option / environment variables).
#pragma once
#include <string>

namespace avas {

enum class Compat { None, Dll };

struct Options {
    Compat compat = Compat::None;  // None: defects corrected; Dll: reproduce AVAS.dll
    int threads = 0;               // 0 = std::thread::hardware_concurrency()
};

Options& options();
inline bool compat_dll() { return options().compat == Compat::Dll; }

// Returns 0 on success, -1 for an unknown key, -2 for an invalid value.
int set_option(const std::string& key, const std::string& value);
// Reads AVAS_ENGINE_COMPAT / AVAS_ENGINE_THREADS once (called by the C API entry points).
void init_options_from_env();

// Value OpenMP would report as omp_get_max_threads() (OMP_NUM_THREADS or logical cores).
int omp_max_threads();

}  // namespace avas
