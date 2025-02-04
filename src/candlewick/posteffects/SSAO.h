#pragma once

#include "../core/Core.h"
#include "../core/Texture.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {
namespace ssao {
  struct SsaoPass {
    SDL_GPUTexture *inDepthMap = nullptr;
    SDL_GPUTexture *inNormalMap = nullptr;
    SDL_GPUSampler *texSampler = nullptr;
    SDL_GPUGraphicsPipeline *pipeline = nullptr;
    Texture ssaoMap{NoInit};

    struct SsaoNoise {
      Texture tex{NoInit};
      SDL_GPUSampler *sampler = nullptr;
    } ssaoNoise;

    SsaoPass(NoInitT) {}
    SsaoPass(const Renderer &renderer, const MeshLayout &layout,
             SDL_GPUTexture *normalMap);

    void render(Renderer &renderer, const Camera &camera);

    // cleanup function
    void release();
  };

} // namespace ssao
} // namespace candlewick
