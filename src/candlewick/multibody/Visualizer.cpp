#include "Visualizer.h"
#include "../core/matrix_util.h"

#include <imgui.h>

namespace candlewick::multibody {

void Visualizer::default_gui_exec(Renderer &render) {
  static bool demo_window_open = true;
  auto &light = this->robotScene.directionalLight;
  ImGui::Begin("Renderer info & controls", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Text("Device driver: %s", render.device.driverName());
  ImGui::SeparatorText("Dir. light");
  ImGui::SliderFloat("Intensity", &light.intensity, 0.1f, 10.f);
  ImGui::End();

  ImGui::ShowDemoWindow(&demo_window_open);
}

void Visualizer::displayImpl() {

  static float angle{0.};
  angle += 1e-2f;
  float radius = 2.5f;
  Float3 eye{std::cos(angle), std::sin(angle), 0.5f};
  eye *= radius;
  Mat4f viewMat = lookAt(eye, {0., 0., 0.});
  Radf fov{45.0_degf};
  int w, h;
  SDL_GetWindowSize(renderer.window, &w, &h);
  float aspectRatio = float(w) / float(h);
  cameraState.setPoseFromView(viewMat);
  cameraState.projection = perspectiveFromFov(fov, aspectRatio, 0.01f, 100.f);
  robotScene.render(renderer, cameraState);
}
} // namespace candlewick::multibody
