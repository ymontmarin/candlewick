#pragma once

#include <pinocchio-visualizers/base-visualizer.hpp>
#include "RobotScene.h"
#include "../core/Device.h"
#include "../core/GuiSystem.h"
#include "../core/Renderer.h"
#include "../utils/CameraControl.h"

#include "../core/LightUniforms.h"

#include <SDL3/SDL_init.h>

namespace candlewick::multibody {

using Eigen::Matrix3f;
using Eigen::Matrix4f;

struct VisualizerConfig {
  SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
};

struct alignas(16) TransformUniformData {
  GpuMat4 model;
  alignas(16) GpuMat4 mvp;
  alignas(16) GpuMat3 normalMatrix;
};

struct alignas(16) light_ubo_t {
  DirectionalLightUniform a;
  GpuVec3 viewPos;
};

class Visualizer : public pinocchio_visualizers::BaseVisualizer {
public:
  Renderer renderer;
  GuiSystem guiSys;
  RobotScene robotScene;

  struct Config {
    Uint32 width;
    Uint32 height;
  };

  static Renderer createRenderer(const Config &config) {
    SDL_Init(SDL_INIT_VIDEO);
    Device dev{SDL_GPU_SHADERFORMAT_SPIRV};
    auto window = SDL_CreateWindow("Candlewick Pinocchio visualizer",
                                   int(config.width), int(config.height), 0);
    return Renderer{std::move(dev), window, SDL_GPU_TEXTUREFORMAT_D24_UNORM};
  }

  void default_gui_exec(Renderer &render);

  Visualizer(Config config, const pin::Model &model,
             const pin::GeometryModel &visualModel,
             GuiSystem::GuiBehavior gui_callback)
      : pinocchio_visualizers::BaseVisualizer(model, visualModel),
        renderer(createRenderer(config)), guiSys(std::move(gui_callback)),
        robotScene(renderer, visualModel, {}) {
    guiSys.init(renderer);
  }

  Visualizer(Config config, const pin::Model &model,
             const pin::GeometryModel &visualModel)
      : Visualizer(config, model, visualModel,
                   [this](auto &r) { default_gui_exec(r); }) {}

  void displayImpl() override;

  ~Visualizer() {
    guiSys.release();
    renderer.destroy();
    SDL_DestroyWindow(renderer.window);
    SDL_Quit();
  }
};

} // namespace candlewick::multibody
