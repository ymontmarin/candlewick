#include "Common.h"
// #include "GenHeightfield.h"

#include "candlewick/core/debug/DepthViz.h"
#include "candlewick/core/debug/Frustum.h"
#include "candlewick/core/Renderer.h"
#include "candlewick/core/GuiSystem.h"
#include "candlewick/core/DebugScene.h"
#include "candlewick/core/DepthAndShadowPass.h"
#include "candlewick/core/LightUniforms.h"

#include "candlewick/multibody/RobotScene.h"
#include "candlewick/multibody/RobotDebug.h"
#include "candlewick/primitives/Plane.h"
#include "candlewick/utils/WriteTextureToImage.h"
#include "candlewick/core/Camera.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>

#include <robot_descriptions_cpp/robot_load.hpp>

#include <pinocchio/multibody/data.hpp>
#include <pinocchio/multibody/geometry.hpp>
#include <pinocchio/algorithm/frames.hpp>
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

#include <entt/entity/registry.hpp>

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
static bool showFrustum = false;
enum VizMode {
  FULL_RENDER,
  DEPTH_DEBUG,
  LIGHT_DEBUG,
};
static VizMode showDebugViz = FULL_RENDER;

static float pixelDensity;
static float displayScale;

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
  RobotScene::Config robot_scene_config;
  robot_scene_config.triangle_has_prepass = true;
  robot_scene_config.enable_normal_target = true;

  argv = app.ensure_utf8(argv);
  app.add_flag("-r,--record", performRecording, "Record output");
  CLI11_PARSE(app, argc, argv);

  if (!SDL_Init(SDL_INIT_VIDEO))
    return 1;

  Renderer renderer =
      createRenderer(wWidth, wHeight, SDL_GPU_TEXTUREFORMAT_D32_FLOAT);

  entt::registry registry{};

  // Load robot
  pin::Model model;
  pin::GeometryModel geom_model;
  robot_descriptions::loadModelsFromToml("ur.toml", "ur5_gripper", model,
                                         &geom_model, NULL);
  // ADD HEIGHTFIELD GEOM
  // {
  //   auto hfield = generatePerlinNoiseHeightfield(40, 10u, 6.f);
  //   pin::GeometryObject gobj{"custom_hfield", 0ul, pin::SE3::Identity(),
  //                            hfield};
  //   gobj.meshColor = 0xA03232FF_rgba;
  //   geom_model.addGeometryObject(gobj);
  // }
  pin::Data pin_data{model};
  pin::GeometryData geom_data{geom_model};

  RobotScene robot_scene{registry, renderer, geom_model, geom_data,
                         robot_scene_config};
  auto &sceneLight = robot_scene.directionalLight;
  sceneLight = {
      .direction = {-1.f, 0.f, -1.},
      .color = {1.0, 1.0, 1.0},
      .intensity = 8.0,
  };

  // Add plane
  const Eigen::Affine3f plane_transform{Eigen::UniformScaling<float>(3.0f)};
  entt::entity plane_entity = robot_scene.addEnvironmentObject(
      loadPlaneTiled(0.5f, 20, 20), plane_transform.matrix());
  auto [plane_obj, plane_vis] =
      registry.get<RobotScene::MeshMaterialComponent,
                   multibody::VisibilityComponent>(plane_entity);

  robot_scene.addEnvironmentObject(loadCube(.33f, {-0.55f, -0.7f}),
                                   Mat4f::Identity());

  const size_t numRobotShapes =
      registry.view<const multibody::PinGeomObjComponent>().size();
  SDL_assert(numRobotShapes == geom_model.ngeoms);
  SDL_Log("Registered %zu robot geometry objects.", numRobotShapes);

  // DEBUG SYSTEM

  DebugScene debug_scene{renderer};
  auto &robot_debug =
      debug_scene.addSystem<multibody::RobotDebugSystem>(model, pin_data);
  auto [triad_id, triad] = debug_scene.addTriad();
  auto [grid_id, grid] = debug_scene.addLineGrid(0xE0A236ff_rgbaf);
  pin::FrameIndex ee_frame_id = model.getFrameId("ee_link");
  robot_debug.addFrameTriad(debug_scene, ee_frame_id);
  robot_debug.addFrameVelocityArrow(debug_scene, ee_frame_id);

  auto depthPassInfo = DepthPassInfo::create(renderer, plane_obj.mesh.layout);
  auto &shadowPassInfo = robot_scene.shadowPass;
  auto shadowDebugPass =
      DepthDebugPass::create(renderer, shadowPassInfo.depthTexture);

  auto depthDebugPass =
      DepthDebugPass::create(renderer, renderer.depth_texture);
  DepthDebugPass::VizStyle depth_mode = DepthDebugPass::VIZ_GRAYSCALE;

  FrustumBoundsDebugSystem frustumBoundsDebug{registry, renderer};

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
      persp_change |= ImGui::DragFloat("fov", newFov, 1.f, 15.f, 90.f, "%.3f",
                                       ImGuiSliderFlags_AlwaysClamp);
      persp_change |=
          ImGui::SliderFloat("Near plane", &nearZ, 0.01f, 0.8f * farZ);
      persp_change |= ImGui::SliderFloat("Far plane", &farZ, nearZ, 20.f);
      if (persp_change)
        updateFov(Radf(newFov));
      break;
    }

    ImGui::SeparatorText("Env. status");
    ImGui::Checkbox("Render plane", &plane_vis.status);
    ImGui::Checkbox("Render grid", &grid.enable);
    ImGui::Checkbox("Render triad", &triad.enable);
    ImGui::Checkbox("Render frustum", &showFrustum);

    ImGui::Checkbox("Ambient occlusion (SSAO)", &robot_scene.useSsao);

    ImGui::RadioButton("Full render mode", (int *)&showDebugViz, FULL_RENDER);
    ImGui::SameLine();
    ImGui::RadioButton("Depth debug", (int *)&showDebugViz, DEPTH_DEBUG);
    ImGui::SameLine();
    ImGui::RadioButton("Light mode", (int *)&showDebugViz, LIGHT_DEBUG);

    if (showDebugViz & (DEPTH_DEBUG | LIGHT_DEBUG)) {
      ImGui::RadioButton("Grayscale", (int *)&depth_mode, 0);
      ImGui::SameLine();
      ImGui::RadioButton("Heatmap", (int *)&depth_mode, 1);
    }

    ImGui::SeparatorText("Lights");
    ImGui::SliderFloat("intens.", &sceneLight.intensity, 0.1f, 10.0f);
    ImGui::DragFloat3("direction", sceneLight.direction.data(), 0.0f, -1.f,
                      1.f);
    ImGui::ColorEdit3("color", sceneLight.color.data());
    ImGui::Separator();
    ImGui::ColorEdit4("grid color", grid.colors[0].data(),
                      ImGuiColorEditFlags_AlphaPreview);
    ImGui::ColorEdit4("plane color", plane_obj.materials[0].baseColor.data());
    ImGui::End();
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    ImGui::ShowDemoWindow(&demo_window_open);
  }};

  if (!gui_system.init(renderer)) {
    return 1;
  }

  // MAIN APPLICATION LOOP

  Uint32 frameNo = 0;

  srand(42);
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

  AABB &worldSpaceBounds = robot_scene.worldSpaceBounds;
  worldSpaceBounds.grow({-1.f, -1.f, 0.f});
  worldSpaceBounds.grow({+1.f, +1.f, 1.f});

  frustumBoundsDebug.addBounds(worldSpaceBounds);
  frustumBoundsDebug.addFrustum(shadowPassInfo.cam);

  Eigen::VectorXd q = q0;
  const double dt = 1e-2;

  while (!quitRequested) {
    // logic
    eventLoop(renderer);
    double alpha = 0.5 * (1. + std::sin(frameNo * dt));
    Eigen::VectorXd qn = q;
    q = pin::interpolate(model, q0, q1, alpha);
    Eigen::VectorXd v = pin::difference(model, q, qn) / dt;
    pin::forwardKinematics(model, pin_data, q, v);
    pin::updateFramePlacements(model, pin_data);
    pin::updateGeometryPlacements(model, pin_data, geom_model, geom_data);
    debug_scene.update();

    FrustumCornersType main_cam_frustum = frustumFromCamera(camera);

    // acquire command buffer and swapchain
    renderer.beginFrame();

    if (renderer.waitAndAcquireSwapchain()) {
      const GpuMat4 viewProj = camera.viewProj();
      multibody::updateRobotTransforms(registry, robot_scene.geomData());
      robot_scene.collectOpaqueCastables();
      auto &castables = robot_scene.castables();
      renderShadowPassFromAABB(renderer, shadowPassInfo, sceneLight, castables,
                               worldSpaceBounds);
      renderDepthOnlyPass(renderer, depthPassInfo, viewProj, castables);
      switch (showDebugViz) {
      case FULL_RENDER:
        renderDepthOnlyPass(renderer, depthPassInfo, viewProj, castables);
        robot_scene.render(renderer, camera);
        debug_scene.render(renderer, camera);
        if (showFrustum)
          frustumBoundsDebug.render(renderer, camera);
        break;
      case DEPTH_DEBUG:
        renderDepthDebug(renderer, depthDebugPass, {depth_mode, nearZ, farZ});
        break;
      case LIGHT_DEBUG:
        renderDepthDebug(renderer, shadowDebugPass,
                         {depth_mode,
                          orthoProjNear(shadowPassInfo.cam.projection),
                          orthoProjFar(shadowPassInfo.cam.projection),
                          CameraProjection::ORTHOGRAPHIC});
        break;
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

  frustumBoundsDebug.release();
  depthPassInfo.release();
  shadowDebugPass.release(renderer.device);
  depthDebugPass.release(renderer.device);
  robot_scene.release();
  debug_scene.release();
  gui_system.release();
  renderer.destroy();
  SDL_DestroyWindow(renderer.window);
  SDL_Quit();
  return 0;
}
