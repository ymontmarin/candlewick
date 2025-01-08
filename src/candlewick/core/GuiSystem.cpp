#include "GuiSystem.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

namespace candlewick {

void GuiSystem::render(Renderer &render) {
  ImGui_ImplSDLGPU_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  this->callback_(render);

  ImGui::Render();
  ImDrawData *draw_data = ImGui::GetDrawData();
  Imgui_ImplSDLGPU_PrepareDrawData(draw_data, render.command_buffer);

  SDL_GPUColorTargetInfo info{
      .texture = render.swapchain,
      .clear_color{},
      .load_op = SDL_GPU_LOADOP_LOAD,
      .store_op = SDL_GPU_STOREOP_STORE,
  };
  auto render_pass =
      SDL_BeginGPURenderPass(render.command_buffer, &info, 1, NULL);
  ImGui_ImplSDLGPU_RenderDrawData(draw_data, render.command_buffer,
                                  render_pass);

  SDL_EndGPURenderPass(render_pass);
}

} // namespace candlewick
