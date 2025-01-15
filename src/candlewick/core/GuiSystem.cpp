#include "GuiSystem.h"
#include "Renderer.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

namespace candlewick {

bool GuiSystem::init(const Renderer &renderer) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.IniFilename = nullptr;

  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForSDLGPU(renderer.window);
  ImGui_ImplSDLGPU3_InitInfo imguiInfo{
      .GpuDevice = renderer.device,
      .ColorTargetFormat =
          SDL_GetGPUSwapchainTextureFormat(renderer.device, renderer.window),
      .MSAASamples = SDL_GPU_SAMPLECOUNT_1,
  };
  return ImGui_ImplSDLGPU3_Init(&imguiInfo);
}

void GuiSystem::render(Renderer &renderer) {
  ImGui_ImplSDLGPU3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  this->callback_(renderer);

  ImGui::Render();
  ImDrawData *draw_data = ImGui::GetDrawData();
  Imgui_ImplSDLGPU3_PrepareDrawData(draw_data, renderer.command_buffer);

  SDL_GPUColorTargetInfo info{
      .texture = renderer.swapchain,
      .clear_color{},
      .load_op = SDL_GPU_LOADOP_LOAD,
      .store_op = SDL_GPU_STOREOP_STORE,
  };
  auto render_pass =
      SDL_BeginGPURenderPass(renderer.command_buffer, &info, 1, NULL);
  ImGui_ImplSDLGPU3_RenderDrawData(draw_data, renderer.command_buffer,
                                   render_pass);

  SDL_EndGPURenderPass(render_pass);
}

void GuiSystem::release() {
  ImGui_ImplSDL3_Shutdown();
  ImGui_ImplSDLGPU3_Shutdown();
  ImGui::DestroyContext();
}

} // namespace candlewick
