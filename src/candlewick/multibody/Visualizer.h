#pragma once

#include <pinocchio-visualizers/base-visualizer.hpp>
#include "RobotScene.h"
#include "../core/Device.h"
#include "../core/Camera.h"
#include "../core/GuiSystem.h"
#include "../core/DebugScene.h"
#include "../core/Renderer.h"

#include <SDL3/SDL_init.h>

namespace candlewick::multibody {

namespace {
using pinocchio_visualizers::BaseVisualizer;
using pinocchio_visualizers::ConstVectorRef;
using pinocchio_visualizers::Matrix4s;
using pinocchio_visualizers::Vector3s;
using pinocchio_visualizers::VectorXs;
} // namespace

struct VisualizerConfig {
  SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
};

/// \brief A synchronous renderer. The display() function will perform the draw
/// calls.
class Visualizer final : public BaseVisualizer {
public:
  using BaseVisualizer::setCameraPose;
  entt::registry registry;
  Renderer renderer;
  GuiSystem guiSys;
  RobotScene robotScene;
  DebugScene debugScene;
  Camera camera;

private:
  BasicDebugModule *basic_debug_module;

public:
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

  void loadViewerModel() override;

  Visualizer(Config config, const pin::Model &model,
             const pin::GeometryModel &visualModel,
             GuiSystem::GuiBehavior gui_callback);

  Visualizer(Config config, const pin::Model &model,
             const pin::GeometryModel &visualModel)
      : Visualizer(config, model, visualModel,
                   [this](auto &r) { default_gui_exec(r); }) {}

  void displayImpl() override;

  void setCameraTarget(const Eigen::Ref<const Vector3s> &target) override;

  void setCameraPosition(const Eigen::Ref<const Vector3s> &position) override;

  void setCameraPose(const Eigen::Ref<const Matrix4s> &pose) override;

  ~Visualizer() {
    robotScene.release();
    debugScene.release();
    guiSys.release();
    renderer.destroy();
    SDL_DestroyWindow(renderer.window);
    SDL_Quit();
  }
};

} // namespace candlewick::multibody
