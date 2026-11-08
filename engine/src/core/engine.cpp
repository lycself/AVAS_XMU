#include "core/engine.h"

#include <atomic>
#include <csignal>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <stdexcept>

#include "core/beam.h"
#include "core/device.h"
#include "core/errors.h"
#include "core/errorset.h"
#include "core/files.h"
#include "core/format.h"
#include "core/lattice.h"
#include "core/options.h"
#include "core/params.h"
#include "core/synchronous.h"
#include "core/tracker.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

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


std::string seconds(std::chrono::steady_clock::time_point t0) {
    return fmt_g(std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
}

}  // namespace

[[noreturn]] void compat_exit(int code) {
    std::fflush(stdout);
    std::exit(code);
}

[[noreturn]] void compat_abort(const std::string& what) {
    std::fflush(stdout);
#if defined(_WIN32)
    (void)what;
    RaiseException(0xE06D7363u, EXCEPTION_NONCONTINUABLE, 0, nullptr);
#else
    std::fprintf(stderr, "terminate called after throwing an instance of 'std::invalid_argument'\n  what():  %s\n",
                 what.c_str());
#endif
    std::abort();
}

[[noreturn]] void compat_crash(const std::string& what) {
    std::fflush(stdout);
#if defined(_WIN32)
    (void)what;
    const ULONG_PTR info[2] = {0, 0};  // read access at address 0
    RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 2, info);
#else
    std::fprintf(stderr, "Segmentation fault (%s)\n", what.c_str());
    std::signal(SIGSEGV, SIG_DFL);
    std::raise(SIGSEGV);
#endif
    std::abort();
}

bool stop_requested() { return g_stop.load(); }

// The C runtime rand() state of AVAS.dll belongs to the process (thread): error samples of a
// later main_agent call continue the sequence.
MsvcRand& crt_rand() {
    static MsvcRand r;
    return r;
}

// main_agent's error loop (spec 10 2.1.2, spec 70): a*b samples, each in output_<k>/.
void run_error_samples(Lattice& lat, Params& p, int* state, int& stage,
                       std::chrono::steady_clock::time_point t0) {
    const bool compat = compat_dll();
    std::uint64_t seed = 0;
    if (!compat) {
        // corrected: samples from the run's random seed (beam.txt overrides input.txt)
        Params pb = p;
        read_beam(pb);
        std::int64_t sd = static_cast<std::int64_t>(pb.randomSeed);
        if (sd == 0) sd = static_cast<std::int64_t>(std::time(nullptr));
        say("Error sample seed: " + std::to_string(sd));
        write_file(paths().output + "errorData.seed.txt", "randomseed " + std::to_string(sd) + "\n", false);
        seed = static_cast<std::uint64_t>(sd);
        p.fixedSeed = sd;  // the same seed for the errors and the initial bunch of every sample
    }
    const std::vector<ErrorSample> samples = generate_error_set(lat, crt_rand(), seed);
    const std::string base = paths().output;
    const Lattice nominal = compat ? Lattice() : lat;
    const Params p0 = p;
    for (size_t k = 0; k < samples.size(); ++k) {
        const std::string dir = base + "output_" + std::to_string(k);
        if (!file_exists(dir) && !make_dir(dir)) {
            say("Error output folder creation failure");
            compat_exit(1);
        }
        paths().output = dir + "/";
        // AVAS.dll shares one lattice and one Initial between the samples (errors accumulate);
        // the corrected engine starts every sample from the nominal lattice
        Lattice fresh;
        Params pk;
        if (!compat) {
            fresh = nominal;
            pk = p0;
        }
        Lattice& L = compat ? lat : fresh;
        Params& P = compat ? p : pk;
        L.sample = &samples[k];
        Beam beam;
        init_particles(beam, P, L);
        set_synchronous_phase(L, P, beam.uz[beam.n], false);
        say("Initialization time: " + seconds(t0) + " s");
        if (state && compat) *state = 2;
        stage = 2;
        run_tracking(L, beam, P);
        say("Simulated total time: " + seconds(t0) + " s");
    }
    paths().output = base;
}

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
    // MPI ranks other than 0 run the same program in a private output folder and stay silent
    const ProcessRank pr = process_rank();
    struct RankFolder {
        std::string saved, dir;
        ~RankFolder() {
            if (dir.empty()) return;
            std::error_code ec;
            std::filesystem::remove_all(std::filesystem::u8path(dir), ec);
            paths().output = saved;
        }
    } rankFolder;
    if (pr.size > 1 && pr.rank > 0) {
        rankFolder.saved = paths().output;
        rankFolder.dir = paths().output + ".mpi_rank_" + std::to_string(pr.rank) + "/";
        make_dir(rankFolder.dir);
        paths().output = rankFolder.dir;
        set_console_quiet(true);
    }
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
        if (lat.err.any()) {
            run_error_samples(lat, p, state, stage, t0);
            return 0;
        }
        init_particles(beam, p, lat);
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
