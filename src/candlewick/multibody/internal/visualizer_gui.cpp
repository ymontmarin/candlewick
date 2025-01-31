#include "../Visualizer.h"

#include <SDL3/SDL_events.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

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

void Visualizer::eventLoop() {
  ImGuiIO &io = ImGui::GetIO();

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);

    if (event.type == SDL_EVENT_QUIT) {
      m_shouldExit = true;
    }

    if (io.WantCaptureMouse | io.WantCaptureKeyboard)
      continue;

    switch (event.type) {}
  }
}

} // namespace candlewick::multibody
