#pragma once

#include "Tags.h"
#include <SDL3/SDL_gpu.h>
#include <initializer_list>

namespace candlewick {

/// \brief Automatically detect which subset of shader formats (MSL, SPIR-V) are
/// compatible with the device.
/// \param name Device name. Pass nullptr (default) to auto-detect the best
/// device.
inline SDL_GPUShaderFormat
auto_detect_shader_format_subset(const char *name = nullptr) {
  SDL_GPUShaderFormat available_formats = SDL_GPU_SHADERFORMAT_INVALID;
  for (SDL_GPUShaderFormat test_format :
       {SDL_GPU_SHADERFORMAT_SPIRV, SDL_GPU_SHADERFORMAT_MSL}) {
    if (SDL_GPUSupportsShaderFormats(test_format, name))
      available_formats |= test_format;
  }
  return available_formats;
}

/// \brief RAII wrapper for \c SDL_GPUDevice.
struct Device {

  explicit Device(NoInitT) noexcept;
  explicit Device(SDL_GPUShaderFormat format_flags, bool debug_mode = false);
  Device(const Device &) = delete;
  Device(Device &&other) noexcept;
  Device &operator=(Device &&other) noexcept;

  operator SDL_GPUDevice *() const { return _device; }
  operator bool() const { return _device; }

  const char *driverName() const { return driver; }
  SDL_GPUShaderFormat shaderFormats() const {
    return SDL_GetGPUShaderFormats(_device);
  }

  void destroy() noexcept;
  ~Device() noexcept { destroy(); }

private:
  SDL_GPUDevice *_device;
  const char *driver;
};

inline Device::Device(NoInitT) noexcept : _device(nullptr), driver(nullptr) {}

inline Device::Device(Device &&other) noexcept {
  _device = other._device;
  driver = other.driver;
  other._device = nullptr;
}

inline Device &Device::operator=(Device &&other) noexcept {
  // destroy if needed. if not needed this is a no-op
  destroy();
  _device = other._device;
  driver = other.driver;
  other._device = nullptr;
  return *this;
}

} // namespace candlewick
