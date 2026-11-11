/* AVAS simulation engine: C interface.
 *
 * Drop-in replacement for the original kernel (AVAS.dll / libAVAS.so):
 *   path(input, output, field)  sets the three directories (a "/" is appended to each);
 *   main_agent(int*)            runs one simulation; returns 0 = ok, 1 = initialisation error,
 *                               2 = simulation error (the message is appended to ErrorLog.txt).
 *
 * Extension: avas_set_option(key, value) returns 0 on success, -1 for an unknown key, -2 for an
 * invalid value.  The environment variable of each key sets its initial value when the library is
 * first used (path(), main_agent(), the tools, avas_set_option, avas_engine_devices); a value set with
 * avas_set_option afterwards takes precedence over the environment.
 *   key               environment variable             values
 *   "compat"          AVAS_ENGINE_COMPAT               "none" (default: defects of the original kernel
 *                                                      corrected) or "dll" (reproduce the Windows AVAS.dll,
 *                                                      including its defects; CPU only)
 *   "threads"         AVAS_ENGINE_THREADS              worker threads of the CPU parts, 0 = logical cores
 *                                                      (results of the default mode do not depend on it)
 *   "device"          AVAS_ENGINE_DEVICE               "cpu" (default), "cuda" (alias "gpu"; required: the
 *                                                      run fails when no usable NVIDIA GPU), "metal" (macOS,
 *                                                      required), "auto" (CUDA when usable, on macOS Metal
 *                                                      for runs with space charge, else the CPU), "emu"
 *                                                      (development: the device back end on the CPU)
 *   "precision"       AVAS_ENGINE_PRECISION            "auto" (fp64 on full-rate fp64 GPUs, else fp32),
 *                                                      "fp64" or "fp32" (CPU: always fp64; Metal: fp32)
 *   "gpus"            AVAS_ENGINE_GPUS                 "" (first GPU), "all" or a list "0,1"
 *   "beamset_warn_gb" AVAS_ENGINE_BEAMSET_WARN_GB      warn when BeamSet.plt is estimated above this (50)
 *   "gpu_atomic"      AVAS_ENGINE_GPU_ATOMIC           measurement only: "1" = atomic GPU sums (not
 *                                                      reproducible)
 *
 * Queries (JSON text; the pointer refers to a buffer of the calling thread that stays valid until the
 * same thread calls the same function again; the caller does not free it.  Do not call them while
 * main_agent runs in another thread):
 *   avas_engine_version()   the engine source hash, 16 hex digits (the build stamp of the libraries)
 *   avas_engine_info()      {"version", "source_hash" (64 hex digits), "build": {"cuda", "metal", "mpi",
 *                           "cuda_archs", "build_type"}, "compiler", "cmake_compiler", "cuda_compiler",
 *                           "platform"}
 *   avas_engine_devices()   {"cuda": {"compiled", "devices": [{"index", "name", "compute", "fp64_full",
 *                           "memory_gb"}], "note"}, "metal": {"compiled", "devices": [{"name", "usable"}],
 *                           "note"}, "cpu": {"threads"}, "options": {the current option values},
 *                           "auto": {"sc": plan, "nosc": plan}, "selected": {"sc": plan, "nosc": plan}}
 *                           where a plan is {"device", "precision", "gpus", "reason", "error"}: what a run
 *                           with / without space charge gets with device "auto" ("auto") or with the
 *                           current options ("selected"); "error" is not null when such a run would fail.
 *                           The decision is the code main_agent uses (no memory estimate here).  Missing
 *                           drivers give empty lists and a "note"; nothing is printed.
 *   avas_engine_last_run()  {"ran", "device", "precision", "gpus", "compat", "threads", "space_charge",
 *                           "seconds", "status"} of the last main_agent(1) call: what it actually used
 *                           (device null when no particles were tracked); all null with "ran": false
 *                           before the first run.
 */
#ifndef AVAS_ENGINE_H
#define AVAS_ENGINE_H

#if defined(AVAS_ENGINE_STATIC)
#define AVAS_API
#elif defined(_WIN32)
#if defined(AVAS_ENGINE_BUILD)
#define AVAS_API __declspec(dllexport)
#else
#define AVAS_API __declspec(dllimport)
#endif
#else
#define AVAS_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif

#if defined(_WIN32)
AVAS_API int path(const wchar_t* input, const wchar_t* output, const wchar_t* field);
#else
AVAS_API int path(const char* input, const char* output, const char* field);
#endif
AVAS_API int main_agent(int* state);
/* AVAS.dll-only tools (no simulation): weighted compression of .dst distributions and the
 * preprocessing of paired results; same arguments as the original (spec 80). */
#if defined(_WIN32)
AVAS_API int sampling(const wchar_t* in, const wchar_t* out, int total, int segFrom, int segTo, int magFrom, int magTo,
                      int nodes, double halfWidth, double terminalCoefficient);
AVAS_API int meshing(const wchar_t* in, const wchar_t* out, int nodes, double halfWidth);
AVAS_API int dataprocess(const wchar_t* root, int i0, int i1, double nodesFiveCube, double nodesOriginal, double halfWidth,
                         int useWeights);
#else
AVAS_API int sampling(const char* in, const char* out, int total, int segFrom, int segTo, int magFrom, int magTo,
                      int nodes, double halfWidth, double terminalCoefficient);
AVAS_API int meshing(const char* in, const char* out, int nodes, double halfWidth);
AVAS_API int dataprocess(const char* root, int i0, int i1, double nodesFiveCube, double nodesOriginal, double halfWidth,
                         int useWeights);
#endif
AVAS_API int avas_set_option(const char* key, const char* value);
AVAS_API const char* avas_engine_version(void);
AVAS_API const char* avas_engine_info(void);
AVAS_API const char* avas_engine_devices(void);
AVAS_API const char* avas_engine_last_run(void);

#ifdef __cplusplus
}
#endif
#endif
