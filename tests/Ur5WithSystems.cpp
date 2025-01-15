#include "Common.h"

#include "candlewick/core/Renderer.h"
#include "candlewick/core/GuiSystem.h"
#include "candlewick/core/Shape.h"
#include "candlewick/core/matrix_util.h"
#include "candlewick/core/LightUniforms.h"
#include "candlewick/utils/WriteTextureToImage.h"
#include "candlewick/utils/MeshData.h"
#include "candlewick/utils/LoadMesh.h"
#include "candlewick/utils/CameraControl.h"
#include "candlewick/multibody/RobotScene.h"

#include "candlewick/primitives/Arrow.h"
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
using multibody::RobotScene;

/// Application constants

constexpr Uint32 wWidth = 1600;
constexpr Uint32 wHeight = 900;
constexpr float aspectRatio = float(wWidth) / float(wHeight);

/// Application state

static bool renderGrid = true;
static Camera camera;
static Radf currentFov = 55.0_radf;
static bool quitRequested = false;

static float pixelDensity;
static float displayScale;

static GpuVec4 gridColor = 0xE0A236ff_rgbaf;
static Mesh gridMesh{NoInit};

static DirectionalLight myLight{
    .direction = {0., -1., -1.},
    .color = {1.0, 1.0, 1.0},
    .intensity = 8.0,
};

void updateFov(Radf newFov) {
  currentFov = newFov;
  camera.projection = perspectiveFromFov(currentFov, aspectRatio, 0.01f, 10.0f);
}

void eventLoop(const Renderer &renderer) {
  auto viewMat = camera.viewMatrix();
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
  camera.setPoseFromView(viewMat);
}

Renderer createRenderer(Uint32 width, Uint32 height,
                        SDL_GPUTextureFormat depth_stencil_format) {
  Device dev{auto_detect_shader_format_subset(), true};
  SDL_Window *window = SDL_CreateWindow(__FILE__, int(width), int(height), 0);
  return Renderer{std::move(dev), window, depth_stencil_format};
}

int main(int argc, char **argv) {
  CLI::App app{"Ur5 example"};
  bool performRecording{false};
  argv = app.ensure_utf8(argv);
  app.add_flag("-r,--record", performRecording, "Record output");
  CLI11_PARSE(app, argc, argv);

  if (!SDL_Init(SDL_INIT_VIDEO))
    return 1;

  Renderer renderer =
      createRenderer(wWidth, wHeight, SDL_GPU_TEXTUREFORMAT_D24_UNORM);
  Device &device = renderer.device;

  // Load plane
  MeshData plane_data = loadPlaneTiled(0.25f, 5, 5);

  // Load grid
  MeshData grid_data = loadGrid(10);
  gridMesh = createMesh(device, grid_data);
  uploadMeshToDevice(device, gridMesh, grid_data);

  // Load robot
  pin::Model model;
  pin::GeometryModel geom_model;
  robot_descriptions::loadModelsFromToml("ur.toml", "ur5_gripper", model,
                                         &geom_model, NULL);
  pin::Data pin_data{model};
  pin::GeometryData geom_data{geom_model};

  RobotScene robot_scene{renderer, geom_model, geom_data, {}};
  const Eigen::Affine3f plane_transform{Eigen::UniformScaling<float>(3.0f)};
  auto &plane_obj = robot_scene.addEnvironmentObject(std::move(plane_data),
                                                     plane_transform.matrix());
  std::array triad_data = createTriad();
  std::vector<Mesh> triad_meshes;
  for (auto &&arrow_data : std::move(triad_data)) {
    Mesh arrow_mesh = createMesh(device, arrow_data);
    uploadMeshToDevice(device, arrow_mesh, arrow_data);
    triad_meshes.push_back(std::move(arrow_mesh));
  }
  auto &robotShapes = robot_scene.robotShapes;
  SDL_assert(robotShapes.size() == geom_model.ngeoms);
  SDL_Log("Created %zu robot mesh shapes.", robotShapes.size());

  GuiSystem guiSys{[&](Renderer &r) {
    static bool demo_window_open = true;
    const float minFov = 15.f;
    const float maxFov = 90.f;

    ImGui::Begin("Renderer info & controls", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Device driver: %s", r.device.driverName());
    ImGui::SeparatorText("Camera");
    Degf newFov{currentFov};
    ImGui::DragFloat("cam_fov", (float *)(newFov), 1.f, minFov, maxFov, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);
    updateFov(Radf(newFov));
    camera.projection =
        perspectiveFromFov(currentFov, aspectRatio, 0.01f, 10.0f);
    ImGui::Checkbox("Render plane", &plane_obj.status);
    ImGui::Checkbox("Render grid", &renderGrid);

    ImGui::SeparatorText("Lights");
    ImGui::DragFloat("intens.", &myLight.intensity, 0.1f, 0.1f, 10.0f);
    ImGui::ColorEdit3("color", myLight.color.data());
    ImGui::Separator();
    ImGui::ColorEdit4("grid color", gridColor.data(),
                      ImGuiColorEditFlags_AlphaPreview);
    ImGui::ColorEdit4("plane color", plane_data.material.baseColor.data());
    ImGui::End();
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    ImGui::ShowDemoWindow(&demo_window_open);
  }};

  if (!guiSys.init(renderer)) {
    return 1;
  }

  /** CREATE PIPELINES **/

  SDL_GPUGraphicsPipeline *debugLinePipeline =
      initGridPipeline(renderer.device, renderer.window, gridMesh.layout(),
                       renderer.depth_format, SDL_GPU_PRIMITIVETYPE_LINELIST);
  SDL_GPUGraphicsPipeline *debugTrianglePipeline = initGridPipeline(
      renderer.device, renderer.window, triad_meshes[0].layout(),
      renderer.depth_format, SDL_GPU_PRIMITIVETYPE_TRIANGLELIST);

  // MAIN APPLICATION LOOP

  Uint32 frameNo = 0;

  camera.projection = perspectiveFromFov(currentFov, aspectRatio, 0.01f, 10.0f);
  camera.setPoseFromView(lookAt({2.0, 0, 2.}, Float3::Zero()));

  Eigen::VectorXd q0 = pin::neutral(model);
  Eigen::VectorXd q1 = pin::randomConfiguration(model);

  media::VideoRecorder recorder{NoInit};
  if (performRecording)
    recorder = media::VideoRecorder{wWidth, wHeight, "ur5.mp4"};

  auto record_callback = [=, &renderer, &recorder]() {
    auto swapchain_format =
        SDL_GetGPUSwapchainTextureFormat(renderer.device, renderer.window);
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
    robot_scene.directionalLight = myLight;
    renderer.beginFrame();
    renderer.acquireSwapchain();

    if (renderer.swapchain) {

      robot_scene.render(renderer, camera, [&](auto *render_pass) {
        // render grid
        Mat4f projViewMat = camera.projection * camera.viewMatrix();
        const GpuMat4 mvp{projViewMat};
        renderer.pushVertexUniform(0, &mvp, sizeof(mvp));
        if (renderGrid) {
          SDL_BindGPUGraphicsPipeline(render_pass, debugLinePipeline);
          renderer.pushFragmentUniform(0, &gridColor, sizeof(gridColor));
          renderer.render(render_pass, gridMesh);

          SDL_BindGPUGraphicsPipeline(render_pass, debugTrianglePipeline);
          for (size_t i = 0; i < 3; i++) {
            Mesh &m = triad_meshes[i];
            GpuVec4 triad_col{triad_data[i].material.baseColor};
            renderer.pushFragmentUniform(0, &triad_col, sizeof(triad_col));
            renderer.render(render_pass, m);
          }
        }
      });

    } else {
      SDL_Log("Failed to acquire swapchain: %s", SDL_GetError());
      break;
    }

    guiSys.render(renderer);

    renderer.endFrame();

    if (performRecording) {
      record_callback();
    }
    frameNo++;
  }

  for (auto &shape : robotShapes) {
    shape.second.release();
  }
  SDL_ReleaseGPUGraphicsPipeline(device, debugLinePipeline);

  guiSys.release();
  renderer.destroy();
  SDL_DestroyWindow(renderer.window);
  SDL_Quit();
  return 0;
}
