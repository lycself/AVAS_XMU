/* AVAS simulation engine: C interface.
 *
 * Drop-in replacement for the original kernel (AVAS.dll / libAVAS.so):
 *   path(input, output, field)  sets the three directories (a "/" is appended to each);
 *   main_agent(int*)            runs one simulation; returns 0 = ok, 1 = initialisation error,
 *                               2 = simulation error (the message is appended to ErrorLog.txt).
 * Extension:
 *   avas_set_option(key, value) returns 0 on success.  Keys:
 *     "compat"  "none" (default, defects of the original kernel corrected) or
 *               "dll"  (reproduce the Windows AVAS.dll, including its defects).
 *               The environment variable AVAS_ENGINE_COMPAT sets the initial value.
 *     "threads" number of worker threads (0 = automatic).
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
AVAS_API int avas_set_option(const char* key, const char* value);
AVAS_API const char* avas_engine_version(void);

#ifdef __cplusplus
}
#endif
#endif
