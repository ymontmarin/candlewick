#include "Visualizer.h"
#include "../core/DepthAndShadowPass.h"
#include "../primitives/Plane.h"

#include <imgui.h>

namespace candlewick::multibody {

void Visualizer::default_gui_exec(Renderer &render) {
  auto &light = this->robotScene.directionalLight;
  ImGui::Begin("Renderer info & controls", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Text("Device driver: %s", render.device.driverName());
  ImGui::SeparatorText("Dir. light");
  ImGui::SliderFloat("Intensity", &light.intensity, 0.1f, 10.f);
  ImGui::ColorEdit3("color", light.color.data());
  ImGui::SeparatorText("Debug elements");
  ImGui::End();
}

Visualizer::Visualizer(const Config &config, const pin::Model &model,
                       const pin::GeometryModel &visualModel,
                       GuiSystem::GuiBehavior gui_callback)
    : BaseVisualizer(model, visualModel), registry{},
      renderer(createRenderer(config)), guiSys(std::move(gui_callback)),
      robotScene(registry, renderer, visualModel, visualData,
                 {.enable_shadows = true}),
      debugScene(renderer) {
  robotScene.directionalLight = {
      .direction = {0., -1., -1.},
      .color = {1.0, 1.0, 1.0},
      .intensity = 8.0,
  };
  guiSys.init(renderer);

  {
    const float radius = 2.5f;
    const auto angle = 45.0_degf;
    Float3 eye{std::cos(angle), std::sin(angle), 0.5f};
    eye *= radius;
    const Radf defaultFov{55.0_degf};
    int w, h;
    SDL_GetWindowSize(renderer.window, &w, &h);
    float aspectRatio = float(w) / float(h);
    camera.view = lookAt(eye, {0., 0., 0.});
    camera.projection =
        perspectiveFromFov(defaultFov, aspectRatio, 0.01f, 100.f);
  }

  robotScene.worldSpaceBounds.grow({-1.f, -1.f, 0.f});
  robotScene.worldSpaceBounds.grow({+1.f, +1.f, 1.f});

  Uint32 prepeat = 25;
  auto plane_obj = robotScene.addEnvironmentObject(
      loadPlaneTiled(0.5f, prepeat, prepeat), Mat4f::Identity());
}

void Visualizer::loadViewerModel() {}

void Visualizer::displayImpl() {
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
  } else {
  }

  renderer.endFrame();
}

void Visualizer::setCameraTarget(const Eigen::Ref<const Vector3s> &target) {
  Float3 eye = this->camera.position();
  camera.view = ::candlewick::lookAt(eye, target.cast<float>());
}

void Visualizer::setCameraPosition(const Eigen::Ref<const Vector3s> &position) {
  ::candlewick::cameraSetWorldPosition(camera, position.cast<float>());
}

void Visualizer::setCameraPose(const Eigen::Ref<const Matrix4s> &pose) {
  camera.view = pose.cast<float>().inverse();
}
} // namespace candlewick::multibody
