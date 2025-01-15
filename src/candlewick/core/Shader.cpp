#include "Shader.h"
#include "Device.h"
#include "errors.h"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_filesystem.h>

namespace candlewick {
SDL_GPUShaderStage detect_shader_stage(const char *filename) {
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

const char *shader_format_name(SDL_GPUShaderFormat shader_format) {
  switch (shader_format) {
  case SDL_GPU_SHADERFORMAT_INVALID:
    return "invalid";
  case SDL_GPU_SHADERFORMAT_PRIVATE:
    return "rivate";
  case SDL_GPU_SHADERFORMAT_SPIRV:
    return "spirv";
  case SDL_GPU_SHADERFORMAT_DXBC:
    return "dxbc";
  case SDL_GPU_SHADERFORMAT_DXIL:
    return "dxil";
  case SDL_GPU_SHADERFORMAT_MSL:
    return "msl";
  case SDL_GPU_SHADERFORMAT_METALLIB:
    return "metallib";
  default:
    CDW_UNREACHABLE_ASSERT("Unknown shader format");
    std::terminate();
  }
}

bool filter_shader_format(SDL_GPUShaderFormat flags, SDL_GPUShaderFormat test) {
  return (flags & test) != 0;
}

Shader::Shader(const Device &device, const char *filename,
               Uint32 uniformBufferCount, std::string_view entryPoint)
    : _shader(nullptr), _device(device) {
  const char *currentPath = SDL_GetBasePath();
  SDL_GPUShaderStage stage = detect_shader_stage(filename);
  char shader_path[256];
  const char *shader_ext;
  auto supported_formats = device.shaderFormats();
  SDL_GPUShaderFormat target_format{0};
  if (filter_shader_format(supported_formats, SDL_GPU_SHADERFORMAT_SPIRV)) {
    target_format = SDL_GPU_SHADERFORMAT_SPIRV;
    shader_ext = "spv";
  } else if (filter_shader_format(supported_formats,
                                  SDL_GPU_SHADERFORMAT_MSL)) {
    target_format = SDL_GPU_SHADERFORMAT_MSL;
    shader_ext = "msl";
  } else {
    SDL_Log("Asked for unavailable shader format.");
    SDL_assert(false);
  }
  SDL_snprintf(shader_path, sizeof(shader_path),
               "%s../../../assets/shaders/compiled/%s.%s", currentPath,
               filename, shader_ext);

  size_t code_size;
  void *code = SDL_LoadFile(shader_path, &code_size);
  if (!code) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load file: %s",
                 SDL_GetError());
  }
  SDL_Log("Loading shader %s (format %s)", shader_path,
          shader_format_name(target_format));
  SDL_GPUShaderCreateInfo info{.code_size = code_size,
                               .code = reinterpret_cast<Uint8 *>(code),
                               .entrypoint = entryPoint.data(),
                               .format = target_format,
                               .stage = stage,
                               .num_samplers = 0,
                               .num_storage_textures = 0,
                               .num_storage_buffers = 0,
                               .num_uniform_buffers = uniformBufferCount,
                               .props = 0U};
  _shader = SDL_CreateGPUShader(device, &info);
  if (!_shader) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create shader, %s",
                 SDL_GetError());
  }
  SDL_free(code);
}

void Shader::release() {
  if (_device && _shader) {
    SDL_ReleaseGPUShader(_device, _shader);
    _shader = NULL;
  }
}

} // namespace candlewick
