#include "candlewick/core/Device.h"
#include "candlewick/core/Shader.h"
#include "candlewick/core/MeshLayout.h"
#define SDL_GPU_SHADERCROSS_IMPLEMENTATION
#include "SDL_gpu_shadercross.h"
#include <array>

using float3 = std::array<float, 3>;
using float4 = std::array<float, 4>;
struct Vertex {
  float3 pos;
  float4 col;
};

float4 red = {1.f, 0., 0., 1.};
float4 green = {0.f, 1., 0., 1.};

// clang-format off
Vertex vertexData[] = {
  {{-1., -1., -1.}, red},
  {{-1., +1., -1.}, red},
  {{+1., +1., -1.}, green},
  {{+1., -1., -1.}, red},
  {{+1., -1., +1.}, red},
  {{-1., -1., +1.}, red},
  {{-1., +1., +1.}, red},
  {{+1., +1., +1.}, green},
};
// clang-format on

struct State {
  SDL_GPUBuffer *buf_vertex;
};

static State state;

int main() {
  using namespace candlewick;
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return 1;
  }
  SDL_ShaderCross_Init();
  Device device{SDL_ShaderCross_GetSPIRVShaderFormats()};

  SDL_Window *window = SDL_CreateWindow(__FILE_NAME__, 1280, 480, 0);
  if (!SDL_ClaimWindowForGPUDevice(device, window)) {
    SDL_Log("Error: %s", SDL_GetError());
    return 1;
  }

  // Shaders

  Shader vertexShader{device, "VertexColor.vert"};
  Shader fragmentShader{device, "VertexColor.frag"};

  // Buffers

  MeshLayout mesh_layout;
  mesh_layout.addBinding(0, sizeof(Vertex))
      .addAttribute(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                    offsetof(Vertex, pos))
      .addAttribute(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                    offsetof(Vertex, col));

  SDL_GPUBufferCreateInfo buffer_desc{.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                                      .size = sizeof(vertexData),
                                      .props = 0};

  state.buf_vertex = SDL_CreateGPUBuffer(device, &buffer_desc);
  if (!state.buf_vertex) {
    SDL_Log("Failed to create vertex buffer: %s", SDL_GetError());
    return 1;
  }
  SDL_SetGPUBufferName(device, state.buf_vertex, "vertex_buf");

  SDL_GPUTransferBufferCreateInfo transfer_buffer_desc{
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
      .size = sizeof(vertexData),
      .props = 0,
  };
  SDL_GPUTransferBuffer *trans_buffer =
      SDL_CreateGPUTransferBuffer(device, &transfer_buffer_desc);
  void *_map = SDL_MapGPUTransferBuffer(device, trans_buffer, false);
  SDL_memcpy(_map, vertexData, sizeof(vertexData));
  SDL_UnmapGPUTransferBuffer(device, trans_buffer);

  SDL_GPUCommandBuffer *cmdbuf;
  // Upload data to buffer
  {
    cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmdbuf);
    SDL_GPUTransferBufferLocation src_location;
    src_location.transfer_buffer = trans_buffer;
    src_location.offset = 0;

    SDL_GPUBufferRegion dst_location;
    dst_location.buffer = state.buf_vertex;
    dst_location.offset = 0;
    dst_location.size = sizeof(vertexData);

    SDL_UploadToGPUBuffer(copy_pass, &src_location, &dst_location, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
  }
  SDL_ReleaseGPUTransferBuffer(device, trans_buffer);
  // Pipeline

  SDL_GPUColorTargetDescription color_desc;
  SDL_zero(color_desc);
  color_desc.format = SDL_GetGPUSwapchainTextureFormat(device, window);

  SDL_GPUGraphicsPipelineCreateInfo pipeline_desc{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .vertex_input_state = mesh_layout.toVertexInputState(),
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .depth_stencil_state{
          .compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
          .enable_depth_test = true,
          .enable_depth_write = true,
      },
      .target_info{
          .color_target_descriptions = &color_desc, .num_color_targets = 1,
          //  .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM,
          //  .has_depth_stencil_target = false,
      },
      .props = 0};
  pipeline_desc.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
  SDL_GPUGraphicsPipeline *pipeline =
      SDL_CreateGPUGraphicsPipeline(device, &pipeline_desc);
  if (!pipeline) {
    SDL_Log("Failed to create pipeline: %s", SDL_GetError());
    return 1;
  }

  SDL_Log("Releasing shaders...");
  vertexShader.release();
  fragmentShader.release();

  // Main loop
  auto Render = [&]() {
    Uint32 w, h;
    SDL_GPURenderPass *render_pass;
    SDL_GPUBufferBinding vertex_binding;
    cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUTexture *swapchain;
    vertex_binding.buffer = state.buf_vertex;
    vertex_binding.offset = 0;
    swapchain = SDL_AcquireGPUSwapchainTexture(cmdbuf, window, &w, &h);
    if (swapchain) {
      SDL_Log("Acquired command buffer and swapchain.");
      SDL_GPUColorTargetInfo ctinfo{.texture = swapchain,
                                    .clear_color = SDL_FColor{},
                                    .load_op = SDL_GPU_LOADOP_CLEAR,
                                    .store_op = SDL_GPU_STOREOP_STORE};
      // SDL_GPUDepthStencilTargetInfo dstinfo;
      // dstinfo.texture = swapchain;
      render_pass = SDL_BeginGPURenderPass(cmdbuf, &ctinfo, 1, NULL);
      SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
      SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);
      SDL_DrawGPUPrimitives(render_pass, sizeof(vertexData) / sizeof(Vertex), 1,
                            0, 0);
      SDL_EndGPURenderPass(render_pass);
    }
    SDL_SubmitGPUCommandBuffer(cmdbuf);
  };

  Render();
  Render();
  Render();

  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);

  SDL_ReleaseGPUBuffer(device, state.buf_vertex);

  SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyWindow(window);
  device.destroy();
  SDL_ShaderCross_Quit();
}
