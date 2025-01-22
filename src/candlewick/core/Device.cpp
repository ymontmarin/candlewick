#include "Device.h"
#include "errors.h"

#include <SDL3/SDL_log.h>
#include <format>

namespace candlewick {

Device::Device(SDL_GPUShaderFormat format_flags, bool debug_mode)
    : _device(nullptr) {
  _device = SDL_CreateGPUDevice(format_flags, debug_mode, nullptr);
  if (!_device) {
    throw RAIIException(
        std::format("Failed to create GPU device: %s", SDL_GetError()));
  }
  driver = SDL_GetGPUDeviceDriver(_device);
  SDL_Log("Device driver: %s", driver);
}

void Device::destroy() noexcept {
  if (_device)
    SDL_DestroyGPUDevice(_device);
  _device = nullptr;
}

} // namespace candlewick
