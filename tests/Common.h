#include "candlewick/core/Core.h"
#include "candlewick/core/Device.h"

#include <SDL3/SDL_gpu.h>

using candlewick::Device;
using candlewick::NoInit;

const float kScrollZoom = 0.05f;

struct Context {
  Device device{NoInit};
  SDL_Window *window;
  SDL_GPUGraphicsPipeline *hudEltPipeline;
};

bool ExampleInit(Context &ctx, Uint32 wWidth, Uint32 wHeight);
void ExampleTeardown(Context &ctx);

void initGridPipeline(Context &ctx, const candlewick::MeshLayout &layout,
                      SDL_GPUTextureFormat depth_stencil_format);

SDL_GPUTexture *createDepthTexture(const Device &device, SDL_Window *window,
                                   SDL_GPUTextureFormat depth_tex_format,
                                   SDL_GPUSampleCount sample_count);
