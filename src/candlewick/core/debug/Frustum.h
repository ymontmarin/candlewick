#pragma once

#include "../Core.h"
#include "../math_types.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct FrustumAndBoundsDebug {
  const Device &device;
  SDL_GPUGraphicsPipeline *pipeline;

  FrustumAndBoundsDebug(const Renderer &renderer);

  void renderFrustum(Renderer &renderer, const Camera &mainCamera,
                     const Camera &otherCam,
                     const Float4 &color = 0x40FF00CC_rgbaf);

  void renderAABB(Renderer &renderer, const Camera &camera, const AABB &aabb,
                  const Float4 &color = 0x00BFFFff_rgbaf);
  void renderOBB(Renderer &renderer, const Camera &camera, const OBB &obb,
                 const Float4 &color = 0x00BFFFff_rgbaf);

  void release();
  ~FrustumAndBoundsDebug() { release(); }
};

} // namespace candlewick
