// Engine entry points behind the C API.
#pragma once
#include <string>

namespace avas {

// path(): stores the three directory prefixes (a "/" is appended) and prints them.
void set_paths(const std::string& input, const std::string& output, const std::string& field);
// main_agent(): one complete run; returns 0, 1 (initialisation error) or 2 (simulation error).
int run_main_agent(int* state);
// True once main_agent(2) asked a running simulation to stop (corrected mode only).
bool stop_requested();

}  // namespace avas
