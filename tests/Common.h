#include "candlewick/core/Core.h"
#include "candlewick/core/Renderer.h"

#include <SDL3/SDL_gpu.h>

using candlewick::Device;
using candlewick::NoInit;

const float kScrollZoom = 0.05f;

struct Context {
  Device device{NoInit};
  SDL_Window *window;
};

bool initExample(Context &ctx, Uint32 wWidth, Uint32 wHeight);
void teardownExample(Context &ctx);

SDL_GPUGraphicsPipeline *
initGridPipeline(const Device &device, SDL_Window *window,
                 const candlewick::MeshLayout &layout,
                 SDL_GPUTextureFormat depth_stencil_format);

inline SDL_GPUGraphicsPipeline *
initGridPipeline(const candlewick::Renderer &renderer,
                 const candlewick::MeshLayout &layout,
                 SDL_GPUTextureFormat depth_stencil_format) {
  return initGridPipeline(renderer.device, renderer.window, layout,
                          depth_stencil_format);
}

SDL_GPUTexture *createDepthTexture(const Device &device, SDL_Window *window,
                                   SDL_GPUTextureFormat depth_tex_format,
                                   SDL_GPUSampleCount sample_count);
