#include "Visualizer.h"

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
  ImGui::Checkbox("Show grid", &basic_debug_module->enableGrid);
  ImGui::Checkbox("Show triad", &basic_debug_module->enableTriad);
  ImGui::End();
}

Visualizer::Visualizer(Config config, const pin::Model &model,
                       const pin::GeometryModel &visualModel,
                       GuiSystem::GuiBehavior gui_callback)
    : BaseVisualizer(model, visualModel), registry{},
      renderer(createRenderer(config)), guiSys(std::move(gui_callback)),
      robotScene(registry, renderer, visualModel, visualData, {}),
      debugScene(renderer) {
  robotScene.directionalLight = {
      .direction = {0., -1., -1.},
      .color = {1.0, 1.0, 1.0},
      .intensity = 8.0,
  };
  guiSys.init(renderer);
  this->basic_debug_module = &debugScene.addModule<BasicDebugModule>();

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
}

void Visualizer::loadViewerModel() {}

void Visualizer::displayImpl() {
  renderer.beginFrame();
  assert(renderer.waitAndAcquireSwapchain());

  robotScene.render(renderer, camera);
  debugScene.render(renderer, camera);
  guiSys.render(renderer);

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
