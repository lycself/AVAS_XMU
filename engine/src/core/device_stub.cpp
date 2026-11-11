// CPU-only build (AVAS_WITH_CUDA=OFF): no accelerator back end.
#include "core/device.h"

namespace avas {

bool cuda_compiled() { return false; }

ProcessRank process_rank() { return ProcessRank{}; }

bool cuda_plan(long long, bool, DevicePlan&, std::string& why) {
    why = "this engine was built without CUDA";
    return false;
}

std::unique_ptr<DeviceBackend> make_cuda_backend(const DevicePlan&, std::string& why) {
    why = "this engine was built without CUDA";
    return nullptr;
}

std::vector<CudaDeviceInfo> cuda_devices(std::string& why) {
    why = "this engine was built without CUDA";
    return {};
}

}  // namespace avas
