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
        std::unique_ptr<DeviceBackend> be = make_emu_backend();
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
