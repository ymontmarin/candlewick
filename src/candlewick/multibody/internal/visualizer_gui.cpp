#include "../Visualizer.h"

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

} // namespace candlewick::multibody
