#pragma once

#include "../core/Core.h"
#include "../core/Tags.h"
#include "../core/LightUniforms.h"

#include <SDL3/SDL_gpu.h>

namespace candlewick {
namespace effects {

  struct ScreenSpaceShadowPass {
    struct Config {
      float maxDist;
      float stepSize;
      int numSteps;
      float shadowBias;
    } config;
    SDL_GPUTexture *depthTexture = nullptr;
    /// Sampler for the depth texture (e.g. from the prepass)
    SDL_GPUSampler *depthSampler = nullptr;
    /// Target texture to render the shadow to.
    SDL_GPUTexture *targetTexture = nullptr;
    /// Render pipeline
    SDL_GPUGraphicsPipeline *pipeline = nullptr;

    ScreenSpaceShadowPass(NoInitT) {}
    ScreenSpaceShadowPass(const Renderer &renderer, const MeshLayout &layout,
                          const Config &config);

    void release(SDL_GPUDevice *device) noexcept;

    void render(Renderer &renderer, const Camera &camera,
                const DirectionalLight &light);
  };

} // namespace effects
} // namespace candlewick
