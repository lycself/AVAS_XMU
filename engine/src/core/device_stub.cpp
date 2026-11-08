// CPU-only build (AVAS_WITH_CUDA=OFF): no accelerator back end.
#include "core/device.h"

namespace avas {

bool cuda_compiled() { return false; }

ProcessRank process_rank() { return ProcessRank{}; }

std::unique_ptr<DeviceBackend> make_cuda_backend(long long, bool, bool, std::string& why) {
    why = "this engine was built without CUDA";
    return nullptr;
}

}  // namespace avas
