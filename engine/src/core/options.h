// Run-time options of the engine (C API avas_set_option / environment variables).
#pragma once
#include <string>

namespace avas {

enum class Compat { None, Dll };
enum class Device { Cpu, Cuda, Auto, Emu };  // Emu: test device (device back end on the CPU)
enum class Precision { Auto, Fp64, Fp32 };

struct Options {
    Compat compat = Compat::None;  // None: defects corrected; Dll: reproduce AVAS.dll
    int threads = 0;               // 0 = std::thread::hardware_concurrency()
    Device device = Device::Cpu;   // cpu (default), cuda (required), auto (GPU when usable)
    Precision precision = Precision::Auto;
    std::string gpus;              // "" = first visible GPU, "all", or a list "0,1"
    bool gpuAtomic = false;        // measurement only: non-deterministic atomic sums on the GPU
    double beamSetWarnGB = 50.0;   // corrected engine: warn when BeamSet.plt is estimated above this (GB = 2^30 B)
};

Options& options();
inline bool compat_dll() { return options().compat == Compat::Dll; }

// Returns 0 on success, -1 for an unknown key, -2 for an invalid value.
int set_option(const std::string& key, const std::string& value);
// Reads AVAS_ENGINE_COMPAT / AVAS_ENGINE_THREADS / AVAS_ENGINE_DEVICE / AVAS_ENGINE_PRECISION /
// AVAS_ENGINE_GPUS / AVAS_ENGINE_BEAMSET_WARN_GB once (called by the C API entry points).
void init_options_from_env();

// Value OpenMP would report as omp_get_max_threads() (OMP_NUM_THREADS or logical cores).
int omp_max_threads();

}  // namespace avas
