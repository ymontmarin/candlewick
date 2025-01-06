#include "Common.h"

#include "candlewick/core/MeshGroup.h"
#include "candlewick/core/Shader.h"
#include "candlewick/core/Shape.h"
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

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

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

#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Config.hpp>

namespace pin = pinocchio;
using namespace candlewick;
using Eigen::Matrix3f;
using Eigen::Matrix4f;

/// Application constants

constexpr Uint32 wWidth = 1600;
constexpr Uint32 wHeight = 900;
constexpr float aspectRatio = float(wWidth) / float(wHeight);

/// Application state

static bool renderPlane = true;
static bool renderGrid = true;
static Matrix4f viewMat;
static Matrix4f projectionMat;
static Radf currentFov = 55.0_radf;
static bool quitRequested = false;

static float pixelDensity;
static float displayScale;

static Mesh gridMesh{NoInit};

static Context ctx;

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

struct alignas(16) light_ubo_t {
  DirectionalLightUniform a;
  GpuVec3 viewPos;
};

void updateFov(Radf newFov) {
  currentFov = newFov;
  projectionMat = perspectiveFromFov(currentFov, aspectRatio, 0.01f, 10.0f);
}

void eventLoop() {
  // update pixel density and display scale
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
      updateFov(Radf(std::min(currentFov * scaleFac, 170.0_radf)));
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

  ImGui::SetNextWindowSize({0, 0}, ImGuiCond_Once);
  ImGui::Begin("Camera", nullptr);
  Degf newFov{currentFov};
  ImGui::DragFloat("cam_fov", (float *)(newFov), 1.f, minFov, maxFov, "%.3f",
                   ImGuiSliderFlags_AlwaysClamp);
  updateFov(Radf(newFov));
  projectionMat = perspectiveFromFov(currentFov, aspectRatio, 0.01f, 10.0f);
  ImGui::Checkbox("Render plane", &renderPlane);
  ImGui::Checkbox("Render grid", &renderGrid);
  ImGui::SeparatorText("light");
  ImGui::DragFloat("intens.", &myLight.intensity, 0.1f, 0.1f, 10.0f);
  ImGui::ColorEdit3("color", myLight.color.data());
  ImGui::End();
  ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
  ImGui::ShowDemoWindow(&demo_window_open);
}

void drawGrid(SDL_GPUCommandBuffer *command_buffer,
              SDL_GPURenderPass *render_pass, Matrix4f projViewMat) {
  GpuMat4 mvp{projViewMat};
  SDL_PushGPUVertexUniformData(command_buffer, 0, &mvp, sizeof(mvp));
  GpuVec4 gridColor = 0xE0A236ff_rgbaf;
  SDL_PushGPUFragmentUniformData(command_buffer, 0, &gridColor,
                                 sizeof(gridColor));
  auto vertex_binding = gridMesh.getVertexBinding(0);
  auto index_binding = gridMesh.getIndexBinding();

  SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);
  SDL_BindGPUIndexBuffer(render_pass, &index_binding,
                         SDL_GPU_INDEXELEMENTSIZE_32BIT);
  SDL_DrawGPUIndexedPrimitives(render_pass, gridMesh.count, 1, 0, 0, 0);
}

bool GuiInit() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.IniFilename = nullptr;

  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForOther(ctx.window);
  ImGui_ImplSDLGPU_InitInfo imguiInfo{
      .GpuDevice = ctx.device,
      .ColorTargetFormat =
          SDL_GetGPUSwapchainTextureFormat(ctx.device, ctx.window),
      .MSAASamples = SDL_GPU_SAMPLECOUNT_1,
  };
  return ImGui_ImplSDLGPU_Init(&imguiInfo);
}

void GuiTeardown() {
  ImGui_ImplSDL3_Shutdown();
  ImGui_ImplSDLGPU_Shutdown();
  ImGui::DestroyContext();
}

int main(int argc, char **argv) {
  CLI::App app{"Ur5 example"};
  bool performRecording{false};
  argv = app.ensure_utf8(argv);
  app.add_flag("-r,--record", performRecording, "Record output");
  CLI11_PARSE(app, argc, argv);
  if (!initExample(ctx, wWidth, wHeight)) {
    return 1;
  }
  Device &device = ctx.device;
  SDL_Window *window = ctx.window;

  if (!GuiInit()) {
    return 1;
  }

  pin::Model model;
  pin::GeometryModel geom_model;
  robot_descriptions::loadModelsFromToml("ur.toml", "ur5_gripper", model,
                                         &geom_model, NULL);
  pin::Data pin_data{model};
  pin::GeometryData geom_data{geom_model};

  std::vector<MeshData> meshDatas;
  std::vector<size_t> meshDataIdx = {0UL};
  for (size_t i = 0; i < geom_model.ngeoms; i++) {
    const auto &gobj = geom_model.geometryObjects[i];
    multibody::loadGeometryObject(gobj, meshDatas);
    meshDataIdx.push_back(meshDatas.size());
  }
  std::vector<ShapeData> robotShapes;
  robotShapes.reserve(geom_model.ngeoms);
  for (size_t i = 0; i < geom_model.ngeoms; i++) {
    std::span<MeshData> slice{meshDatas.begin() + meshDataIdx[i],
                              meshDatas.begin() + meshDataIdx[i + 1]};
    /// Create group of meshes, upload to device
    MeshGroup mg = createMeshGroup(device, slice, true);
    std::vector<PbrMaterialData> materials;
    for (auto &md : slice) {
      materials.push_back(md.material);
    }
    robotShapes.push_back({std::move(mg), std::move(materials)});
  }

  SDL_Log("Loaded %zu MeshData objects.", meshDatas.size());
  for (size_t i = 0; i < meshDatas.size(); i++) {
    SDL_Log("Mesh %zu: %zu vertices, %zu indices", i,
            meshDatas[i].numVertices(), meshDatas[i].numIndices());
  }
  SDL_Log("Created %zu robot mesh groups.", robotShapes.size());

  // Load plane
  MeshData plane_data = loadPlaneTiled(0.25f, 5, 5);
  Mesh plane = convertToMesh(device, plane_data);
  uploadMeshToDevice(device, plane, plane_data);

  // Load grid
  MeshData grid_data = loadGrid(10);
  gridMesh = convertToMesh(device, grid_data);
  uploadMeshToDevice(device, gridMesh, grid_data);

  /** CREATE PIPELINES **/
  SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
  SDL_GPUTexture *depthTexture = createDepthTexture(
      device, window, depth_stencil_format, SDL_GPU_SAMPLECOUNT_1);
  // Robot mesh pipeline
  SDL_GPUGraphicsPipeline *mesh_pipeline = nullptr;
  const auto swapchain_format =
      SDL_GetGPUSwapchainTextureFormat(device, window);
  {
    Shader vertexShader{device, "PbrBasic.vert", 1};
    Shader fragmentShader{device, "PbrBasic.frag", 2};

    SDL_GPUColorTargetDescription colorTarget;
    colorTarget.format = swapchain_format;
    SDL_zero(colorTarget.blend_state);

    // create pipeline
    SDL_GPUGraphicsPipelineCreateInfo mesh_pipeline_desc{
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state =
            robotShapes[0].meshes[0].layout().toVertexInputState(),
        .primitive_type = meshDatas[0].primitiveType,
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
    if (mesh_pipeline == nullptr) {
      SDL_Log("Failed to create pipeline: %s", SDL_GetError());
      return 1;
    }

    vertexShader.release();
    fragmentShader.release();
  }

  initGridPipeline(ctx, gridMesh.layout(), depth_stencil_format);

  // MAIN APPLICATION LOOP

  Uint32 frameNo = 0;

  projectionMat = perspectiveFromFov(currentFov, aspectRatio, 0.01f, 10.0f);
  viewMat = lookAt({2.0, 0, 2.}, Float3::Zero());

  Eigen::VectorXd q0 = pin::neutral(model);
  Eigen::VectorXd q1 = pin::randomConfiguration(model);

  media::VideoRecorder recorder{NoInit};
  if (performRecording)
    ::new (&recorder) media::VideoRecorder{wWidth, wHeight, "ur5.mp4"};

  auto record_callback = [=, &recorder](const auto &swapchain,
                                        auto swapchain_format) {
    media::videoWriteTextureToFrame(device, recorder, swapchain,
                                    swapchain_format, wWidth, wHeight);
  };

  while (frameNo < 5000 && !quitRequested) {
    // logic
    eventLoop();
    double phi = 0.5 * (1. + std::sin(frameNo * 1e-2));
    Eigen::VectorXd q = pin::interpolate(model, q0, q1, phi);
    pin::forwardKinematics(model, pin_data, q);
    pin::updateGeometryPlacements(model, pin_data, geom_model, geom_data);

    ImGui_ImplSDLGPU_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    drawMyImguiMenu();
    ImGui::Render();
    ImDrawData *draw_data = ImGui::GetDrawData();

    // render pass

    SDL_GPUCommandBuffer *command_buffer;
    SDL_GPUTexture *swapchain;
    SDL_GPURenderPass *render_pass;

    command_buffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain, nullptr,
                                   nullptr);

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

      SDL_assert(robotShapes.size() == geom_model.ngeoms);

      Matrix4f modelView;
      Matrix4f projViewMat;

      const light_ubo_t lightUbo{myLight, cameraViewPos(viewMat)};

      SDL_BindGPUGraphicsPipeline(render_pass, mesh_pipeline);

      // loop over mesh groups
      for (size_t i = 0; i < geom_model.ngeoms; i++) {
        const pin::SE3 &placement = geom_data.oMg[i];
        const ShapeData &shape = robotShapes[i];

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

        for (size_t j = 0; j < shape.meshes.size(); j++) {
          const auto material = shape.materials[j].toUniform();
          SDL_PushGPUFragmentUniformData(command_buffer, 0, &material,
                                         sizeof(PbrMaterialUniform));
          SDL_GPUBufferBinding vertex_binding =
              shape.meshes[j].getVertexBinding(0);
          SDL_GPUBufferBinding index_binding =
              shape.meshes[j].getIndexBinding();
          SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);
          SDL_BindGPUIndexBuffer(render_pass, &index_binding,
                                 SDL_GPU_INDEXELEMENTSIZE_32BIT);
          SDL_DrawGPUIndexedPrimitives(render_pass, shape.meshes[j].count, 1, 0,
                                       0, 0);
        }
      }

      // RENDER PLANE
      if (renderPlane) {
        Eigen::Affine3f plane_transform{Eigen::UniformScaling<float>(3.0f)};
        modelView = viewMat * plane_transform.matrix();
        projViewMat = projectionMat * modelView;
        const Matrix3f normalMatrix =
            plane_transform.linear().inverse().transpose();
        TransformUniformData cameraUniform{plane_transform.matrix(),
                                           projViewMat, normalMatrix};
        const auto material = plane_data.material.toUniform();
        SDL_PushGPUVertexUniformData(command_buffer, 0, &cameraUniform,
                                     sizeof(cameraUniform));
        SDL_PushGPUFragmentUniformData(command_buffer, 1, &lightUbo,
                                       sizeof(lightUbo));
        SDL_PushGPUFragmentUniformData(command_buffer, 0, &material,
                                       sizeof(PbrMaterialUniform));

        SDL_GPUBufferBinding vertex_binding = plane.getVertexBinding(0);
        SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);

        if (plane.isIndexed()) {
          SDL_GPUBufferBinding index_binding = plane.getIndexBinding();
          SDL_BindGPUIndexBuffer(render_pass, &index_binding,
                                 SDL_GPU_INDEXELEMENTSIZE_32BIT);
          SDL_DrawGPUIndexedPrimitives(render_pass, plane.count, 1, 0, 0, 0);
        } else {
          SDL_DrawGPUPrimitives(render_pass, plane.count, 1, 0, 0);
        }
      }

      // render grid
      if (renderGrid) {
        SDL_BindGPUGraphicsPipeline(render_pass, ctx.hudElemPipeline);
        drawGrid(command_buffer, render_pass, projectionMat * viewMat);
      }

      SDL_EndGPURenderPass(render_pass);
    } else {
      SDL_Log("Failed to acquire swapchain: %s", SDL_GetError());
      break;
    }

    // TODO: fix init new render pass
    {
      SDL_GPUColorTargetInfo target_info = {
          .texture = swapchain,
          .mip_level = 0,
          .layer_or_depth_plane = 0,
          .clear_color = SDL_FColor{0., 0., 0., 0.},
          .load_op = SDL_GPU_LOADOP_LOAD,
          .store_op = SDL_GPU_STOREOP_STORE,
          .cycle = false,
      };
      Imgui_ImplSDLGPU_PrepareDrawData(draw_data, command_buffer);
      // Call AFTER preparing draw data (which creates a copy pass)
      render_pass =
          SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);
      ImGui_ImplSDLGPU_RenderDrawData(draw_data, command_buffer, render_pass);
      SDL_EndGPURenderPass(render_pass);
    }

    SDL_SubmitGPUCommandBuffer(command_buffer);

    if (performRecording) {
      record_callback(swapchain, swapchain_format);
    }
    frameNo++;
  }

  for (auto &shape : robotShapes) {
    releaseMeshGroup(device, shape.meshes);
  }
  SDL_ReleaseGPUGraphicsPipeline(device, mesh_pipeline);

  GuiTeardown();
  teardownExample(ctx);
  return 0;
}
