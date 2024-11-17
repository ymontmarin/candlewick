#include "candlewick/core/Device.h"
#include "candlewick/core/Mesh.h"
#include "candlewick/core/Shader.h"
#include "candlewick/core/math_util.h"
#include "candlewick/core/CopyBuffer.h"
#include "candlewick/utils/MeshData.h"
#include "candlewick/utils/load_mesh.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_filesystem.h>

#include <Eigen/Geometry>

using namespace candlewick;
using Eigen::Matrix3f;
using Eigen::Matrix4f;

// const char *meshFilename = "assets/meshes/mammoth.obj";
// const char *meshFilename = "assets/meshes/Stanford_Bunny.stl";
// const char *meshFilename = "assets/meshes/cube.obj";
const char *meshFilename = "assets/meshes/teapot.obj";

const float wWidth = 1280;
const float wHeight = 720;
const float aspectRatio = wWidth / wHeight;

static Device device{NoInit};
static SDL_Window *window;

static bool ExampleInit() {
  if (!SDL_Init(SDL_INIT_VIDEO))
    return false;
  device = Device{SDL_GPU_SHADERFORMAT_SPIRV, true};

  window = SDL_CreateWindow(__FILE_NAME__, wWidth, wHeight, 0);
  if (!SDL_ClaimWindowForGPUDevice(device, window)) {
    SDL_Log("Error %s", SDL_GetError());
    return false;
  }

  return true;
}

static void ExampleTeardown() {
  SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyWindow(window);
  device.destroy();
  SDL_Quit();
}

SDL_GPUTexture *createDepthTexture(const Device &device, SDL_Window *window,
                                   SDL_GPUTextureFormat depth_tex_format,
                                   SDL_GPUSampleCount sample_count) {
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
      .sample_count = sample_count,
      .props = 0,
  };
  return SDL_CreateGPUTexture(device, &texinfo);
}

int main() {
  if (!ExampleInit()) {
    return 1;
  }

  const char *basePath = SDL_GetBasePath();
  char meshPath[256];
  SDL_snprintf(meshPath, 256, "%s../../../%s", basePath, meshFilename);

  std::vector<MeshData> meshDatas;
  LoadMeshReturn ret = loadSceneMeshes(meshPath, meshDatas);
  if (ret != LoadMeshReturn::OK) {
    SDL_Log("Failed to load mesh.");
    return 1;
  }
  SDL_Log("Loaded %zu MeshData objects.", meshDatas.size());
  for (size_t i = 0; i < meshDatas.size(); i++) {
    SDL_Log("Mesh %zu: %zu vertices, %zu indices", i,
            meshDatas[i].numVertices(), meshDatas[i].numIndices());
  }

  std::vector<Mesh> meshes;
  for (std::size_t j = 0; j < meshDatas.size(); j++) {
    // Mesh mesh = convertToMesh(device, meshDatas[j]);
    Mesh mesh = convertToMeshIndexed(device, meshDatas[j]);
    meshes.push_back(std::move(mesh));
  }
  SDL_assert(meshDatas[0].numIndices() == meshes[0].count);

  /** COPY DATA TO GPU **/

  SDL_Log("Uploading mesh...");
  uploadMesh(device, meshes[0], meshDatas[0]);

  /** CREATE PIPELINE **/
  Shader vertexShader{device, "VertexNormal.vert", 2};
  Shader fragmentShader{device, "VertexNormal.frag", 0};

  SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
  SDL_GPUTexture *depthTexture = createDepthTexture(
      device, window, depth_stencil_format, SDL_GPU_SAMPLECOUNT_1);

  SDL_GPUColorTargetDescription colorTarget;
  SDL_zero(colorTarget);
  colorTarget.format = SDL_GetGPUSwapchainTextureFormat(device, window);
  SDL_GPUDepthStencilTargetInfo depthTarget;
  SDL_zero(depthTarget);
  depthTarget.clear_depth = 1.0;
  depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
  depthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;
  depthTarget.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
  depthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
  depthTarget.texture = depthTexture;
  depthTarget.cycle = true;

  // create pipeline
  SDL_GPUGraphicsPipelineCreateInfo pipeline_desc{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .vertex_input_state = meshes[0].layout().toVertexInputState(),
      .primitive_type = meshes[0].layout().primitiveType(),
      .rasterizer_state{.fill_mode = SDL_GPU_FILLMODE_FILL,
                        .cull_mode = SDL_GPU_CULLMODE_BACK,
                        .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE},
      .depth_stencil_state{.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
                           .enable_depth_test = true,
                           .enable_depth_write = true},
      .target_info{.color_target_descriptions = &colorTarget,
                   .num_color_targets = 1,
                   .depth_stencil_format = depth_stencil_format,
                   .has_depth_stencil_target = true},
      .props = 0,
  };
  SDL_Log("Creating pipeline...");
  SDL_GPUGraphicsPipeline *pipeline =
      SDL_CreateGPUGraphicsPipeline(device, &pipeline_desc);
  if (pipeline == NULL) {
    SDL_Log("Failed to create pipeline: %s", SDL_GetError());
    return 1;
  }

  vertexShader.release();
  fragmentShader.release();

  SDL_GPUCommandBuffer *command_buffer;
  SDL_GPUTexture *swapchain;

  const float fov = 55.0_radf;
  Matrix4f projectionMat =
      orthographicMatrix({aspectRatio * fov, fov}, 0.1, 10.0);

  Uint32 frameNo = 0;
  for (Uint32 i = 0; i < 200; i++) {
    Eigen::Affine3f modelMat = Eigen::Affine3f::Identity();
    Float3 eye{0., 0., 0.2};
    float phi = i * 0.02;
    float radius = 3.5;
    eye.x() = radius * std::cos(phi);
    eye.y() = radius * std::sin(phi);
    Float3 center = Float3::Zero();
    Matrix4f viewMat = lookAt(eye, center, {0., 0., 1.});
    // model-view matrix
    const Matrix4f modelView = viewMat * modelMat.matrix();
    // MVP matrix
    const Matrix4f projViewMat = projectionMat * modelView;
    const Matrix3f normalMatrix =
        modelView.topLeftCorner<3, 3>().inverse().transpose();

    // render pass

    SDL_GPURenderPass *render_pass;

    command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_Log("Frame [%u]", frameNo);

    if (!SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain,
                                        NULL, NULL)) {
      SDL_Log("Failed to acquire swapchain: %s", SDL_GetError());
      break;
    } else {

      SDL_GPUColorTargetInfo ctinfo{
          .texture = swapchain,
          .clear_color = SDL_FColor{0., 0., 0., 0.},
          .load_op = SDL_GPU_LOADOP_CLEAR,
          .store_op = SDL_GPU_STOREOP_STORE,
          .cycle = false,
      };
      SDL_GPUBufferBinding vertex_binding{
          .buffer = meshes[0].vertexBuffers[0],
          .offset = meshes[0].vertexBufferOffsets[0],
      };
      SDL_GPUBufferBinding index_binding{
          .buffer = meshes[0].indexBuffer,
          .offset = meshes[0].indexBufferOffset,
      };
      render_pass =
          SDL_BeginGPURenderPass(command_buffer, &ctinfo, 1, &depthTarget);
      SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
      SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);
      SDL_BindGPUIndexBuffer(render_pass, &index_binding,
                             SDL_GPU_INDEXELEMENTSIZE_32BIT);

      struct uniform_t {
        Matrix4f mvp;
        Matrix3f normalMatrix;
      } uniformData{projViewMat, normalMatrix};

      SDL_PushGPUVertexUniformData(command_buffer, 0, &uniformData,
                                   sizeof(uniformData));
      SDL_DrawGPUIndexedPrimitives(render_pass, meshes[0].count, 1, 0, 0, 0);
      // SDL_DrawGPUPrimitives(render_pass, meshes[0].count, 1, 0, 0);

      SDL_EndGPURenderPass(render_pass);
    }

    SDL_SubmitGPUCommandBuffer(command_buffer);
    frameNo++;
  }

  for (auto &mesh : meshes) {
    mesh.releaseOwnedBuffers(device);
  }
  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);

  ExampleTeardown();
  return 0;
}
