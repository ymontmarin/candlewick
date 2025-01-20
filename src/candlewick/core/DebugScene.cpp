#include "DebugScene.h"
#include "Shader.h"

#include "../primitives/Arrow.h"
#include "../primitives/Grid.h"

namespace candlewick {

BasicDebugModule::BasicDebugModule(DebugScene &scene)
    : DebugModule(scene), triad(NoInit), grid(NoInit) {
  const auto &dev = scene.device();
  auto triad_datas = createTriad();
  std::tie(triad, triad_views) = createMeshFromBatch(dev, triad_datas, true);
  for (size_t i = 0; i < 3; i++) {
    triad_colors[i] = triad_datas[i].material.baseColor;
  }

  auto grid_data = loadGrid(20);
  grid = createMesh(dev, grid_data);
  uploadMeshToDevice(dev, grid.toView(), grid_data);
  grid_color = grid_data.material.baseColor;

  scene.setupPipelines(triad.layout);
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
  _swapchainTextureFormat = renderer.getSwapchainTextureFormat();
  _depthFormat = renderer.depth_format;
}

void DebugScene::setupPipelines(const MeshLayout &layout) {
  if (linePipeline && trianglePipeline)
    return;
  Shader vertexShader{_device, "Hud3dElement.vert", 1};
  Shader fragmentShader{_device, "Hud3dElement.frag", 1};
  SDL_GPUColorTargetDescription color_desc;
  SDL_zero(color_desc);
  color_desc.format = _swapchainTextureFormat;
  SDL_GPUGraphicsPipelineCreateInfo info{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .vertex_input_state = layout.toVertexInputState(),
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .rasterizer_state{.fill_mode = SDL_GPU_FILLMODE_FILL,
                        .cull_mode = SDL_GPU_CULLMODE_NONE,
                        .depth_bias_constant_factor = 0.0001f,
                        .depth_bias_slope_factor = 0.001f,
                        .enable_depth_bias = true,
                        .enable_depth_clip = true},
      .depth_stencil_state{.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
                           .enable_depth_test = true,
                           .enable_depth_write = true},
      .target_info{.color_target_descriptions = &color_desc,
                   .num_color_targets = 1,
                   .depth_stencil_format = _depthFormat,
                   .has_depth_stencil_target = true},
  };
  if (!trianglePipeline)
    trianglePipeline = SDL_CreateGPUGraphicsPipeline(_device, &info);

  // re-use
  info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
  if (!linePipeline)
    linePipeline = SDL_CreateGPUGraphicsPipeline(_device, &info);
}

void DebugScene::render(Renderer &renderer, const Camera &camera) {
  drawCommands.clear();

  for (auto &module : modules) {
    module->addDrawCommands(*this, camera);
  }

  SDL_GPUColorTargetInfo color_target_info;
  SDL_zero(color_target_info);
  color_target_info.texture = renderer.swapchain;
  color_target_info.load_op = SDL_GPU_LOADOP_LOAD;
  color_target_info.store_op = SDL_GPU_STOREOP_STORE;
  SDL_GPUDepthStencilTargetInfo depth_target_info;
  depth_target_info.load_op = SDL_GPU_LOADOP_LOAD;
  depth_target_info.store_op = SDL_GPU_STOREOP_DONT_CARE;
  depth_target_info.stencil_load_op = SDL_GPU_LOADOP_LOAD;
  depth_target_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
  depth_target_info.texture = renderer.depth_texture;
  depth_target_info.cycle = false;

  SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
      renderer.command_buffer, &color_target_info, 1, &depth_target_info);

  for (const auto &cmd : drawCommands) {
    SDL_assert(cmd.mesh_views.size() == cmd.colors.size());
    switch (cmd.pipeline_type) {
    case DebugPipelines::TRIANGLE_FILL:
      SDL_BindGPUGraphicsPipeline(render_pass, trianglePipeline);
      break;
    case DebugPipelines::LINE:
      SDL_BindGPUGraphicsPipeline(render_pass, linePipeline);
      break;
    }
    renderer.pushVertexUniform(TRANSFORM_SLOT, &cmd.mvp, sizeof(cmd.mvp));
    renderer.bindMesh(render_pass, *cmd.mesh);
    for (size_t i = 0; i < cmd.mesh_views.size(); i++) {
      const auto &color = cmd.colors[i];
      renderer.pushFragmentUniform(COLOR_SLOT, &color, sizeof(color));
      renderer.drawView(render_pass, cmd.mesh_views[i]);
    }
  }

  SDL_EndGPURenderPass(render_pass);
}

void DebugScene::release() {
  if (_device) {
    SDL_ReleaseGPUGraphicsPipeline(_device, trianglePipeline);
    SDL_ReleaseGPUGraphicsPipeline(_device, linePipeline);
  }
}
} // namespace candlewick
