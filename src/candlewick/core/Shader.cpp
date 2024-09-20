#include "Shader.h"
#include "Device.h"
#include <SDL3/SDL_filesystem.h>
// #include "SDL_gpu_shadercross.h"

namespace candlewick {

Shader::Shader(Device &device, const char *filename) : device(device) {
  const char *currentPath = SDL_GetBasePath();
  SDL_GPUShaderStage stage = detect_shader_stage(filename);
  char path[256];
  SDL_snprintf(path, sizeof(path), "%s../../../assets/%s.spv", currentPath,
               filename);
  SDL_Log("Loading shader %s", path);

  size_t code_size;
  void *code = SDL_LoadFile(path, &code_size);
  if (!code) {
    SDL_Log("Failed to load file: %s", SDL_GetError());
  }
  SDL_GPUShaderCreateInfo info{.code_size = code_size,
                               .code = reinterpret_cast<Uint8 *>(code),
                               .entrypoint = "main",
                               .format = SDL_GPU_SHADERFORMAT_SPIRV,
                               .stage = stage,
                               .num_samplers = 0,
                               .num_storage_textures = 0,
                               .num_storage_buffers = 0,
                               .num_uniform_buffers = 0,
                               .props = 0U};
  _shader =
      // (SDL_GPUShader *)SDL_ShaderCross_CompileFromSPIRV(device, &info,
      // false);
      SDL_CreateGPUShader(device, &info);
  if (!_shader) {
    SDL_Log("Failed to create shader, %s", SDL_GetError());
  }
  SDL_free(code);
}

void Shader::release() {
  if (device)
    SDL_ReleaseGPUShader(device, _shader);
}

} // namespace candlewick
