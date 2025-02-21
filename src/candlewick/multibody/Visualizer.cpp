#include "Visualizer.h"
#include "../core/Device.h"
#include "../core/CameraControls.h"
#include "../core/DepthAndShadowPass.h"
#include "../core/errors.h"
#include "../primitives/Plane.h"

namespace candlewick::multibody {

Visualizer::Visualizer(const Config &config, const pin::Model &model,
                       const pin::GeometryModel &visual_model,
                       GuiSystem::GuiBehavior gui_callback)
    : BaseVisualizer(model, visual_model), registry{}, renderer(NoInit),
      guiSystem(NoInit, std::move(gui_callback)) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    throw std::runtime_error(
        std::format("Failed to init video: {}", SDL_GetError()));
  }

  Device dev{auto_detect_shader_format_subset()};
  SDL_Window *window =
      SDL_CreateWindow("Candlewick Pinocchio visualizer", int(config.width),
                       int(config.height), SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if (!window) {
    throw RAIIException(
        std::format("Failed to acquire window: {}", SDL_GetError()));
  }
  SDL_Log("Video driver: %s", SDL_GetCurrentVideoDriver());
  ::new (&renderer)
      Renderer{std::move(dev), window, config.depth_stencil_format};

  robotScene.emplace(registry, renderer, visualModel(), visualData(),
                     RobotScene::Config{.enable_shadows = true});
  debugScene.emplace(registry, renderer);

  robotScene->directionalLight = {
      .direction = {0., -1., -1.},
      .color = {1.0, 1.0, 1.0},
      .intensity = 8.0,
  };
  guiSystem.init(renderer);

  robotScene->worldSpaceBounds.grow({-1.f, -1.f, 0.f});
  robotScene->worldSpaceBounds.grow({+1.f, +1.f, 1.f});

  Uint32 prepeat = 25;
  robotScene->addEnvironmentObject(loadPlaneTiled(0.5f, prepeat, prepeat),
                                   Mat4f::Identity());

  this->resetCamera();
}

void Visualizer::resetCamera() {
  const float radius = 2.5f;
  const Degf xy_plane_view_angle = 45.0_degf;
  Float3 eye{std::cos(xy_plane_view_angle), std::sin(xy_plane_view_angle),
             0.5f};
  eye *= radius;
  int w, h;
  SDL_GetWindowSize(renderer.window, &w, &h);
  float aspectRatio = float(w) / float(h);
  camera.view = lookAt(eye, {0., 0., 0.});
  camera.projection =
      perspectiveFromFov(DEFAULT_FOV, aspectRatio, 0.01f, 100.f);
}

void Visualizer::loadViewerModel() {}

void Visualizer::setCameraTarget(const Eigen::Ref<const Vector3> &target) {
  Float3 eye = this->camera.position();
  camera.view = lookAt(eye, target.cast<float>());
}

void Visualizer::setCameraPosition(const Eigen::Ref<const Vector3> &position) {
  camera_util::setWorldPosition(camera, position.cast<float>());
}

void Visualizer::setCameraPose(const Eigen::Ref<const Matrix4> &pose) {
  camera.view = pose.cast<float>().inverse();
}

Visualizer::~Visualizer() {
  // m_render_thread.join();

  robotScene->release();
  debugScene->release();
  guiSystem.release();
  renderer.destroy();
  SDL_DestroyWindow(renderer.window);
  SDL_Quit();
}

void Visualizer::displayImpl() {
  this->processEvents();

  debugScene->update();
  {
    robotScene->updateTransforms();
  }
  render();
}

void Visualizer::render() {

  CommandBuffer cmdBuf = renderer.acquireCommandBuffer();
  if (renderer.waitAndAcquireSwapchain(cmdBuf)) {
    robotScene->collectOpaqueCastables();
    std::span castables = robotScene->castables();
    renderShadowPassFromAABB(cmdBuf, robotScene->shadowPass,
                             robotScene->directionalLight, castables,
                             robotScene->worldSpaceBounds);

    robotScene->render(cmdBuf, camera);
    debugScene->render(cmdBuf, camera);
    guiSystem.render(cmdBuf);
  }

  cmdBuf.submit();
}

} // namespace candlewick::multibody
