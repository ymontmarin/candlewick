#include "candlewick/core/MeshLayout.h"
#include "candlewick/core/Shader.h"
#include "candlewick/core/math_types.h"
#include "candlewick/primitives/Cube.h"
#include "candlewick/utils/MeshTransforms.h"

#include "Common.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>

using namespace candlewick;

SDL_GPUGraphicsPipeline *
initGridPipeline(const Device &device, SDL_Window *window,
                 const MeshLayout &layout,
                 SDL_GPUTextureFormat depth_stencil_format,
                 SDL_GPUPrimitiveType primitive_type) {
  auto vertexShader = Shader::fromMetadata(device, "Hud3dElement.vert");
  auto fragmentShader = Shader::fromMetadata(device, "Hud3dElement.frag");

  SDL_GPUColorTargetDescription colorTarget{
      .format = SDL_GetGPUSwapchainTextureFormat(device, window),
      .blend_state = {
          .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
          .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
          .color_blend_op = SDL_GPU_BLENDOP_ADD,
          .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
          .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
          .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
          .enable_blend = true,
      }};
  SDL_GPUGraphicsPipelineCreateInfo createInfo{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .vertex_input_state = layout.toVertexInputState(),
      .primitive_type = primitive_type,
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
  SDL_GPUGraphicsPipeline *hudElemPipeline =
      SDL_CreateGPUGraphicsPipeline(device, &createInfo);
  return hudElemPipeline;
}

MeshData loadCube(float size, const Float2 &loc) {
  MeshData m = toOwningMeshData(loadCubeSolid());
  m.material.metalness = 0.4f;
  m.material.baseColor = 0xaab023ff_rgbaf;
  Eigen::Affine3f tr;
  tr.setIdentity();
  tr.translate(Float3{loc[0], loc[1], 0.f});
  tr.scale(0.5f * size);
  tr.translate(Float3{0, 0, 1.2f});
  apply3DTransformInPlace(m, tr);
  return m;
}
