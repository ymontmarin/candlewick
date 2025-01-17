#pragma once

#include "Scene.h"
#include "Mesh.h"
#include "Renderer.h"
#include "math_util.h"

#include <memory>

namespace candlewick {

enum class DebugPipelines {
  TRIANGLE_FILL,
  LINE,
};

struct DebugDrawCommand {
  DebugPipelines pipeline_type;
  const Mesh *mesh;
  std::vector<MeshView> mesh_views;
  // uniform data :: matches our common debug shader
  // vertex
  GpuMat4 mvp;
  // fragment
  std::vector<GpuVec4> colors;
};

class DebugScene;

struct DebugModule {
  virtual void addDrawCommands(DebugScene &scene, const Camera &camera) = 0;
  virtual ~DebugModule() = default;
};

/// \brief Just the basic 3D triad.
struct BasicDebugModule : DebugModule {
  inline static GpuVec4 DEFAULT_GRID_COLOR = 0xE0A236ff_rgbaf;

  Mesh triad;
  std::array<MeshView, 3> triad_views;
  std::array<GpuVec4, 3> triad_colors;
  Mesh grid;
  GpuVec4 grid_color = DEFAULT_GRID_COLOR;
  SDL_GPUGraphicsPipeline *pipeline;

  bool enableTriad{true};
  bool enableGrid{true};

  BasicDebugModule(const Device &dev);
  void addDrawCommands(DebugScene &scene, const Camera &camera) override;
};

class DebugScene {
  std::vector<std::unique_ptr<DebugModule>> modules;
  const Device &_device;
  SDL_GPUGraphicsPipeline *trianglePipeline;
  SDL_GPUGraphicsPipeline *linePipeline;

public:
  enum { TRANSFORM_SLOT = 0 };
  enum { COLOR_SLOT = 0 };

  DebugScene(const Renderer &renderer);

  std::vector<DebugDrawCommand> drawCommands;

  template <std::derived_from<DebugModule> U, typename... CtorArgs>
  U &addModule(CtorArgs &&...ctor_args) {
    U *p = new U(std::forward<CtorArgs>(ctor_args)...);
    modules.emplace_back(p);
    return *p;
  }

  void render(Renderer &renderer, const Camera &camera) {
    drawCommands.clear();

    for (auto &module : modules) {
      module->addDrawCommands(*this, camera);
    }

    SDL_GPUColorTargetInfo color_target_info{
        .texture = renderer.swapchain,
        .clear_color{},
        .load_op = SDL_GPU_LOADOP_LOAD,
        .store_op = SDL_GPU_STOREOP_STORE,
        .cycle = false,
    };
    SDL_GPUDepthStencilTargetInfo depth_target_info;
    depth_target_info.load_op = SDL_GPU_LOADOP_LOAD;
    depth_target_info.store_op = SDL_GPU_STOREOP_DONT_CARE;
    depth_target_info.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
    depth_target_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
    depth_target_info.texture = renderer.depth_texture;

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

  void release();
};

static_assert(Scene<DebugScene>);

inline void DebugScene::release() {
  if (_device) {
    SDL_ReleaseGPUGraphicsPipeline(_device, trianglePipeline);
    SDL_ReleaseGPUGraphicsPipeline(_device, linePipeline);
  }
}

} // namespace candlewick
