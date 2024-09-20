#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>

namespace candlewick {

inline SDL_GPUShaderStage detect_shader_stage(const char *filename) {
  SDL_GPUShaderStage stage;
  if (SDL_strstr(filename, ".vert"))
    stage = SDL_GPU_SHADERSTAGE_VERTEX;
  else if (SDL_strstr(filename, ".frag"))
    stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  else {
    SDL_Log("Failed to detect Shader stage.");
    stage = SDL_GPUShaderStage(-1);
  }
  return stage;
}

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
