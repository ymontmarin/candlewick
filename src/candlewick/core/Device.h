#pragma once

#include "Tags.h"
#include <SDL3/SDL_gpu.h>
#include <initializer_list>

namespace candlewick {

/// \brief Automatically detect which subset of shader formats (MSL, SPIR-V) are
/// compatible with the device.
/// \param name Device name. Pass NULL (default) to auto-detect the best device.
inline SDL_GPUShaderFormat
auto_detect_shader_format_subset(const char *name = NULL) {
  SDL_GPUShaderFormat available_formats = SDL_GPU_SHADERFORMAT_INVALID;
  for (SDL_GPUShaderFormat test_format :
       {SDL_GPU_SHADERFORMAT_SPIRV, SDL_GPU_SHADERFORMAT_MSL}) {
    if (SDL_GPUSupportsShaderFormats(test_format, name))
      available_formats |= test_format;
  }
  return available_formats;
}

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
