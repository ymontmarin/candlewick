#include "Visualizer.h"
#include "../core/CameraControls.h"
#include "../core/DepthAndShadowPass.h"
#include "../primitives/Plane.h"

namespace candlewick::multibody {

Visualizer::Visualizer(const Config &config, const pin::Model &model,
                       const pin::GeometryModel &visualModel,
                       GuiSystem::GuiBehavior gui_callback)
    : BaseVisualizer(model, visualModel), registry{},
      renderer(createRenderer(config)), guiSys(NoInit, std::move(gui_callback)),
      robotScene(registry, renderer, visualModel, visualData,
                 {.enable_shadows = true}),
      debugScene(renderer) {
  robotScene.directionalLight = {
      .direction = {0., -1., -1.},
      .color = {1.0, 1.0, 1.0},
      .intensity = 8.0,
  };
  guiSys.init(renderer);

  robotScene.worldSpaceBounds.grow({-1.f, -1.f, 0.f});
  robotScene.worldSpaceBounds.grow({+1.f, +1.f, 1.f});

  Uint32 prepeat = 25;
  robotScene.addEnvironmentObject(loadPlaneTiled(0.5f, prepeat, prepeat),
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

void Visualizer::displayImpl() {

  this->eventLoop();

  debugScene.update();

  renderer.beginFrame();
  if (renderer.waitAndAcquireSwapchain()) {
    updateRobotTransforms(registry, visualData);
    robotScene.collectOpaqueCastables();
    std::span castables = robotScene.castables();
    renderShadowPassFromAABB(renderer, robotScene.shadowPass,
                             robotScene.directionalLight, castables,
                             robotScene.worldSpaceBounds);

    robotScene.render(renderer, camera);
    debugScene.render(renderer, camera);
    guiSys.render(renderer);
  }

  renderer.endFrame();
}

void Visualizer::setCameraTarget(const Eigen::Ref<const Vector3s> &target) {
  Float3 eye = this->camera.position();
  camera.view = lookAt(eye, target.cast<float>());
}

void Visualizer::setCameraPosition(const Eigen::Ref<const Vector3s> &position) {
  camera_util::setWorldPosition(camera, position.cast<float>());
}

void Visualizer::setCameraPose(const Eigen::Ref<const Matrix4s> &pose) {
  camera.view = pose.cast<float>().inverse();
}

Visualizer::~Visualizer() noexcept {
  robotScene.release();
  debugScene.release();
  guiSys.release();
  renderer.destroy();
  SDL_DestroyWindow(renderer.window);
  SDL_Quit();
}
} // namespace candlewick::multibody
