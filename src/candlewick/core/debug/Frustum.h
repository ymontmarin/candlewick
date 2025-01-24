#pragma once

#include "../Core.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct FrustumAndBoundsDebug {
  SDL_GPUGraphicsPipeline *pipeline;

  static FrustumAndBoundsDebug create(const Renderer &renderer);
  void render(Renderer &renderer, const Camera &camera, const Camera &otherCam);
  void release(const Device &device);
};

} // namespace candlewick
