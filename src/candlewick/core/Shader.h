#pragma once

#include "Core.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

SDL_GPUShaderStage detect_shader_stage(const char *filename);

const char *shader_format_name(SDL_GPUShaderFormat shader_format);

/// \brief RAII wrapper around \c SDL_GPUShader, with loading utilities.
struct Shader {
  Shader(const Device &device, const char *filename, Uint32 uniformBufferCount,
         Uint32 numSamplers = 0);
  Shader(const Shader &) = delete;
  operator SDL_GPUShader *() noexcept { return _shader; }
  void release();
  ~Shader() { release(); }

private:
  SDL_GPUShader *_shader;
  SDL_GPUDevice *_device;
};

} // namespace candlewick
