#pragma once

#include "Tags.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct Device {

  explicit Device(NoInitT);
  explicit Device(SDL_GPUShaderFormat format_flags, bool debug_mode = false);
  Device(const Device &) = delete;
  Device(Device &&other) {
    _device = other._device;
    driver = other.driver;
    other._device = nullptr;
  }

  operator SDL_GPUDevice *() const { return _device; }
  operator bool() const { return _device; }

  const char *driverName() const { return driver; }
  SDL_GPUShaderFormat shaderFormats() const {
    return SDL_GetGPUShaderFormats(_device);
  }

  void destroy();
  ~Device() { destroy(); }

private:
  SDL_GPUDevice *_device;
  const char *driver;
};

inline Device::Device(NoInitT) : _device(nullptr), driver(nullptr) {}

} // namespace candlewick
