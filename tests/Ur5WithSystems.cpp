#include "Common.h"

#include "candlewick/core/debug/DepthViz.h"
#include "candlewick/core/Renderer.h"
#include "candlewick/core/GuiSystem.h"
#include "candlewick/core/DebugScene.h"

#include "candlewick/core/math_util.h"
#include "candlewick/core/LightUniforms.h"
#include "candlewick/utils/WriteTextureToImage.h"
#include "candlewick/utils/CameraControl.h"
#include "candlewick/multibody/RobotScene.h"

#include "candlewick/primitives/Plane.h"

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

constexpr Uint32 wWidth = 1680;
constexpr Uint32 wHeight = 1050;
constexpr float aspectRatio = float(wWidth) / float(wHeight);

/// Application state

static Radf currentFov = 55.0_radf;
static float nearZ = 0.01f;
static float farZ = 10.f;
static float currentOrthoScale = 1.f;
static CameraProjection cam_type = CameraProjection::PERSPECTIVE;
static Camera camera{
    .projection = perspectiveFromFov(currentFov, aspectRatio, nearZ, farZ),
    .view{lookAt({2.0, 0, 2.}, Float3::Zero())},
};
static bool quitRequested = false;
static bool showDebugViz = false;

static float pixelDensity;
static float displayScale;

static DirectionalLight myLight{
    .direction = {0., -1., -1.},
    .color = {1.0, 1.0, 1.0},
    .intensity = 8.0,
};

static void updateFov(Radf newFov) {
  camera.projection = perspectiveFromFov(newFov, aspectRatio, nearZ, farZ);
  currentFov = newFov;
}

static void updateOrtho(float zoom) {
  float iz = 1.f / zoom;
  camera.projection = orthographicMatrix({iz * aspectRatio, iz}, -8., 8.);
  currentOrthoScale = zoom;
}

void eventLoop(const Renderer &renderer) {
  CylinderCameraControl camControl{camera};
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
      switch (cam_type) {
      case CameraProjection::ORTHOGRAPHIC:
        updateOrtho(std::clamp(scaleFac * currentOrthoScale, 0.1f, 2.f));
        break;
      case CameraProjection::PERSPECTIVE:
        updateFov(Radf(std::min(currentFov * scaleFac, 170.0_radf)));
        break;
      }
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      const float step_size = 0.06f;
      switch (event.key.key) {
      case SDLK_LEFT:
        cameraLocalTranslateX(camera, +step_size);
        break;
      case SDLK_RIGHT:
        cameraLocalTranslateX(camera, -step_size);
        break;
      case SDLK_UP:
        cameraWorldTranslateZ(camera, -step_size);
        break;
      case SDLK_DOWN:
        cameraWorldTranslateZ(camera, +step_size);
        break;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      SDL_MouseButtonFlags mouseButton = event.motion.state;
      bool controlPressed = SDL_GetModState() & SDL_KMOD_CTRL;
      if (mouseButton & SDL_BUTTON_LMASK) {
        if (controlPressed) {
          camControl.moveInOut(0.95f, event.motion.yrel);
        } else {
          camControl.viewportDrag({event.motion.xrel, event.motion.yrel},
                                  rotSensitivity, panSensitivity);
        }
      }
      if (mouseButton & SDL_BUTTON_RMASK) {
        float camXLocRotSpeed = 0.01f * pixelDensity;
        cameraLocalRotateX(camera, camXLocRotSpeed * event.motion.yrel);
      }
      break;
    }
    }
  }
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
      createRenderer(wWidth, wHeight, SDL_GPU_TEXTUREFORMAT_D32_FLOAT);

  // Load robot
  pin::Model model;
  pin::GeometryModel geom_model;
  robot_descriptions::loadModelsFromToml("ur.toml", "ur5_gripper", model,
                                         &geom_model, NULL);
  pin::Data pin_data{model};
  pin::GeometryData geom_data{geom_model};

  RobotScene robot_scene{renderer, geom_model, geom_data, {}};

  // Add plane
  const Eigen::Affine3f plane_transform{Eigen::UniformScaling<float>(3.0f)};
  auto &plane_obj = robot_scene.addEnvironmentObject(
      loadPlaneTiled(0.25f, 5, 5), plane_transform.matrix());
  auto &robotShapes = robot_scene.robotObjects;
  SDL_assert(robotShapes.size() == geom_model.ngeoms);
  SDL_Log("Created %zu robot mesh shapes.", robotShapes.size());

  /** DEBUG SYSTEM **/
  DebugScene debug_scene{renderer};
  auto &basic_debug_module = debug_scene.addModule<BasicDebugModule>();
  basic_debug_module.grid_color = 0xE0A236ff_rgbaf;

  auto depth_debug = DepthDebugPass::create(renderer, renderer.depth_texture);
  static DepthDebugPass::VizStyle depth_mode = DepthDebugPass::VIZ_GRAYSCALE;

  GuiSystem gui_system{[&](Renderer &r) {
    static bool demo_window_open = true;

    ImGui::Begin("Renderer info & controls", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Device driver: %s", r.device.driverName());
    ImGui::SeparatorText("Camera");
    bool ortho_change, persp_change;
    ortho_change = ImGui::RadioButton("Orthographic", (int *)&cam_type,
                                      int(CameraProjection::ORTHOGRAPHIC));
    ImGui::SameLine();
    persp_change = ImGui::RadioButton("Perspective", (int *)&cam_type,
                                      int(CameraProjection::PERSPECTIVE));
    switch (cam_type) {
    case CameraProjection::ORTHOGRAPHIC:
      ortho_change |=
          ImGui::DragFloat("zoom", &currentOrthoScale, 0.01f, 0.1f, 2.f, "%.3f",
                           ImGuiSliderFlags_AlwaysClamp);
      if (ortho_change)
        updateOrtho(currentOrthoScale);
      break;
    case CameraProjection::PERSPECTIVE:
      Degf newFov{currentFov};
      persp_change |= ImGui::DragFloat("fov", (float *)newFov, 1.f, 15.f, 90.f,
                                       "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (persp_change)
        updateFov(Radf(newFov));
      break;
    }

    ImGui::SeparatorText("Env. status");
    ImGui::Checkbox("Render plane", &plane_obj.status);
    ImGui::Checkbox("Render grid", &basic_debug_module.enableGrid);
    ImGui::Checkbox("Render triad", &basic_debug_module.enableTriad);
    if (ImGui::Checkbox("Show depth debug", &showDebugViz)) {
      // do stuff here
      if (showDebugViz) {
        SDL_Log("Turned on depth debug viz.");
      }
    }
    if (showDebugViz) {
      ImGui::RadioButton("Grayscale", (int *)&depth_mode, 0);
      ImGui::SameLine();
      ImGui::RadioButton("Heatmap", (int *)&depth_mode, 1);
    }

    ImGui::SeparatorText("Lights");
    ImGui::DragFloat("intens.", &myLight.intensity, 0.1f, 0.1f, 10.0f);
    ImGui::ColorEdit3("color", myLight.color.data());
    ImGui::Separator();
    ImGui::ColorEdit4("grid color", basic_debug_module.grid_color.data(),
                      ImGuiColorEditFlags_AlphaPreview);
    ImGui::ColorEdit4("plane color", plane_obj.materials.baseColor.data());
    ImGui::End();
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    ImGui::ShowDemoWindow(&demo_window_open);
  }};

  if (!gui_system.init(renderer)) {
    return 1;
  }

  // MAIN APPLICATION LOOP

  Uint32 frameNo = 0;

  Eigen::VectorXd q0 = pin::neutral(model);
  Eigen::VectorXd q1 = pin::randomConfiguration(model);

  media::VideoRecorder recorder{NoInit};
  if (performRecording)
    recorder = media::VideoRecorder{wWidth, wHeight, "ur5.mp4"};

  auto record_callback = [=, &renderer, &recorder]() {
    auto swapchain_format = renderer.getSwapchainTextureFormat();
    media::videoWriteTextureToFrame(renderer.device, recorder,
                                    renderer.swapchain, swapchain_format,
                                    wWidth, wHeight);
  };

  while (!quitRequested) {
    // logic
    eventLoop(renderer);
    double phi = 0.5 * (1. + std::sin(frameNo * 1e-2));
    Eigen::VectorXd q = pin::interpolate(model, q0, q1, phi);
    pin::forwardKinematics(model, pin_data, q);
    pin::updateGeometryPlacements(model, pin_data, geom_model, geom_data);

    // acquire command buffer and swapchain
    robot_scene.directionalLight = myLight;
    renderer.beginFrame();

    if (renderer.acquireSwapchain()) {
      robot_scene.render(renderer, camera);
      if (showDebugViz) {
        renderDepthDebug(renderer, depth_debug, {depth_mode, nearZ, farZ});
      } else {
        debug_scene.render(renderer, camera);
      }
      gui_system.render(renderer);
    } else {
      SDL_Log("Failed to acquire swapchain: %s", SDL_GetError());
      continue;
    }

    renderer.endFrame();

    if (performRecording) {
      record_callback();
    }
    frameNo++;
  }

  depth_debug.release(renderer.device);
  robot_scene.release();
  debug_scene.release();
  gui_system.release();
  renderer.destroy();
  SDL_DestroyWindow(renderer.window);
  SDL_Quit();
  return 0;
}
