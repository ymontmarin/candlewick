#include "Common.h"

#include "candlewick/core/MeshGroup.h"
#include "candlewick/core/Shader.h"
#include "candlewick/core/matrix_util.h"
#include "candlewick/core/LightUniforms.h"
#include "candlewick/core/MaterialUniform.h"
#include "candlewick/utils/WriteTextureToImage.h"
#include "candlewick/utils/MeshData.h"
#include "candlewick/utils/LoadMesh.h"
#include "candlewick/utils/CameraControl.h"
#include "candlewick/multibody/LoadPinocchioGeometry.h"

#include "candlewick/primitives/Plane.h"
#include "candlewick/primitives/Grid.h"

#include "candlewick/gui/imgui_impl_sdl3_extend.h"
#include "candlewick/gui/imgui_sdl_gpu.h"

#include <robot_descriptions_cpp/robot_spec.hpp>
#include <robot_descriptions_cpp/robot_load.hpp>

#include <pinocchio/multibody/data.hpp>
#include <pinocchio/multibody/geometry.hpp>
#include <pinocchio/algorithm/kinematics.hpp>
#include <pinocchio/algorithm/joint-configuration.hpp>
#include <pinocchio/algorithm/geometry.hpp>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_gpu.h>

namespace pin = pinocchio;
using namespace candlewick;
using Eigen::Matrix3f;
using Eigen::Matrix4f;

const Uint32 wWidth = 1600;
const Uint32 wHeight = 900;
const float aspectRatio = float(wWidth) / float(wHeight);

static bool add_plane = true;
static bool add_grid = true;
static Matrix4f viewMat;
static Matrix4f projectionMat;
static Rad<float> fov = 55.0_radf;
static bool quitRequested = false;

static float pixelDensity;
static float displayScale;

static struct {
  MeshData data;
  Mesh mesh;
  Float4 color;
} gridMesh{
    .data = loadGrid(10),
    .mesh{NoInit},
    .color = 0xE0A236ff_rgbaf,
};

Context ctx;

struct alignas(16) TransformUniformData {
  GpuMat4 model;
  alignas(16) GpuMat4 mvp;
  alignas(16) GpuMat3 normalMatrix;
};

static DirectionalLightUniform myLight{
    .direction = {0., -1., -1.},
    .color = {1.0, 1.0, 1.0},
    .intensity = 4.0,
};

void updateFov(Rad<float> newFov) {
  fov = newFov;
  projectionMat = perspectiveFromFov(fov, aspectRatio, 0.01f, 10.0f);
}

void eventLoop() {
  pixelDensity = SDL_GetWindowPixelDensity(ctx.window);
  displayScale = SDL_GetWindowDisplayScale(ctx.window);
  const float rotSensitivity = 5e-3f * pixelDensity;
  const float panSensitivity = 1e-2f * pixelDensity;
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    ImGuiIO &io = ImGui::GetIO();
    if (event.type == SDL_EVENT_QUIT) {
      SDL_Log("Application exit requested.");
      quitRequested = true;
      break;
    }

    if (io.WantCaptureMouse | io.WantCaptureKeyboard)
      continue;
    switch (event.type) {
    case SDL_EVENT_MOUSE_WHEEL: {
      float wy = event.wheel.y;
      const float scaleFac = std::exp(kScrollZoom * wy);
      // orthographicZoom(projectionMat, scaleFac);
      updateFov(Radf(std::min(fov * scaleFac, 170.0_radf)));
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      const float step_size = 0.06f;
      switch (event.key.key) {
      case SDLK_LEFT:
        cameraLocalTranslateX(viewMat, +step_size);
        break;
      case SDLK_RIGHT:
        cameraLocalTranslateX(viewMat, -step_size);
        break;
      case SDLK_UP:
        cameraWorldTranslateZ(viewMat, -step_size);
        break;
      case SDLK_DOWN:
        cameraWorldTranslateZ(viewMat, +step_size);
        break;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      SDL_MouseButtonFlags mouseButton = event.motion.state;
      bool controlPressed = SDL_GetModState() & SDL_KMOD_CTRL;
      if (mouseButton & SDL_BUTTON_LMASK) {
        if (controlPressed) {
          cylinderCameraMoveInOut(viewMat, 0.95f, event.motion.yrel);
        } else {
          cylinderCameraViewportDrag(
              viewMat, Float2{event.motion.xrel, event.motion.yrel},
              rotSensitivity, panSensitivity);
        }
      }
      if (mouseButton & SDL_BUTTON_RMASK) {
        float camXLocRotSpeed = 0.01f * pixelDensity;
        cameraLocalRotateX(viewMat, camXLocRotSpeed * event.motion.yrel);
      }
      break;
    }
    }
  }
}

void drawMyImguiMenu() {
  static bool demo_window_open = true;
  const float minFov = 15.f;
  const float maxFov = 90.f;

  ImGui::Begin("Camera", nullptr);
  Deg<float> _fovdeg{fov};
  ImGui::DragFloat("cam_fov", (float *)(_fovdeg), 1.f, minFov, maxFov, "%.3f",
                   ImGuiSliderFlags_AlwaysClamp);
  updateFov(Radf(_fovdeg));
  projectionMat = perspectiveFromFov(fov, aspectRatio, 0.01f, 10.0f);
  ImGui::Checkbox("Render plane", &add_plane);
  ImGui::Checkbox("Render grid", &add_grid);
  ImGui::End();
  ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
  ImGui::ShowDemoWindow(&demo_window_open);
}

void drawGrid(SDL_GPUCommandBuffer *command_buffer,
              SDL_GPURenderPass *render_pass, Matrix4f projViewMat) {
  struct {
    GpuMat4 mvp;
  } cameraUniform{.mvp = projViewMat};
  SDL_PushGPUVertexUniformData(command_buffer, 0, &cameraUniform,
                               sizeof(cameraUniform));
  struct {
    alignas(16) GpuVec4 color;
  } colorUniform{gridMesh.color};
  static_assert(IsVertexType<decltype(colorUniform)>, "");
  SDL_PushGPUFragmentUniformData(command_buffer, 0, &colorUniform,
                                 sizeof(colorUniform));
  auto vertex_binding = gridMesh.mesh.getVertexBinding(0);
  auto index_binding = gridMesh.mesh.getIndexBinding();

  SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);
  SDL_BindGPUIndexBuffer(render_pass, &index_binding,
                         SDL_GPU_INDEXELEMENTSIZE_32BIT);
  SDL_DrawGPUIndexedPrimitives(render_pass, gridMesh.mesh.count, 1, 0, 0, 0);
}

bool GuiInit() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.IniFilename = nullptr;

  ImGui::StyleColorsDark();
  if (!ImGui_ImplSDL3_InitForSDLGPU3(ctx.window, ctx.device)) {
    SDL_Log("Failed to init ImGui for SDL");
    return false;
  }
  return ImGui_ImplSDLGPU3_Init(ctx.device, ctx.window);
}

void GuiTeardown() {
  ImGui_ImplSDLGPU3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

int main() {
  if (!ExampleInit(ctx, wWidth, wHeight)) {
    return 1;
  }
  Device &device = ctx.device;
  SDL_Window *window = ctx.window;

  if (!GuiInit()) {
    return 1;
  }

  pin::Model model;
  pin::GeometryModel geom_model;
  auto robot_spec =
      robot_descriptions::loadRobotSpecFromToml("ur.toml", "ur5_gripper");
  robot_descriptions::loadModelFromSpec(robot_spec, model);
  robot_descriptions::loadGeomFromSpec(robot_spec, model, geom_model,
                                       pin::VISUAL);
  pin::Data pin_data{model};
  pin::GeometryData geom_data{geom_model};

  std::vector<MeshData> meshDatas;
  std::vector<MeshGroup> meshGroups;
  meshGroups.reserve(geom_model.ngeoms);
  std::vector<size_t> meshDataIdx = {0UL};
  for (size_t i = 0; i < geom_model.ngeoms; i++) {
    const auto &gobj = geom_model.geometryObjects[i];
    multibody::loadGeometryObject(gobj, meshDatas);
    meshDataIdx.push_back(meshDatas.size());
  }
  for (size_t i = 0; i < geom_model.ngeoms; i++) {
    std::span<MeshData> slice{meshDatas.begin() + meshDataIdx[i],
                              meshDatas.begin() + meshDataIdx[i + 1]};
    meshGroups.emplace_back(device, slice);
    uploadMeshGroupToDevice(device, meshGroups[i]);
  }

  SDL_Log("Loaded %zu MeshData objects.", meshDatas.size());
  for (size_t i = 0; i < meshDatas.size(); i++) {
    SDL_Log("Mesh %zu: %zu vertices, %zu indices", i,
            meshDatas[i].numVertices(), meshDatas[i].numIndices());
  }
  SDL_Log("Created %zu mesh groups.", meshGroups.size());

  // Load plane
  struct {
    MeshData data;
    Mesh mesh;
  } plane{loadPlaneTiled(0.25f, 5, 5), Mesh{NoInit}};
  plane.mesh = convertToMesh(device, plane.data);
  uploadMeshToDevice(device, plane.mesh, plane.data);

  // Load grid
  gridMesh.mesh = convertToMesh(device, gridMesh.data);
  uploadMeshToDevice(device, gridMesh.mesh, gridMesh.data);

  /** CREATE PIPELINES **/
  SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
  SDL_GPUTexture *depthTexture = createDepthTexture(
      device, window, depth_stencil_format, SDL_GPU_SAMPLECOUNT_1);
  SDL_GPUGraphicsPipeline *mesh_pipeline = NULL;
  {
    Shader vertexShader{device, "PbrBasic.vert", 1};
    Shader fragmentShader{device, "PbrBasic.frag", 2};

    SDL_GPUColorTargetDescription colorTarget{
        .format = SDL_GetGPUSwapchainTextureFormat(ctx.device, ctx.window)};
    SDL_zero(colorTarget.blend_state);
    SDL_Log("Mesh pipeline color target format: %d", colorTarget.format);

    // create pipeline
    SDL_GPUGraphicsPipelineCreateInfo mesh_pipeline_desc{
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state =
            meshGroups[0].meshes[0].layout().toVertexInputState(),
        .primitive_type = meshGroups[0].meshDatas[0].primitiveType,
        .depth_stencil_state{.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
                             .enable_depth_test = true,
                             .enable_depth_write = true},
        .target_info{.color_target_descriptions = &colorTarget,
                     .num_color_targets = 1,
                     .depth_stencil_format = depth_stencil_format,
                     .has_depth_stencil_target = true},
        .props = 0,
    };

    mesh_pipeline = SDL_CreateGPUGraphicsPipeline(device, &mesh_pipeline_desc);
    if (mesh_pipeline == NULL) {
      SDL_Log("Failed to create pipeline: %s", SDL_GetError());
      return 1;
    }

    vertexShader.release();
    fragmentShader.release();
  }

  initGridPipeline(ctx, gridMesh.mesh.layout(), depth_stencil_format);

  // MAIN APPLICATION LOOP

  Uint32 frameNo = 0;

  projectionMat = perspectiveFromFov(fov, aspectRatio, 0.01f, 10.0f);
  viewMat = lookAt({2.0, 0, 2.}, Float3::Zero());

  Eigen::VectorXd q0 = pin::neutral(model);
  Eigen::VectorXd q1 = pin::randomConfiguration(model);

  media::VideoRecorder recorder{wWidth, wHeight, "ur5.mp4"};

  while (frameNo < 5000 && !quitRequested) {
    // logic
    eventLoop();
    double phi = 0.5 * (1. + std::sin(frameNo * 1e-2));
    Eigen::VectorXd q = pin::interpolate(model, q0, q1, phi);
    pin::forwardKinematics(model, pin_data, q);
    pin::updateGeometryPlacements(model, pin_data, geom_model, geom_data);

    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    drawMyImguiMenu();
    ImGui::Render();

    // render pass

    SDL_GPUCommandBuffer *command_buffer;
    SDL_GPUTexture *swapchain;
    SDL_GPURenderPass *render_pass;

    command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain, NULL,
                                   NULL);
    // SDL_Log("Frame [%u]", frameNo);

    if (swapchain) {

      SDL_GPUColorTargetInfo ctinfo{
          .texture = swapchain,
          .clear_color = SDL_FColor{0., 0., 0., 0.},
          .load_op = SDL_GPU_LOADOP_CLEAR,
          .store_op = SDL_GPU_STOREOP_STORE,
          .cycle = false,
      };
      SDL_GPUDepthStencilTargetInfo depthTarget;
      SDL_zero(depthTarget);
      depthTarget.clear_depth = 1.0;
      depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
      depthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;
      depthTarget.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
      depthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
      depthTarget.texture = depthTexture;
      depthTarget.cycle = true;

      render_pass =
          SDL_BeginGPURenderPass(command_buffer, &ctinfo, 1, &depthTarget);

      SDL_assert(meshGroups.size() == geom_model.ngeoms);

      Matrix4f modelView;
      Matrix4f projViewMat;

      struct alignas(16) light_ubo_t {
        DirectionalLightUniform a;
        alignas(16) GpuVec3 viewPos;
      };
      const light_ubo_t lightUbo{myLight, cameraViewPos(viewMat)};

      SDL_BindGPUGraphicsPipeline(render_pass, mesh_pipeline);

      // loop over mesh groups
      for (size_t i = 0; i < geom_model.ngeoms; i++) {
        const pin::SE3 &placement = geom_data.oMg[i];
        const MeshGroup &mg = meshGroups[i];

        Matrix4f modelMat = placement.toHomogeneousMatrix().cast<float>();
        modelView = viewMat * modelMat.matrix();
        projViewMat = projectionMat * modelView;
        const Matrix3f normalMatrix =
            modelMat.topLeftCorner<3, 3>().inverse().transpose();
        TransformUniformData cameraUniform{
            modelMat,
            projViewMat,
            normalMatrix,
        };

        SDL_PushGPUVertexUniformData(command_buffer, 0, &cameraUniform,
                                     sizeof(cameraUniform));
        SDL_PushGPUFragmentUniformData(command_buffer, 1, &lightUbo,
                                       sizeof(lightUbo));

        for (size_t j = 0; j < mg.size(); j++) {
          const auto material = mg.meshDatas[j].material.toUniform();
          SDL_PushGPUFragmentUniformData(command_buffer, 0, &material,
                                         sizeof(PbrMaterialUniform));
          SDL_GPUBufferBinding vertex_binding{
              .buffer = mg.masterVertexBuffer,
              .offset = mg.meshes[j].vertexBufferOffsets[0],
          };
          SDL_GPUBufferBinding index_binding{
              .buffer = mg.masterIndexBuffer,
              .offset = mg.meshes[j].indexBufferOffset,
          };
          SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);
          SDL_BindGPUIndexBuffer(render_pass, &index_binding,
                                 SDL_GPU_INDEXELEMENTSIZE_32BIT);
          SDL_DrawGPUIndexedPrimitives(render_pass, mg.meshes[j].count, 1, 0, 0,
                                       0);
        }
      }

      // RENDER PLANE
      if (add_plane) {
        Eigen::Affine3f plane_transform{Eigen::UniformScaling<float>(3.0f)};
        modelView = viewMat * plane_transform.matrix();
        projViewMat = projectionMat * modelView;
        const Matrix3f normalMatrix =
            plane_transform.linear().inverse().transpose();
        TransformUniformData cameraUniform{plane_transform.matrix(),
                                           projViewMat, normalMatrix};
        const auto material = plane.data.material.toUniform();
        SDL_PushGPUVertexUniformData(command_buffer, 0, &cameraUniform,
                                     sizeof(cameraUniform));
        SDL_PushGPUFragmentUniformData(command_buffer, 1, &lightUbo,
                                       sizeof(lightUbo));
        SDL_PushGPUFragmentUniformData(command_buffer, 0, &material,
                                       sizeof(PbrMaterialUniform));

        SDL_GPUBufferBinding vertex_binding = plane.mesh.getVertexBinding(0);
        SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);

        if (plane.mesh.isIndexed()) {
          SDL_GPUBufferBinding index_binding = plane.mesh.getIndexBinding();
          SDL_BindGPUIndexBuffer(render_pass, &index_binding,
                                 SDL_GPU_INDEXELEMENTSIZE_32BIT);
          SDL_DrawGPUIndexedPrimitives(render_pass, plane.mesh.count, 1, 0, 0,
                                       0);
        } else {
          SDL_DrawGPUPrimitives(render_pass, plane.mesh.count, 1, 0, 0);
        }
      }

      // render grid
      if (add_grid) {
        SDL_BindGPUGraphicsPipeline(render_pass, ctx.hudEltPipeline);
        drawGrid(command_buffer, render_pass, projectionMat * viewMat);
      }

      SDL_EndGPURenderPass(render_pass);
    } else {
      SDL_Log("Failed to acquire swapchain: %s", SDL_GetError());
      break;
    }

    ImGui_ImplSDLGPU3_RenderDrawData(ImGui::GetDrawData(), command_buffer,
                                     swapchain);

    SDL_SubmitGPUCommandBuffer(command_buffer);

    auto swapchainFormat = SDL_GetGPUSwapchainTextureFormat(device, window);
    media::videoWriteTextureToFrame(device, recorder, swapchain,
                                    swapchainFormat, wWidth, wHeight);

    frameNo++;
  }

  for (auto &mg : meshGroups) {
    mg.releaseBuffers(device);
  }
  SDL_ReleaseGPUGraphicsPipeline(device, mesh_pipeline);

  GuiTeardown();
  ExampleTeardown(ctx);
  return 0;
}
