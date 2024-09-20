#pragma once

#include <SDL3/SDL_gpu.h>

namespace candlewick {

SDL_GPUShaderStage detect_shader_stage(const char *filename);

struct Shader {
  Shader(SDL_GPUDevice *device, const char *filename);
  Shader(const Shader &) = delete;
  operator SDL_GPUShader *() { return _shader; }
  void release();

private:
  SDL_GPUShader *_shader;
  SDL_GPUDevice *_device;
};

} // namespace candlewick
