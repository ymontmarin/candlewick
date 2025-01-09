#include "Renderer.h"
#include "errors.h"
#include <utility>

namespace candlewick {
Renderer::Renderer(Device &&device, SDL_Window *window)
    : device(std::move(device)), window(window) {
  if (!SDL_ClaimWindowForGPUDevice(this->device, this->window))
    throw RAIIException(SDL_GetError());
}

Renderer::Renderer(Device &&device, SDL_Window *window,
                   SDL_GPUTextureFormat depth_tex_format)
    : Renderer(std::move(device), window) {

  int width, height;
  SDL_GetWindowSize(window, &width, &height);
  SDL_GPUTextureCreateInfo texInfo{
      .type = SDL_GPU_TEXTURETYPE_2D,
      .format = depth_tex_format,
      .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
      .width = Uint32(width),
      .height = Uint32(height),
      .layer_count_or_depth = 1,
      .num_levels = 1,
      .sample_count = SDL_GPU_SAMPLECOUNT_1,
  };
  depth_texture = SDL_CreateGPUTexture(this->device, &texInfo);
}

} // namespace candlewick
