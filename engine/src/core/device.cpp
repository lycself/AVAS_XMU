#include "core/device.h"

#include "core/errors.h"
#include "core/format.h"
#include "core/options.h"

namespace avas {

std::unique_ptr<DeviceBackend> make_device_backend(long long nParticles, bool spaceCharge) {
    const Options& o = options();
    if (o.device == Device::Cpu) return nullptr;
    if (o.device == Device::Emu) {
        if (compat_dll()) throw EngineError("compat=dll is only available on the CPU (device emu was requested)");
        std::unique_ptr<ScAccel> acc;
        if (o.precision == Precision::Fp32) {
            // the fp32 reference of the Metal back end (M6c): its kernels on the CPU
            std::string why;
            acc = make_f32_cpu_accel(why);
            if (!acc) throw EngineError("device emu, precision fp32: " + why);
        }
        std::unique_ptr<DeviceBackend> be = make_emu_backend(std::move(acc));
        say("Device: " + be->describe());
        return be;
    }
    if (o.device == Device::Metal) {
        if (compat_dll()) throw EngineError("compat=dll is only available on the CPU (device metal was requested)");
        std::string why;
        std::unique_ptr<ScAccel> acc = make_metal_accel(why);
        if (!acc) throw EngineError("device metal: " + why);
        std::unique_ptr<DeviceBackend> be = make_emu_backend(std::move(acc));
        say("Device: " + be->describe());
        return be;
    }
    const bool required = o.device == Device::Cuda;
    if (compat_dll()) {
        if (required) throw EngineError("compat=dll is only available on the CPU (device cuda was requested)");
        say("Device: CPU (auto: compat=dll runs on the CPU only)");
        return nullptr;
    }
    if (!cuda_compiled() && !required && metal_compiled()) {
        // auto on macOS (M6c decision 6): Metal when the run has space charge, else the CPU
        if (!spaceCharge) {
            say("Device: CPU (auto: no space charge, Metal is used for runs with space charge)");
            return nullptr;
        }
        std::string why;
        std::unique_ptr<ScAccel> acc = make_metal_accel(why);
        if (!acc) {
            say("Device: CPU (auto: " + why + ")");
            return nullptr;
        }
        std::unique_ptr<DeviceBackend> be = make_emu_backend(std::move(acc));
        say("Device: " + be->describe() + " (auto)");
        return be;
    }
    if (!cuda_compiled()) {
        if (required) throw EngineError("device cuda: this engine was built without CUDA (AVAS_WITH_CUDA=OFF)");
        say("Device: CPU (auto: this engine was built without CUDA)");
        return nullptr;
    }
    std::string why;
    std::unique_ptr<DeviceBackend> be = make_cuda_backend(nParticles, spaceCharge, required, why);
    if (!be) {
        if (required) throw EngineError("device cuda: " + why);
        say("Device: CPU (auto: " + why + ")");
        return nullptr;
    }
    say("Device: " + be->describe());
    return be;
}

}  // namespace avas
