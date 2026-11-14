// Engine entry points behind the C API.
#pragma once
#include <string>
#include <vector>

namespace avas {

// path(): stores the three directory prefixes (a "/" is appended) and prints them.
void set_paths(const std::string& input, const std::string& output, const std::string& field);
// main_agent(): one complete run; returns 0, 1 (initialisation error) or 2 (simulation error).
int run_main_agent(int* state);
// True once main_agent(2) asked a running simulation to stop (corrected mode only).
bool stop_requested();

// What the last main_agent(1) call actually used (C API avas_engine_last_run()).
struct LastRun {
    bool ran = false;          // false: no main_agent(1) yet
    std::string device;        // "cpu", "cuda", "metal", "emu"; empty when no particles were tracked
    std::string precision;     // "fp64", "fp32"; empty like device
    std::vector<int> gpus;     // CUDA devices
    std::string compat;        // "none" or "dll"
    int threads = 0;           // worker threads of the CPU parts (option threads, 0 = logical cores)
    int spaceCharge = -1;      // 1 / 0 for the tracked particles; -1 when none were tracked
    std::string reason;        // why this device (DevicePlan::reason / code); empty like device
    std::string reasonCode;
    double seconds = 0.0;      // wall time of main_agent
    int status = 0;            // return value of main_agent (0 ok, 1 / 2 errors)
};
const LastRun& last_run();

}  // namespace avas
