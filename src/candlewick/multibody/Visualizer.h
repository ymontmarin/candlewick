#pragma once

#include "RobotScene.h"
#include "../core/Device.h"
#include "../core/Camera.h"
#include "../core/GuiSystem.h"
#include "../core/DebugScene.h"
#include "../core/Renderer.h"

#include <pinocchio/visualizers/base-visualizer.hpp>
#include <SDL3/SDL_init.h>
#include <entt/entity/registry.hpp>

namespace candlewick::multibody {

namespace {
  using pinocchio::visualizers::BaseVisualizer;
  using pinocchio::visualizers::ConstVectorRef;
  using pinocchio::visualizers::Vector3;
  using pinocchio::visualizers::VectorXs;
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

/// \brief A Pinocchio robot visualizer. The display() function will perform the
/// draw calls.
///
/// This visualizer is asynchronous. It will create its render context
/// (Renderer) in a separate thread along with the GPU device and window, and
/// run until shouldExit() returns true.
class Visualizer final : public BaseVisualizer {
public:
  using BaseVisualizer::setCameraPose;
  entt::registry registry;
  Renderer renderer;
  GuiSystem guiSystem;
  std::optional<RobotScene> robotScene;
  std::optional<DebugScene> debugScene;
  Camera camera;
  CameraControlParams cameraParams;

  static constexpr Radf DEFAULT_FOV = 55.0_radf;

  struct Config {
    Uint32 width;
    Uint32 height;
    SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
  };

  static Renderer createRenderer(const Config &config);

  /// \brief Default GUI callback for the Visualizer; provide your own callback
  /// to the Visualizer constructor to change this behaviour.
  static void default_gui_exec(Visualizer &viz);

  void resetCamera();
  void loadViewerModel() override;

  Visualizer(const Config &config, const pin::Model &model,
             const pin::GeometryModel &visualModel,
             GuiSystem::GuiBehavior gui_callback);

  Visualizer(const Config &config, const pin::Model &model,
             const pin::GeometryModel &visualModel)
      : Visualizer(config, model, visualModel,
                   [this](auto &) { default_gui_exec(*this); }) {}

  ~Visualizer() override;

  void displayPrecall() override { m_mutex.lock(); }
  void displayImpl() override { m_mutex.unlock(); }

  void setCameraTarget(const Eigen::Ref<const Vector3> &target) override;

  void setCameraPosition(const Eigen::Ref<const Vector3> &position) override;

  void setCameraPose(const Eigen::Ref<const Matrix4> &pose) override;

  void enableCameraControl(bool v) override { m_cameraControl = v; }

  void eventLoop();

  bool shouldExit() const noexcept { return m_shouldExit; }

  /// \brief Clear objects
  void clean() override {
    robotScene->clearEnvironment();
    robotScene->clearRobotGeometries();
    debugScene->registry().clear();
  }

private:
  bool m_cameraControl = true;
  bool m_shouldExit = false;
  // mutex to lock in precall
  std::mutex m_mutex;
  std::thread m_render_thread;

  void renderThreadMain(const Config &config);
};

inline Renderer Visualizer::createRenderer(const Config &config) {
  [[maybe_unused]] bool ret = SDL_Init(SDL_INIT_VIDEO);
  assert(ret);
  Device dev{auto_detect_shader_format_subset()};
  SDL_Window *window =
      SDL_CreateWindow("Candlewick Pinocchio visualizer", int(config.width),
                       int(config.height), 0);
  return Renderer{std::move(dev), window, config.depth_stencil_format};
}

} // namespace candlewick::multibody
