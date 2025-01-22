#include "Shader.h"
#include "Device.h"
#include "errors.h"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_filesystem.h>

#include <memory>
#include <format>
#define JSON_NO_IO
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>

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

struct ShaderCode {
  std::unique_ptr<Uint8[], decltype(&SDL_free)> data;
  size_t size;
};

ShaderCode loadShaderFile(const char *filename, const char *shader_ext) {
  char shader_path[256];
  SDL_snprintf(shader_path, sizeof(shader_path), "%s/%s.%s",
               CANDLEWICK_SHADER_BIN_DIR, filename, shader_ext);

  size_t code_size;
  void *code = SDL_LoadFile(shader_path, &code_size);
  if (!code) {
    throw RAIIException(
        std::format("Failed to load shader file: %s", SDL_GetError()));
  }
  return ShaderCode{.data{reinterpret_cast<Uint8 *>(code), SDL_free},
                    .size = code_size};
}

Shader::Shader(const Device &device, const char *filename, const Config &config)
    : _shader(nullptr), _device(device) {
  SDL_GPUShaderStage stage = detect_shader_stage(filename);

  SDL_GPUShaderFormat supported_formats = device.shaderFormats();
  SDL_GPUShaderFormat target_format = SDL_GPU_SHADERFORMAT_INVALID;
  const char *shader_ext;
  const char *entry_point;

  if (supported_formats & SDL_GPU_SHADERFORMAT_SPIRV) {
    target_format = SDL_GPU_SHADERFORMAT_SPIRV;
    shader_ext = "spv";
    entry_point = "main";
  } else if (supported_formats & SDL_GPU_SHADERFORMAT_MSL) {
    target_format = SDL_GPU_SHADERFORMAT_MSL;
    shader_ext = "msl";
    entry_point = "main0";
  } else {
    throw RAIIException(
        "Failed to load shader: no available supported shader format.");
  }

  SDL_Log("Loading shader file %s/%s", filename, shader_ext);
  ShaderCode shader_code = loadShaderFile(filename, shader_ext);

  SDL_GPUShaderCreateInfo info{
      .code_size = shader_code.size,
      .code = shader_code.data.get(),
      .entrypoint = entry_point,
      .format = target_format,
      .stage = stage,
      .num_samplers = config.samplers,
      .num_storage_textures = config.storage_textures,
      .num_storage_buffers = config.storage_buffers,
      .num_uniform_buffers = config.uniform_buffers,
      .props = 0U,
  };
  if (!(_shader = SDL_CreateGPUShader(device, &info))) {
    throw RAIIException(
        std::format("Failed to load shader: %s", SDL_GetError()));
  }
}

void Shader::release() noexcept {
  if (_device && _shader) {
    SDL_ReleaseGPUShader(_device, _shader);
    _shader = nullptr;
  }
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Shader::Config, uniform_buffers, samplers,
                                   storage_textures, storage_buffers);

Shader::Config loadShaderMetadata(const char *filename) {
  char metadata_path[256];
  SDL_snprintf(metadata_path, sizeof(metadata_path), "%s/%s.json",
               CANDLEWICK_SHADER_BIN_DIR, filename);

  auto data = loadShaderFile(filename, "json");
  auto meta =
      nlohmann::json::parse(data.data.get(), data.data.get() + data.size);
  return meta.get<Shader::Config>();
}

} // namespace candlewick
