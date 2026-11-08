// Build without Metal (AVAS_WITH_METAL=OFF or not macOS): no Metal accelerator.
#include "core/device.h"

namespace avas {

bool metal_compiled() { return false; }

std::unique_ptr<ScAccel> make_metal_accel(std::string& why) {
    why = "this engine was built without Metal (AVAS_WITH_METAL=OFF)";
    return nullptr;
}

std::unique_ptr<ScAccel> make_f32_cpu_accel(std::string& why) {
    why = "the fp32 tier of the emu device needs the Metal build for now (AVAS_WITH_METAL=OFF)";
    return nullptr;
}

}  // namespace avas
