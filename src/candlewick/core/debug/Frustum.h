#pragma once

#include "../Core.h"
#include "../math_types.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct FrustumAndBoundsDebug {
  SDL_GPUGraphicsPipeline *pipeline;

  static FrustumAndBoundsDebug create(const Renderer &renderer);
  void renderLightFrustum(Renderer &renderer, const Camera &camera,
                          const Camera &otherCam,
                          const Float4 &color = 0x40FF00CC_rgbaf);
  void renderBounds(Renderer &renderer, const Camera &camera, const AABB &aabb,
                    const Float4 &color = 0x00BFFFff_rgbaf);
  void renderOBB(Renderer &renderer, const Camera &camera, const OBB &obb,
                 const Float4 &color = 0x00BFFFff_rgbaf);
  void release(const Device &device);
};

} // namespace candlewick
