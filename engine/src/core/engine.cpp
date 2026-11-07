#include "core/engine.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#include "core/beam.h"
#include "core/errors.h"
#include "core/files.h"
#include "core/format.h"
#include "core/lattice.h"
#include "core/options.h"
#include "core/params.h"
#include "core/synchronous.h"
#include "core/tracker.h"

namespace avas {

namespace {

std::atomic<bool> g_stop{false};
std::atomic<bool> g_running{false};

// errorOut(msg, stage): message to stdout and appended to ErrorLog.txt.
void error_out(const std::string& msg, int stage) {
    say(msg);
    std::string line = time_stamp();
    if (stage == 1) line += "   Initialisation stage error: " + msg + "\n";
    else if (stage == 2) line += "   simulation stage error: " + msg + "\n";
    else return;
    write_file(paths().output + "ErrorLog.txt", line, true);
}

void reject_unimplemented(const Params& p) {
    auto no = [](const std::string& what) {
        throw EngineError(what + " is not implemented in the new engine yet");
    };
    if (p.spaceCharge == 1) no("Space charge (input.txt spacecharge 1)");
    if (p.boundary == 1) no("boundary.txt (input.txt boundary 1)");
    if (p.lpBoundary == 1) no("input.txt lpboundary 1");
    if (p.dumpPeriodicity != 0) no("input.txt dumpperiodicity");
    if (p.pcHistogram[0] == 1.0) no("input.txt pchistogram 1");
    if (p.longLimits[0] != 0.0) no("input.txt longlimits");
    if (!p.track.empty()) no("input.txt track");
}

std::string seconds(std::chrono::steady_clock::time_point t0) {
    return fmt_g(std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
}

}  // namespace

[[noreturn]] void compat_exit(int code) {
    std::fflush(stdout);
    std::exit(code);
}

[[noreturn]] void compat_abort(const std::string& what) {
    std::fprintf(stderr, "terminate called after throwing an instance of 'std::invalid_argument'\n  what():  %s\n",
                 what.c_str());
    std::fflush(stdout);
    std::abort();
}

bool stop_requested() { return g_stop.load(); }

void set_paths(const std::string& input, const std::string& output, const std::string& field) {
    say("The input path has been set to " + input);
    say("The fieldMap path has been set to " + field);
    say("The output path has been set to " + output);
    paths().input = input + "/";
    paths().field = field + "/";
    paths().output = output + "/";
}

int run_main_agent(int* state) {
    init_options_from_env();
    const bool compat = compat_dll();
    if (!compat && state && *state == 2) {
        // main_agent(2) is the Python "stop" call; the original ignores the argument and starts a
        // second run (defect A).  Here it only asks a running simulation to stop.
        if (g_running.load()) g_stop = true;
        return 0;
    }
    g_stop = false;
    g_running = true;
    struct Running {
        ~Running() { g_running = false; }
    } runningGuard;

    const auto t0 = std::chrono::steady_clock::now();
    say("The programme is running.");
    int stage = 1;
    try {
        Params p;
        Lattice lat;
        Beam beam;
        read_input(p);
        if (p.singleParticleMode == 1) {
            say("Single particle mode startup ... ...");
            init_lattice(lat, p);
            read_beam(p);
            set_synchronous_phase(lat, p, -33.0, true);
            say("Simulation ends.");
            return 0;
        }
        init_lattice(lat, p);
        init_particles(beam, p, lat);
        reject_unimplemented(p);
        set_synchronous_phase(lat, p, beam.uz[beam.n], false);
        say("Initialization time: " + seconds(t0) + " s");
        if (state && compat) *state = 2;
        stage = 2;
        run_tracking(lat, beam, p);
        say("Simulated total time: " + seconds(t0) + " s");
        return 0;
    } catch (const EngineError& e) {
        error_out(e.what(), stage);
    } catch (const NumberError& e) {
        if (compat) compat_abort(e.what());
        error_out(std::string("Invalid number in the input files (") + e.what() + ")", stage);
    } catch (const std::out_of_range& e) {
        if (compat) compat_abort(e.what());
        error_out(std::string("Missing parameter in the input files (") + e.what() + ")", stage);
    } catch (const std::exception& e) {
        error_out(e.what(), stage);
    }
    return stage;
}

}  // namespace avas
