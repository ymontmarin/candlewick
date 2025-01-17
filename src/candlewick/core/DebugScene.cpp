#include "DebugScene.h"
#include "Shader.h"

#include "../primitives/Arrow.h"
#include "../primitives/Grid.h"

namespace candlewick {

BasicDebugModule::BasicDebugModule(const Device &dev)
    : DebugModule(), triad(NoInit), grid(NoInit) {
  auto triad_datas = createTriad();
  std::vector<MeshView> _views;
  std::tie(triad, _views) = createMeshFromBatch(dev, triad_datas, true);
  for (size_t i = 0; i < 3; i++) {
    triad_views[i] = std::move(_views[i]);
    triad_colors[i] = triad_datas[i].material.baseColor;
  }

  auto grid_data = loadGrid(20);
  grid = createMesh(dev, grid_data);
  uploadMeshToDevice(dev, grid.toView(), grid_data);
  grid_color = grid_data.material.baseColor;
}

void BasicDebugModule::addDrawCommands(DebugScene &scene,
                                       const Camera &camera) {
  if (enableTriad) {
    scene.drawCommands.push_back({
        .pipeline_type = DebugPipelines::TRIANGLE_FILL,
        .mesh = &triad,
        .mesh_views = {triad_views.begin(), triad_views.end()},
        .mvp = camera.viewProj(),
        .colors = {triad_colors.begin(), triad_colors.end()},
    });
  }
  if (enableGrid) {
    scene.drawCommands.push_back({
        .pipeline_type = DebugPipelines::LINE,
        .mesh = &grid,
        .mesh_views{grid.toView()},
        .mvp = camera.viewProj(),
        .colors = {grid_color},
    });
  }
}

DebugScene::DebugScene(const Renderer &renderer)
    : _device(renderer.device), trianglePipeline(nullptr),
      linePipeline(nullptr) {
  Shader vertexShader{_device, "Hud3dElement.vert", 1};
  Shader fragmentShader{renderer.device, "Hud3dElement.frag", 1};
  SDL_GPUColorTargetDescription color_desc{
      .format = SDL_GetGPUSwapchainTextureFormat(_device, renderer.window),
      .blend_state = {
          .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
          .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
          .color_blend_op = SDL_GPU_BLENDOP_ADD,
          .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
          .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
          .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
          .enable_blend = true,
      }};
  SDL_GPUGraphicsPipelineCreateInfo info{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .rasterizer_state{.fill_mode = SDL_GPU_FILLMODE_FILL,
                        .cull_mode = SDL_GPU_CULLMODE_NONE},
      .target_info{.color_target_descriptions = &color_desc,
                   .num_color_targets = 1,
                   .depth_stencil_format = renderer.depth_format,
                   .has_depth_stencil_target = true},
  };
  trianglePipeline = SDL_CreateGPUGraphicsPipeline(_device, &info);

  // re-use
  info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
  linePipeline = SDL_CreateGPUGraphicsPipeline(_device, &info);
}
} // namespace candlewick
