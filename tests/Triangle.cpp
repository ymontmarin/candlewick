#include "candlewick/core/Shader.h"
#include "candlewick/core/Device.h"
#include <SDL3/SDL_init.h>
// #define SDL_GPU_SHADERCROSS_IMPLEMENTATION
// #include "SDL_gpu_shadercross.h"

using namespace candlewick;
SDL_Window *window;

int main() {

  // Initialize SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Failed to init SDL: %s", SDL_GetError());
  }
  // SDL_ShaderCross_Init();
  const int numGpus = SDL_GetNumGPUDrivers();
  SDL_Log("Found %d GPU drivers.", numGpus);

  const char *title = "sdl_window_test";
  Device device{// SDL_ShaderCross_GetSPIRVShaderFormats()
                SDL_GPU_SHADERFORMAT_SPIRV, true};
  if (!device)
    return 1;
  Uint32 w = 1280;
  Uint32 h = 480;
  if (!(window = SDL_CreateWindow(title, w, h, 0))) {
    SDL_Log("Failed to create window! %s", SDL_GetError());
    return 1;
  }

  if (!SDL_ClaimWindowForGPUDevice(device, window)) {
    SDL_Log("Failed to claim window for GPU device");
    return 1;
  }

  Shader vertexShader{device, "BasicTriangle.vert"};

  Shader fragmentShader{device, "SolidColor.frag"};

  SDL_GPUColorTargetDescription coldescs[]{
      {.format = SDL_GetGPUSwapchainTextureFormat(device, window)},
  };
  SDL_GPUGraphicsPipelineCreateInfo pipeInfo{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .target_info =
          {
              .color_target_descriptions = coldescs,
              .num_color_targets = 1,
          },
  };
  pipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
  SDL_GPUGraphicsPipeline *pipelineFill =
      SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

  pipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
  SDL_GPUGraphicsPipeline *pipelineLine =
      SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

  bool MustQuit = false;

  while (!MustQuit) {
    SDL_Event evt;

    while (SDL_PollEvent(&evt)) {
      if (evt.type == SDL_EVENT_QUIT) {
        MustQuit = true;
        break;
      } else {
        // get command buffer
        SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(device);
        if (!cmdbuf) {
          SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
          return 1;
        }

        SDL_GPUTexture *swapchainTexture =
            SDL_AcquireGPUSwapchainTexture(cmdbuf, window, &w, &h);
        if (!swapchainTexture) {
          SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
          return 1;
        }
        SDL_Log("Acquired command buffer and swapchain.");
        SDL_GPUColorTargetInfo ctinfo;
        ctinfo.texture = swapchainTexture;
        ctinfo.clear_color = SDL_FColor{0., 0., 0., 1.};
        ctinfo.load_op = SDL_GPU_LOADOP_CLEAR;
        ctinfo.store_op = SDL_GPU_STOREOP_STORE;
        SDL_GPURenderPass *renderPass =
            SDL_BeginGPURenderPass(cmdbuf, &ctinfo, 1, NULL);
        SDL_BindGPUGraphicsPipeline(renderPass, pipelineFill);

        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
        SDL_EndGPURenderPass(renderPass);
        SDL_SubmitGPUCommandBuffer(cmdbuf);
      }
    }
    if (MustQuit) {
      break;
    }
  }
  SDL_Log("Quitting...");
  vertexShader.release();
  fragmentShader.release();
  SDL_ReleaseGPUGraphicsPipeline(device, pipelineFill);
  SDL_ReleaseGPUGraphicsPipeline(device, pipelineLine);

  SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyWindow(window);
  device.destroy();

  return 0;
}
