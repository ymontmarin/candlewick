#include "GuiSystem.h"
#include "Renderer.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

#include <stdexcept>

namespace candlewick {

GuiSystem::GuiSystem(const Renderer &renderer, GuiBehavior behav)
    : _renderer(&renderer), _callback(behav) {
  if (!init(renderer)) {
    throw std::runtime_error("Failed to initialize ImGui for SDLGPU3.");
  }
}

bool GuiSystem::init(const Renderer &renderer) {
  _renderer = &renderer;
  assert(!_initialized); // can't initialize twice
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.IniFilename = nullptr;

  ImGui::StyleColorsDark();
  if (!ImGui_ImplSDL3_InitForSDLGPU(renderer.window)) {
    return false;
  }
  ImGui_ImplSDLGPU3_InitInfo imguiInfo{
      .Device = renderer.device,
      .ColorTargetFormat =
          SDL_GetGPUSwapchainTextureFormat(renderer.device, renderer.window),
      .MSAASamples = SDL_GPU_SAMPLECOUNT_1,
  };
  _initialized = ImGui_ImplSDLGPU3_Init(&imguiInfo);
  return _initialized;
}

void GuiSystem::render(CommandBuffer &cmdBuf) {
  ImGui_ImplSDLGPU3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  this->_callback(*_renderer);

  ImGui::Render();
  ImDrawData *draw_data = ImGui::GetDrawData();
  Imgui_ImplSDLGPU3_PrepareDrawData(draw_data, cmdBuf);

  SDL_GPUColorTargetInfo info{
      .texture = _renderer->swapchain,
      .clear_color{},
      .load_op = SDL_GPU_LOADOP_LOAD,
      .store_op = SDL_GPU_STOREOP_STORE,
  };
  auto render_pass = SDL_BeginGPURenderPass(cmdBuf, &info, 1, NULL);
  ImGui_ImplSDLGPU3_RenderDrawData(draw_data, cmdBuf, render_pass);

  SDL_EndGPURenderPass(render_pass);
}

void GuiSystem::release() {
  ImGui_ImplSDL3_Shutdown();
  ImGui_ImplSDLGPU3_Shutdown();
  ImGui::DestroyContext();
}

} // namespace candlewick
