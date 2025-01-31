#pragma once

#include <pinocchio-visualizers/base-visualizer.hpp>
#include "RobotScene.h"
#include "../core/Device.h"
#include "../core/Camera.h"
#include "../core/GuiSystem.h"
#include "../core/DebugScene.h"
#include "../core/Renderer.h"

#include <SDL3/SDL_init.h>
#include <entt/entity/registry.hpp>

namespace candlewick::multibody {

namespace {
  using pinocchio_visualizers::BaseVisualizer;
  using pinocchio_visualizers::ConstVectorRef;
  using pinocchio_visualizers::Matrix4s;
  using pinocchio_visualizers::Vector3s;
  using pinocchio_visualizers::VectorXs;
} // namespace

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

  struct Config {
    Uint32 width;
    Uint32 height;
    SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
  };

  static Renderer createRenderer(const Config &config);

  /// \brief Default GUI callback for the Visualizer; provide your own callback
  /// to the Visualizer constructor to change this behaviour.
  void default_gui_exec(Renderer &render);

  void loadViewerModel() override;

  Visualizer(const Config &config, const pin::Model &model,
             const pin::GeometryModel &visualModel,
             GuiSystem::GuiBehavior gui_callback);

  Visualizer(const Config &config, const pin::Model &model,
             const pin::GeometryModel &visualModel)
      : Visualizer(config, model, visualModel,
                   [this](auto &r) { default_gui_exec(r); }) {}

  ~Visualizer() noexcept;

  void displayImpl() override;

  void setCameraTarget(const Eigen::Ref<const Vector3s> &target) override;

  void setCameraPosition(const Eigen::Ref<const Vector3s> &position) override;

  void setCameraPose(const Eigen::Ref<const Matrix4s> &pose) override;

  void eventLoop();

  bool shouldExit() const noexcept { return m_shouldExit; }

private:
  bool m_shouldExit = false;
};

inline Renderer Visualizer::createRenderer(const Config &config) {
  bool ret = SDL_Init(SDL_INIT_VIDEO);
  assert(ret);
  Device dev{auto_detect_shader_format_subset()};
  SDL_Window *window =
      SDL_CreateWindow("Candlewick Pinocchio visualizer", int(config.width),
                       int(config.height), 0);
  return Renderer{std::move(dev), window, config.depth_stencil_format};
}

} // namespace candlewick::multibody
