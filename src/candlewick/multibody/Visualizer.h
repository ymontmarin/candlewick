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

/// Camera control parameters: senstivities, key bindings, etc...
/// \todo Move to some kind of "runtime" library, or back to core?
struct CameraControlParams {
  float rotSensitivity = 5e-3f;
  float panSensitivity = 1e-2f;
  float zoomSensitivity = 0.05f;
  float localRotSensitivity = 0.01f;
  bool yInvert = true;

  // modifier key assignments
  struct ModifierConfig {
    SDL_Keymod zoomModifier = SDL_KMOD_LCTRL;
  } modifiers;
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
  CameraControlParams cameraParams;

  struct Config {
    Uint32 width;
    Uint32 height;
    SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
  };

  static Renderer createRenderer(const Config &config);

  /// \brief Default GUI callback for the Visualizer; provide your own callback
  /// to the Visualizer constructor to change this behaviour.
  static void default_gui_exec(Visualizer &viz);

  void loadViewerModel() override;

  Visualizer(const Config &config, const pin::Model &model,
             const pin::GeometryModel &visualModel,
             GuiSystem::GuiBehavior gui_callback);

  Visualizer(const Config &config, const pin::Model &model,
             const pin::GeometryModel &visualModel)
      : Visualizer(config, model, visualModel,
                   [this](auto &) { default_gui_exec(*this); }) {}

  ~Visualizer() noexcept;

  void displayImpl() override;

  void setCameraTarget(const Eigen::Ref<const Vector3s> &target) override;

  void setCameraPosition(const Eigen::Ref<const Vector3s> &position) override;

  void setCameraPose(const Eigen::Ref<const Matrix4s> &pose) override;

  void enableCameraControl(bool v) override { m_cameraControl = v; }

  void eventLoop();

  bool shouldExit() const noexcept { return m_shouldExit; }

  /// \brief Clear objects (for now, only environment entities).
  /// \todo Allow cleaning up the robot.
  void clearEnvironment() {
    registry.destroy(m_environmentEntities.begin(),
                     m_environmentEntities.end());
  }

private:
  bool m_cameraControl = true;
  bool m_shouldExit = false;
  std::vector<entt::entity> m_environmentEntities;
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
