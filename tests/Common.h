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

[[nodiscard]] SDL_GPUGraphicsPipeline *
initGridPipeline(const Device &device, SDL_Window *window,
                 const candlewick::MeshLayout &layout,
                 SDL_GPUTextureFormat depth_stencil_format,
                 SDL_GPUPrimitiveType primitive_type);

[[nodiscard]] SDL_GPUTexture *
createDepthTexture(const Device &device, SDL_Window *window,
                   SDL_GPUTextureFormat depth_tex_format,
                   SDL_GPUSampleCount sample_count);
