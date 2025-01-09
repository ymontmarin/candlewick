#include "Common.h"

#include "candlewick/core/Renderer.h"
#include "candlewick/core/GuiSystem.h"
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

#include <robot_descriptions_cpp/robot_load.hpp>

#include <pinocchio/multibody/data.hpp>
#include <pinocchio/multibody/geometry.hpp>
#include <pinocchio/algorithm/kinematics.hpp>
#include <pinocchio/algorithm/joint-configuration.hpp>
#include <pinocchio/algorithm/geometry.hpp>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_init.h>
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

void eventLoop(const Renderer &renderer) {
  // update pixel density and display scale
  pixelDensity = SDL_GetWindowPixelDensity(renderer.window);
  displayScale = SDL_GetWindowDisplayScale(renderer.window);
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

Renderer createRenderer(Uint32 width, Uint32 height,
                        SDL_GPUTextureFormat depth_stencil_format) {
  Device dev{SDL_GPU_SHADERFORMAT_SPIRV, true};
  SDL_Window *window = SDL_CreateWindow(__FILE__, int(width), int(height), 0);
  return Renderer{std::move(dev), window, depth_stencil_format};
}

int main(int argc, char **argv) {
  SDL_GPUGraphicsPipeline *debugLinePipeline;
  SDL_GPUGraphicsPipeline *meshPipeline;

  CLI::App app{"Ur5 example"};
  bool performRecording{false};
  argv = app.ensure_utf8(argv);
  app.add_flag("-r,--record", performRecording, "Record output");
  CLI11_PARSE(app, argc, argv);

  if (!SDL_Init(SDL_INIT_VIDEO))
    return 1;

  SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
  Renderer renderer = createRenderer(wWidth, wHeight, depth_stencil_format);
  Device &device = renderer.device;

  GuiSystem guiSys{[](Renderer &) {
    static bool demo_window_open = true;
    const float minFov = 15.f;
    const float maxFov = 90.f;

    ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
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
  }};

  if (!guiSys.init(renderer)) {
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

  std::vector<Shape> robotShapes;
  for (size_t i = 0; i < geom_model.ngeoms; i++) {
    std::span<MeshData> slice{meshDatas.begin() + meshDataIdx[i],
                              meshDatas.begin() + meshDataIdx[i + 1]};
    /// Create group of meshes, upload to device
    auto shape = Shape::createShapeFromDatas(device, slice, true);
    robotShapes.push_back(std::move(shape));
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
  // Robot mesh pipeline
  const auto swapchain_format =
      SDL_GetGPUSwapchainTextureFormat(device, renderer.window);
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
        .vertex_input_state = robotShapes[0].layout().toVertexInputState(),
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

    meshPipeline = SDL_CreateGPUGraphicsPipeline(device, &mesh_pipeline_desc);
    if (meshPipeline == nullptr) {
      SDL_Log("Failed to create pipeline: %s", SDL_GetError());
      return 1;
    }

    vertexShader.release();
    fragmentShader.release();
  }

  debugLinePipeline =
      initGridPipeline(renderer, gridMesh.layout(), depth_stencil_format);

  // MAIN APPLICATION LOOP

  Uint32 frameNo = 0;

  projectionMat = perspectiveFromFov(currentFov, aspectRatio, 0.01f, 10.0f);
  viewMat = lookAt({2.0, 0, 2.}, Float3::Zero());

  Eigen::VectorXd q0 = pin::neutral(model);
  Eigen::VectorXd q1 = pin::randomConfiguration(model);

  media::VideoRecorder recorder{NoInit};
  if (performRecording)
    ::new (&recorder) media::VideoRecorder{wWidth, wHeight, "ur5.mp4"};

  auto record_callback = [=, &renderer, &recorder](auto swapchain_format) {
    media::videoWriteTextureToFrame(renderer.device, recorder,
                                    renderer.swapchain, swapchain_format,
                                    wWidth, wHeight);
  };

  while (frameNo < 5000 && !quitRequested) {
    // logic
    eventLoop(renderer);
    double phi = 0.5 * (1. + std::sin(frameNo * 1e-2));
    Eigen::VectorXd q = pin::interpolate(model, q0, q1, phi);
    pin::forwardKinematics(model, pin_data, q);
    pin::updateGeometryPlacements(model, pin_data, geom_model, geom_data);

    // acquire command buffer and swapchain
    renderer.beginFrame();
    renderer.acquireSwapchain();
    SDL_GPUCommandBuffer *command_buffer = renderer.command_buffer;
    SDL_GPURenderPass *render_pass;

    if (renderer.swapchain) {

      SDL_GPUColorTargetInfo ctinfo{
          .texture = renderer.swapchain,
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
      depthTarget.texture = renderer.depth_texture;

      render_pass =
          SDL_BeginGPURenderPass(command_buffer, &ctinfo, 1, &depthTarget);

      SDL_assert(robotShapes.size() == geom_model.ngeoms);

      Matrix4f modelView;
      Matrix4f projViewMat;

      const light_ubo_t lightUbo{myLight, cameraViewPos(viewMat)};

      SDL_BindGPUGraphicsPipeline(render_pass, meshPipeline);

      // loop over mesh groups
      for (size_t i = 0; i < geom_model.ngeoms; i++) {
        const pin::SE3 &placement = geom_data.oMg[i];
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
        renderer.renderShape(render_pass, robotShapes[i]);
      }

      // RENDER PLANE
      if (renderPlane) {
        Eigen::Affine3f plane_transform{Eigen::UniformScaling<float>(3.0f)};
        modelView.noalias() = viewMat * plane_transform.matrix();
        projViewMat.noalias() = projectionMat * modelView;
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
        renderer.renderMesh(render_pass, plane);
      }

      // render grid
      if (renderGrid) {
        SDL_BindGPUGraphicsPipeline(render_pass, debugLinePipeline);
        GpuMat4 mvp{projViewMat};
        GpuVec4 gridColor = 0xE0A236ff_rgbaf;
        SDL_PushGPUVertexUniformData(command_buffer, 0, &mvp, sizeof(mvp));
        SDL_PushGPUFragmentUniformData(command_buffer, 0, &gridColor,
                                       sizeof(gridColor));
        renderer.renderMesh(render_pass, gridMesh);
      }

      SDL_EndGPURenderPass(render_pass);
    } else {
      SDL_Log("Failed to acquire swapchain: %s", SDL_GetError());
      break;
    }

    guiSys.render(renderer);

    renderer.endFrame();

    if (performRecording) {
      record_callback(swapchain_format);
    }
    frameNo++;
  }

  for (auto &shape : robotShapes) {
    shape.release();
  }
  SDL_ReleaseGPUGraphicsPipeline(device, meshPipeline);
  SDL_ReleaseGPUGraphicsPipeline(device, debugLinePipeline);

  guiSys.release();
  renderer.destroy();
  SDL_DestroyWindow(renderer.window);
  SDL_Quit();
  return 0;
}
