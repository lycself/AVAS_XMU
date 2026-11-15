#include "core/device.h"

#include "core/errors.h"
#include "core/format.h"
#include "core/options.h"

namespace avas {

namespace {

DevicePlan g_used;

DevicePlan cpu_plan(const std::string& reason, const std::string& code) {
    DevicePlan p;
    p.reason = reason;
    p.code = code;
    return p;
}

DevicePlan failed_plan(const std::string& error) {
    DevicePlan p;
    p.error = error;
    return p;
}

}  // namespace

DevicePlan plan_device(long long nParticles, bool spaceCharge, long long meshCells) {
    const Options& o = options();
    if (o.device == Device::Cpu) return cpu_plan("requested", "requested");
    if (o.device == Device::Emu) {
        if (compat_dll()) return failed_plan("compat=dll is only available on the CPU (device emu was requested)");
        DevicePlan p;
        p.device = "emu";
        p.precision = o.precision == Precision::Fp32 ? "fp32" : "fp64";
        p.reason = "requested";
        p.code = "requested";
        return p;
    }
    if (o.device == Device::Metal) {
        if (compat_dll()) return failed_plan("compat=dll is only available on the CPU (device metal was requested)");
        std::string name, why;
        if (!metal_probe(name, why)) return failed_plan("device metal: " + why);
        DevicePlan p;
        p.device = "metal";
        p.precision = "fp32";
        p.reason = "requested";
        p.code = "requested";
        p.desc = name;
        return p;
    }
    const bool required = o.device == Device::Cuda;
    if (compat_dll()) {
        if (required) return failed_plan("compat=dll is only available on the CPU (device cuda was requested)");
        return cpu_plan("auto: compat=dll runs on the CPU only", "compat_dll");
    }
    if (!cuda_compiled() && !required && metal_compiled()) {
        // auto on macOS (M6c decision 6, M8-plan 12): Metal when the run has space charge and at least
        // kMetalParticlesPerCell particles per mesh cell, else the CPU
        if (!spaceCharge) return cpu_plan("auto: no space charge, Metal is used for runs with space charge", "no_space_charge");
        if (nParticles > 0 && nParticles < kMetalMinParticles)
            return cpu_plan("auto: " + std::to_string(nParticles) + " particles (fewer than " + std::to_string(kMetalMinParticles) +
                                "): the CPU is faster than Metal",
                            "few_particles");
        if (nParticles > 0 && meshCells > 0 &&
            static_cast<double>(nParticles) < kMetalParticlesPerCell * static_cast<double>(meshCells))
            return cpu_plan("auto: " + std::to_string(nParticles) + " particles for " + std::to_string(meshCells) +
                                " space-charge mesh cells (fewer than " + fmt_g(kMetalParticlesPerCell) +
                                " per cell): the CPU is faster than Metal",
                            "few_particles");
        std::string name, why;
        if (!metal_probe(name, why)) return cpu_plan("auto: " + why, "no_metal");
        DevicePlan p;
        p.device = "metal";
        p.precision = "fp32";
        p.reason = meshCells > 0 ? "auto" : "auto: Metal for runs with space charge, at least " + std::to_string(kMetalMinParticles) +
                                                " particles and at least " + fmt_g(kMetalParticlesPerCell) +
                                                " particles per mesh cell, else the CPU";
        p.code = "auto_space_charge";
        p.desc = name;
        p.cpuBelowParticlesPerCell = kMetalParticlesPerCell;
        p.cpuBelowParticles = kMetalMinParticles;
        return p;
    }
    if (!cuda_compiled()) {
        if (required) return failed_plan("device cuda: this engine was built without CUDA (AVAS_WITH_CUDA=OFF)");
        return cpu_plan("auto: this engine was built without CUDA", "no_cuda_build");
    }
    DevicePlan p;
    std::string why;
    if (!cuda_plan(nParticles, spaceCharge, p, why)) {
        if (required) return failed_plan("device cuda: " + why);
        return cpu_plan("auto: " + why, p.code.empty() ? "no_gpu" : p.code);
    }
    return p;
}

const DevicePlan& used_device() { return g_used; }

void reset_used_device() {
    g_used = DevicePlan();
    g_used.device.clear();
    g_used.precision.clear();
}

std::unique_ptr<DeviceBackend> make_device_backend(long long nParticles, bool spaceCharge, long long meshCells) {
    const Options& o = options();
    DevicePlan plan = plan_device(nParticles, spaceCharge, meshCells);
    if (!plan.error.empty()) throw EngineError(plan.error);
    plan.spaceCharge = spaceCharge;
    auto fallback = [&](const std::string& why) {
        say("Device: CPU (auto: " + why + ")");
        g_used = cpu_plan("auto: " + plan.device + " could not be used at run time: " + why, "run_time_failure");
        g_used.spaceCharge = spaceCharge;
    };
    if (plan.device == "cpu") {
        if (o.device != Device::Cpu) say("Device: CPU (" + plan.reason + ")");
        g_used = plan;
        return nullptr;
    }
    if (plan.device == "emu") {
        std::unique_ptr<ScAccel> acc;
        if (plan.precision == "fp32") {
            // the fp32 reference of the Metal back end (M6c): its kernels on the CPU
            std::string why;
            acc = make_f32_cpu_accel(why);
            if (!acc) throw EngineError("device emu, precision fp32: " + why);
        }
        std::unique_ptr<DeviceBackend> be = make_emu_backend(std::move(acc));
        say("Device: " + be->describe());
        g_used = plan;
        return be;
    }
    if (plan.device == "metal") {
        std::string why;
        std::unique_ptr<ScAccel> acc = make_metal_accel(why);
        if (!acc) {
            if (o.device == Device::Metal) throw EngineError("device metal: " + why);
            fallback(why);
            return nullptr;
        }
        std::unique_ptr<DeviceBackend> be = make_emu_backend(std::move(acc));
        say("Device: " + be->describe() + (o.device == Device::Metal ? "" : " (auto)"));
        g_used = plan;
        return be;
    }
    std::string why;
    std::unique_ptr<DeviceBackend> be = make_cuda_backend(plan, why);
    if (!be) {
        if (o.device == Device::Cuda) throw EngineError("device cuda: " + why);
        fallback(why);
        return nullptr;
    }
    say("Device: " + be->describe());
    g_used = plan;
    return be;
}

}  // namespace avas
