#include "candlewick/core/MeshLayout.h"
#include "candlewick/core/Shader.h"

#include "Common.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <new>

bool ExampleInit(Context &ctx, float wWidth, float wHeight) {
  if (!SDL_Init(SDL_INIT_VIDEO))
    return false;
  new (&ctx.device) Device{SDL_GPU_SHADERFORMAT_SPIRV, true};

  ctx.window = SDL_CreateWindow("candlewick: examples", wWidth, wHeight, 0);
  if (!SDL_ClaimWindowForGPUDevice(ctx.device, ctx.window)) {
    SDL_Log("Error %s", SDL_GetError());
    return false;
  }

  return true;
}

void ExampleTeardown(Context &ctx) {
  SDL_ReleaseWindowFromGPUDevice(ctx.device, ctx.window);
  SDL_DestroyWindow(ctx.window);
  if (ctx.lineListPipeline) {
    SDL_ReleaseGPUGraphicsPipeline(ctx.device, ctx.lineListPipeline);
  }
  ctx.device.destroy();
  SDL_Quit();
}

void initGridPipeline(Context &ctx, const candlewick::MeshLayout &layout,
                      SDL_GPUTextureFormat depth_stencil_format) {
  using namespace candlewick;
  Shader vertexShader{ctx.device, "Hud3dElement.vert", 1};
  Shader fragmentShader{ctx.device, "Hud3dElement.frag", 0};

  SDL_GPUColorTargetDescription colorTarget{
      .format = SDL_GetGPUSwapchainTextureFormat(ctx.device, ctx.window)};
  SDL_zero(colorTarget.blend_state);
  SDL_GPUGraphicsPipelineCreateInfo createInfo{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .vertex_input_state = layout.toVertexInputState(),
      .primitive_type = layout.primitiveType(),
      .rasterizer_state{.fill_mode = SDL_GPU_FILLMODE_FILL,
                        .cull_mode = SDL_GPU_CULLMODE_NONE,
                        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE},
      .depth_stencil_state{.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
                           .enable_depth_test = true,
                           .enable_depth_write = true},
      .target_info{.color_target_descriptions = &colorTarget,
                   .num_color_targets = 1,
                   .depth_stencil_format = depth_stencil_format,
                   .has_depth_stencil_target = true},
      .props = 0,
  };
  ctx.lineListPipeline = SDL_CreateGPUGraphicsPipeline(ctx.device, &createInfo);
  vertexShader.release();
  fragmentShader.release();
}
