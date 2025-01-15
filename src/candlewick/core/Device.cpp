#include "Device.h"
#include "errors.h"
#include <SDL3/SDL_log.h>

namespace candlewick {

Device::Device(SDL_GPUShaderFormat format_flags, bool debug_mode)
    : _device(NULL) {
  _device = SDL_CreateGPUDevice(format_flags, debug_mode, NULL);
  if (!_device) {
    SDL_Log("CreateGPUDevice failed");
    throw RAIIException(SDL_GetError());
  }
  driver = SDL_GetGPUDeviceDriver(_device);
  SDL_Log("Device driver: %s", driver);
}

void Device::destroy() {
  if (_device)
    SDL_DestroyGPUDevice(_device);
  _device = NULL;
}

} // namespace candlewick
