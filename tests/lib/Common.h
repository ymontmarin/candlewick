#include "candlewick/core/Core.h"
#include "candlewick/core/Device.h"
#include "candlewick/core/math_types.h"
#include "candlewick/utils/Utils.h"

#include <SDL3/SDL_gpu.h>

using candlewick::Device;
using candlewick::Float2;
using candlewick::MeshData;
using candlewick::NoInit;

const float kScrollZoom = 0.05f;

struct Context {
  Device device{NoInit};
  SDL_Window *window;
};

bool initExample(Context &ctx, Uint32 wWidth, Uint32 wHeight,
                 SDL_WindowFlags windowFlags = 0);
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

MeshData loadCube(float size, const Float2 &loc);
