#include "candlewick/core/Core.h"
#include "candlewick/core/Device.h"

#include <SDL3/SDL_gpu.h>

using candlewick::Device;
using candlewick::NoInit;

const float kScrollZoom = 0.05;

struct Context {
  Device device{NoInit};
  SDL_Window *window;
  SDL_GPUGraphicsPipeline *lineListPipeline;
};

bool ExampleInit(Context &ctx, float wWidth, float wHeight);
void ExampleTeardown(Context &ctx);

void initGridPipeline(Context &ctx, const candlewick::MeshLayout &layout,
                      SDL_GPUTextureFormat depth_stencil_format);
