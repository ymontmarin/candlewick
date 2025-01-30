#include "DebugScene.h"
#include "Camera.h"
#include "Shader.h"

#include "../primitives/Arrow.h"
#include "../primitives/Grid.h"

namespace candlewick {

void DebugScene::onDestroyMeshComponent(entt::registry &registry,
                                        entt::entity entity) {
  // release the Mesh object
  registry.get<DebugMeshComponent>(entity).mesh.release(_device);
}

DebugScene::DebugScene(const Renderer &renderer)
    : _device(renderer.device), _trianglePipeline(nullptr),
      _linePipeline(nullptr), _registry() {
  _swapchainTextureFormat = renderer.getSwapchainTextureFormat();
  _depthFormat = renderer.depth_format;

  _connections.emplace_back(
      _registry.on_destroy<DebugMeshComponent>()
          .connect<&DebugScene::onDestroyMeshComponent>(*this));
}

std::tuple<entt::entity, DebugMeshComponent &> DebugScene::addTriad() {
  auto triad_datas = createTriad();
  std::vector<GpuVec4> triad_colors(3);
  Mesh triad = createMeshFromBatch(_device, triad_datas, true);
  for (size_t i = 0; i < 3; i++) {
    triad_colors[i] = triad_datas[i].material.baseColor;
  }
  setupPipelines(triad.layout);
  auto entity = _registry.create();
  auto &item = _registry.emplace<DebugMeshComponent>(
      entity, DebugPipelines::TRIANGLE_FILL, std::move(triad), triad_colors,
      Mat4f::Identity());
  return {entity, item};
}

std::tuple<entt::entity, DebugMeshComponent &>
DebugScene::addLineGrid(std::optional<Float4> color) {
  auto grid_data = loadGrid(20);
  Mesh grid = createMesh(_device, grid_data, true);
  GpuVec4 grid_color = color.value_or(grid_data.material.baseColor);

  setupPipelines(grid.layout);
  auto entity = _registry.create();
  auto &item = _registry.emplace<DebugMeshComponent>(
      entity, DebugPipelines::LINE, std::move(grid), std::vector{grid_color},
      Mat4f::Identity());
  return {entity, item};
}

void DebugScene::renderMeshComponents(Renderer &renderer,
                                      SDL_GPURenderPass *render_pass,
                                      const Camera &camera) const {
  const Mat4f viewProj = camera.viewProj();

  auto mesh_view = _registry.view<const DebugMeshComponent>();
  for (auto [ent, cmd] : mesh_view.each()) {
    if (!cmd.enable)
      continue;

    switch (cmd.pipeline_type) {
    case DebugPipelines::TRIANGLE_FILL:
      SDL_BindGPUGraphicsPipeline(render_pass, _trianglePipeline);
      break;
    case DebugPipelines::LINE:
      SDL_BindGPUGraphicsPipeline(render_pass, _linePipeline);
      break;
    }

    const GpuMat4 mvp = viewProj * cmd.transform;
    renderer.pushVertexUniform(TRANSFORM_SLOT, &mvp, sizeof(mvp));
    renderer.bindMesh(render_pass, cmd.mesh);
    for (size_t i = 0; i < cmd.mesh.numViews(); i++) {
      const auto &color = cmd.colors[i];
      renderer.pushFragmentUniform(COLOR_SLOT, &color, sizeof(color));
      renderer.drawView(render_pass, cmd.mesh.view(i));
    }
  }
}

void DebugScene::setupPipelines(const MeshLayout &layout) {
  if (_linePipeline && _trianglePipeline)
    return;
  auto vertexShader = Shader::fromMetadata(_device, "Hud3dElement.vert");
  auto fragmentShader = Shader::fromMetadata(_device, "Hud3dElement.frag");
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
  if (!_trianglePipeline)
    _trianglePipeline = SDL_CreateGPUGraphicsPipeline(_device, &info);

  // re-use
  info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
  if (!_linePipeline)
    _linePipeline = SDL_CreateGPUGraphicsPipeline(_device, &info);
}

void DebugScene::render(Renderer &renderer, const Camera &camera) const {

  SDL_GPUColorTargetInfo color_target_info;
  SDL_zero(color_target_info);
  color_target_info.texture = renderer.swapchain;
  color_target_info.load_op = SDL_GPU_LOADOP_LOAD;
  color_target_info.store_op = SDL_GPU_STOREOP_STORE;
  SDL_GPUDepthStencilTargetInfo depth_target_info;
  depth_target_info.load_op = SDL_GPU_LOADOP_LOAD;
  depth_target_info.store_op = SDL_GPU_STOREOP_STORE;
  depth_target_info.stencil_load_op = SDL_GPU_LOADOP_LOAD;
  depth_target_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
  depth_target_info.texture = renderer.depth_texture;
  depth_target_info.cycle = false;

  SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
      renderer.command_buffer, &color_target_info, 1, &depth_target_info);

  // step 1: render using common pipelines
  renderMeshComponents(renderer, render_pass, camera);

  // step 2: call custom rendering logic of registered subsystems
  for (auto &system : _systems) {
    system->render(*this, renderer, render_pass, camera);
  }

  SDL_EndGPURenderPass(render_pass);
}

void DebugScene::release() {
  if (_device) {
    SDL_ReleaseGPUGraphicsPipeline(_device, _trianglePipeline);
    SDL_ReleaseGPUGraphicsPipeline(_device, _linePipeline);
  }
  // clean up all DebugMeshComponent objects.
  _registry.clear<DebugMeshComponent>();
}
} // namespace candlewick
