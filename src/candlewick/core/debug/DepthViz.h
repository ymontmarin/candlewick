#pragma once

#include "../Core.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct DepthDebugPass {
  enum VizStyle : int { VIZ_GRAYSCALE, VIZ_HEATMAP };
  struct Options {
    VizStyle mode;
    float near;
    float far;
  };
  SDL_GPUTexture *depthTexture;
  SDL_GPUSampler *sampler;
  SDL_GPUGraphicsPipeline *pipeline;

  static DepthDebugPass create(const Renderer &renderer,
                               SDL_GPUTexture *depthTexture);

  void release(SDL_GPUDevice *device);
};

inline void DepthDebugPass::release(SDL_GPUDevice *device) {
  if (sampler) {
    SDL_ReleaseGPUSampler(device, sampler);
    sampler = NULL;
  }
  if (pipeline) {
    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    pipeline = NULL;
  }
}

void renderDepthDebug(Renderer &renderer, const DepthDebugPass &pass,
                      const DepthDebugPass::Options &opts);

} // namespace candlewick
