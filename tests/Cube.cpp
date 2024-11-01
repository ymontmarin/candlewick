#include "candlewick/core/Device.h"
#include "candlewick/core/Shader.h"
#include "candlewick/core/MeshLayout.h"
#include <SDL3_gpu_shadercross/SDL_gpu_shadercross.h>
#include "math_util.hpp"

struct Vertex {
  Float3 pos;
  Float4 col;
};

const Float4 red = {1.f, 0., 0., 1.};
const Float4 green = {0.f, 1., 0., 1.};
const Float4 blue = {0.f, 0., 1., 1.};

// clang-format off
const Vertex vertexData[] = {
  // top
  // f1
  {{0., 0., 1.}, red},
  {{1., 0., 1.}, (blue + red) / 2},
  {{1., 1., 1.}, blue},
  // f2
  {{1., 1., 1.}, blue},
  {{0., 1., 1.}, green},
  {{0., 0., 1.}, red},
  // -y
  // f3
  {{0., 0., 1.}, red},
  {{0., 0., 0.}, blue},
  {{1., 0., 0.}, red},
  // f4
  {{1., 0., 0.}, red},
  {{1., 0., 1.}, (blue + red) / 2},
  {{0., 0., 1.}, red},
  // -x
  // f5
  {{0., 0., 1.}, red},
  {{0., 1., 1.}, red},
  {{0., 1., 0.}, red},
  // f6
  {{0., 1., 0.}, red},
  {{0., 0., 0.}, red},
  {{0., 0., 1.}, red},
  // bottom
  // f7
  {{1., 1., 0.}, blue},
  {{1., 0., 0.}, blue},
  {{0., 0., 0.}, blue},
  // f8
  {{0., 0., 0.}, blue},
  {{0., 1., 0.}, blue},
  {{1., 1., 0.}, blue},
  // +x
  // f9
  {{1., 0., 0.}, green},
  {{1., 1., 0.}, green},
  {{1., 1., 1.}, green},
  // f10
  {{1., 1., 1.}, green},
  {{1., 0., 1.}, green},
  {{1., 0., 0.}, green},
  // +y
  // f11
  {{0., 1., 1.}, blue},
  {{1., 1., 1.}, (red+blue)/2},
  {{1., 1., 0.}, blue},
  // f12
  {{1., 1., 0.}, blue},
  {{0., 1., 0.}, (2 * green+blue)/3},
  {{0., 1., 1.}, blue},
};
// clang-format on

struct State {
  SDL_GPUBuffer *buf_vertex;
  SDL_GPUSampleCount sample_count;
};

static State state;

SDL_GPUTextureFormat getSupportedDepthFormat(const candlewick::Device &device) {
  if (SDL_GPUTextureSupportsFormat(
          device, SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
          SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
    return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
  } else if (SDL_GPUTextureSupportsFormat(
                 device, SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
                 SDL_GPU_TEXTURETYPE_2D,
                 SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
    return SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
  } else {
    return SDL_GPUTextureFormat(-1);
  }
};

SDL_GPUTexture *createDepthTexture(candlewick::Device &device,
                                   SDL_Window *window,
                                   SDL_GPUTextureFormat depth_tex_format) {
  Uint32 w, h;
  SDL_GetWindowSizeInPixels(window, (int *)&w, (int *)&h);
  SDL_GPUTextureCreateInfo texinfo{
      .type = SDL_GPU_TEXTURETYPE_2D,
      .format = depth_tex_format,
      .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
      .width = w,
      .height = h,
      .layer_count_or_depth = 1,
      .num_levels = 1,
      .sample_count = state.sample_count,
      .props = 0,
  };
  return SDL_CreateGPUTexture(device, &texinfo);
}

int main() {
  using namespace candlewick;
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return 1;
  }
  SDL_ShaderCross_Init();
  Device device{SDL_ShaderCross_GetSPIRVShaderFormats()};

  const float wWidth = 1280;
  const float wHeight = 720;
  const float aspectRatio = wWidth / wHeight;
  SDL_Window *window = SDL_CreateWindow(__FILE_NAME__, wWidth, wHeight, 0);
  if (!SDL_ClaimWindowForGPUDevice(device, window)) {
    SDL_Log("Error: %s", SDL_GetError());
    return 1;
  }

  // Shaders

  Shader vertexShader{device, "VertexColor.vert", 1};
  Shader fragmentShader{device, "VertexColor.frag", 0};

  // Buffers

  MeshLayout mesh_layout;
  mesh_layout.addBinding(0, sizeof(Vertex))
      .addAttribute(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
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
  SDL_GPUTransferBuffer *transfer_buffer =
      SDL_CreateGPUTransferBuffer(device, &transfer_buffer_desc);
  void *_map = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);
  SDL_memcpy(_map, vertexData, sizeof(vertexData));
  SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

  SDL_GPUCommandBuffer *cmdbuf;
  // Upload data to buffer
  {
    cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmdbuf);
    SDL_GPUTransferBufferLocation src_location;
    src_location.transfer_buffer = transfer_buffer;
    src_location.offset = 0;

    SDL_GPUBufferRegion dst_location;
    dst_location.buffer = state.buf_vertex;
    dst_location.offset = 0;
    dst_location.size = sizeof(vertexData);

    SDL_UploadToGPUBuffer(copy_pass, &src_location, &dst_location, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
  }
  SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);

  state.sample_count = SDL_GPU_SAMPLECOUNT_1;

  // Pipeline

  SDL_GPUColorTargetDescription color_desc;
  SDL_zero(color_desc);
  color_desc.format = SDL_GetGPUSwapchainTextureFormat(device, window);

  // SDL_GPUTextureFormat depth_stencil_format =
  // getSupportedDepthFormat(device);
  SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
  assert(int(depth_stencil_format) >= 0);

  SDL_GPUGraphicsPipelineCreateInfo pipeline_desc{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .vertex_input_state = mesh_layout.toVertexInputState(),
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .rasterizer_state =
          {
              .fill_mode = SDL_GPU_FILLMODE_FILL,
              .cull_mode = SDL_GPU_CULLMODE_NONE,
              .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
          },
      .depth_stencil_state{
          .compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
          .enable_depth_test = true,
          .enable_depth_write = true,
      },
      .target_info =
          {
              .color_target_descriptions = &color_desc,
              .num_color_targets = 1,
              .depth_stencil_format = depth_stencil_format,
              .has_depth_stencil_target = true,
          },
      .props = 0};
  pipeline_desc.rasterizer_state.front_face =
      SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
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

  // Depth texture
  SDL_GPUTexture *texture_depth =
      createDepthTexture(device, window, depth_stencil_format);

  using Eigen::Matrix4f;
  const float fov_rad = 45.0_radf;
  Matrix4f perp =
      orthographicMatrix({aspectRatio * fov_rad, fov_rad}, 0.1, 10.);
  Matrix4f modelMat = Matrix4f::Identity();
  modelMat.col(3).head<3>() << -0.5f, -0.5f, -0.5f;
  Matrix4f view;
  Matrix4f projViewMat;

  Uint32 frame = 0;

  // Main loop
  for (Uint32 i = 0; i < 200; i++) {
    SDL_GPURenderPass *render_pass;
    SDL_GPUBufferBinding vertex_binding;
    cmdbuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUTexture *swapchain;
    vertex_binding.buffer = state.buf_vertex;
    vertex_binding.offset = 0;
    const Float3 center{0., 0., 0.};
    Float3 eye{0., 0., 0.};
    // start at phi -> eye.x = 2.5, eye.y = 0.5
    const float phi = 0.05 * frame;
    eye.x() += 2.0 * std::cos(phi);
    eye.y() += 2.0 * std::sin(phi);
    view = lookAt(eye, center, {0., 0., 1.});
    projViewMat = perp * view * modelMat;

    if (SDL_AcquireGPUSwapchainTexture(cmdbuf, window, &swapchain, NULL,
                                       NULL)) {
      SDL_GPUColorTargetInfo ctinfo{.texture = swapchain,
                                    .clear_color = SDL_FColor{},
                                    .load_op = SDL_GPU_LOADOP_CLEAR,
                                    .store_op = SDL_GPU_STOREOP_STORE};
      SDL_GPUDepthStencilTargetInfo depth_target;
      SDL_zero(depth_target);
      depth_target.clear_depth = 1.0f;
      depth_target.load_op = SDL_GPU_LOADOP_CLEAR;
      depth_target.store_op = SDL_GPU_STOREOP_DONT_CARE;
      depth_target.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
      depth_target.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
      depth_target.texture = texture_depth;
      depth_target.cycle = true;
      render_pass = SDL_BeginGPURenderPass(cmdbuf, &ctinfo, 1, &depth_target);
      SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
      SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);
      constexpr auto vertexCount = std::size(vertexData);
      SDL_PushGPUVertexUniformData(cmdbuf, 0, projViewMat.data(),
                                   sizeof(projViewMat));
      SDL_DrawGPUPrimitives(render_pass, vertexCount, 1, 0, 0);
      SDL_EndGPURenderPass(render_pass);
    } else {
      SDL_Log("Failed to acquire swapchain: %s", SDL_GetError());
      break;
    }
    SDL_SubmitGPUCommandBuffer(cmdbuf);
    SDL_Log("Frame [%3d] phi = %.2f", frame, phi);
    frame++;
  }

  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);

  SDL_ReleaseGPUTexture(device, texture_depth);

  SDL_ReleaseGPUBuffer(device, state.buf_vertex);

  SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyWindow(window);
  device.destroy();
  SDL_ShaderCross_Quit();
  SDL_Quit();
}
