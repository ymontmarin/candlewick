#include "../Visualizer.h"

#include <SDL3/SDL_events.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

namespace candlewick::multibody {

void add_light_gui_entry(DirectionalLight &light) {
  ImGui::SliderFloat("intensity", &light.intensity, 0.1f, 10.f);
  ImGui::DragFloat3("direction", light.direction.data(), 0.0f, -1.f, 1.f);
  light.direction.stableNormalize();
  ImGui::ColorEdit3("color", light.color.data());
}

void Visualizer::default_gui_exec(Visualizer &viz) {
  auto &render = viz.renderer;
  static bool show_demo_window = false;
  auto &light = viz.robotScene.directionalLight;
  ImGui::Begin("Renderer info & controls", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Text("Device driver: %s", render.device.driverName());

  ImGui::SeparatorText("Lights");
  ImGui::SetItemTooltip("Configuration for lights");
  add_light_gui_entry(light);

  ImGui::End();

  ImGui::ShowDemoWindow(&show_demo_window);
}

void Visualizer::eventLoop() {
  ImGuiIO &io = ImGui::GetIO();

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);

    if (event.type == SDL_EVENT_QUIT) {
      _shouldExit = true;
    }

    if (io.WantCaptureMouse | io.WantCaptureKeyboard)
      continue;

    switch (event.type) {}
  }
}

} // namespace candlewick::multibody
