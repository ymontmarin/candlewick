#include "Common.h"

#include "candlewick/core/MeshGroup.h"
#include "candlewick/core/Shader.h"
#include "candlewick/core/math_util.h"
#include "candlewick/core/LightUniforms.h"
#include "candlewick/core/MaterialUniform.h"
#include "candlewick/utils/MeshData.h"
#include "candlewick/utils/LoadMesh.h"
#include "candlewick/utils/CylinderControl.h"
#include "candlewick/multibody/LoadPinocchioGeometry.h"

#include "candlewick/primitives/Plane.h"

#include <robot_descriptions_cpp/robot_spec.hpp>
#include <robot_descriptions_cpp/robot_load.hpp>

#include <pinocchio/multibody/data.hpp>
#include <pinocchio/multibody/geometry.hpp>
#include <pinocchio/algorithm/kinematics.hpp>
#include <pinocchio/algorithm/joint-configuration.hpp>
#include <pinocchio/algorithm/geometry.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_filesystem.h>

namespace pin = pinocchio;
using namespace candlewick;
using Eigen::Matrix3f;
using Eigen::Matrix4f;

const float wWidth = 1600;
const float wHeight = 900;
const float aspectRatio = wWidth / wHeight;

static bool add_plane = true;
static Matrix4f viewMat;
static Matrix4f projectionMat;
static Rad<float> fov = 55.0_radf;

Context ctx;

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

struct alignas(16) TransformUniformData {
  GpuMat4 model;
  alignas(16) GpuMat4 mvp;
  alignas(16) GpuMat3 normalMatrix;
};

static DirectionalLightUniform myLight{
    .direction = {0., -1., -1.},
    .color = {1.0, 1.0, 1.0},
    .intensity = 1.0,
};

void eventLoop(bool &quitRequested) {
  const float pixelDensity = SDL_GetWindowPixelDensity(ctx.window);
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      SDL_Log("Application exit requested.");
      quitRequested = true;
      break;
    }
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
      float wy = event.wheel.y;
      const float scaleFac = std::exp(kScrollZoom * wy);
      // orthographicZoom(projectionMat, scaleFac);
      // recreate
      fov = std::min(fov * scaleFac, 170.0_radf);
      SDL_Log("Change fov to %f", rad2deg(fov));
      projectionMat = perspectiveFromFov(fov, aspectRatio, 0.01, 10.0);
    }
    if (event.type == SDL_EVENT_KEY_DOWN) {
      const float step_size = 0.06;
      switch (event.key.key) {
      case SDLK_UP:
        cylinderCameraUpDown(viewMat, +step_size);
        break;
      case SDLK_DOWN:
        cylinderCameraUpDown(viewMat, -step_size);
        break;
      }
    }
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
      SDL_MouseButtonFlags mouseButton = event.motion.state;
      bool dragging = mouseButton >= SDL_BUTTON_LMASK;
      if (dragging) {
        Float2 camViewportSpeed{0.005, 0.01};
        camViewportSpeed *= pixelDensity;
        cylinderCameraViewportDrag(
            viewMat, Float2{event.motion.xrel, event.motion.yrel}.cwiseProduct(
                         camViewportSpeed));
      }
      if (mouseButton >= SDL_BUTTON_RMASK) {
        float camXLocRotSpeed = 0.01 * pixelDensity;
        cylinderXLocalRotate(viewMat, camXLocRotSpeed * event.motion.yrel);
      }
    }
  }
}

int main() {
  if (!ExampleInit(ctx, wWidth, wHeight)) {
    return 1;
  }
  Device &device = ctx.device;
  SDL_Window *window = ctx.window;

  pin::Model model;
  pin::GeometryModel geom_model;
  auto robot_spec = robot_descriptions::loadRobotSpecFromToml("ur.toml", "ur5");
  robot_descriptions::loadModelFromSpec(robot_spec, model);
  robot_descriptions::loadGeomFromSpec(robot_spec, model, geom_model,
                                       pin::VISUAL);
  SDL_assert(model.nq == 7);
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
  } plane{loadPlaneTiled(0.2f, 3, 1), Mesh{NoInit}};
  new (&plane.mesh) Mesh{convertToMesh(device, plane.data)};
  uploadMeshToDevice(device, plane.mesh, plane.data);
  /** CREATE PIPELINE **/
  Shader vertexShader{device, "PbrBasic.vert", 1};
  Shader fragmentShader{device, "PbrBasic.frag", 2};

  SDL_GPUTextureFormat depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
  SDL_GPUTexture *depthTexture = createDepthTexture(
      device, window, depth_stencil_format, SDL_GPU_SAMPLECOUNT_1);

  SDL_GPUColorTargetDescription colorTarget;
  SDL_zero(colorTarget);
  colorTarget.format = SDL_GetGPUSwapchainTextureFormat(device, window);

  // create pipeline
  SDL_GPUGraphicsPipelineCreateInfo mesh_pipeline_desc{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .vertex_input_state =
          meshGroups[0].meshes[0].layout().toVertexInputState(),
      .primitive_type = meshGroups[0].meshes[0].layout().primitiveType(),
      .rasterizer_state{.fill_mode = SDL_GPU_FILLMODE_FILL,
                        .cull_mode = SDL_GPU_CULLMODE_NONE,
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
  SDL_GPUGraphicsPipeline *mesh_pipeline =
      SDL_CreateGPUGraphicsPipeline(device, &mesh_pipeline_desc);
  if (mesh_pipeline == NULL) {
    SDL_Log("Failed to create pipeline: %s", SDL_GetError());
    return 1;
  }

  vertexShader.release();
  fragmentShader.release();

  projectionMat = perspectiveFromFov(fov, aspectRatio, 0.01, 10.0);

  Uint32 frameNo = 0;
  bool quitRequested = false;
  viewMat = lookAt({2.0, 0, 2.}, Float3::Zero());

  Eigen::VectorXd q0 = pin::neutral(model);
  Eigen::VectorXd q1 = pin::randomConfiguration(model);

  while (frameNo < 1000 && !quitRequested) {
    // logic
    eventLoop(quitRequested);
    double phi = 0.5 * (1. + std::sin(frameNo * 1e-2));
    Eigen::VectorXd q = pin::interpolate(model, q0, q1, phi);
    pin::forwardKinematics(model, pin_data, q);
    pin::updateGeometryPlacements(model, pin_data, geom_model, geom_data);

    // render pass

    SDL_GPUCommandBuffer *command_buffer;
    SDL_GPUTexture *swapchain;
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

      // loop over mesh groups
      for (size_t i = 0; i < geom_model.ngeoms; i++) {
        const pin::SE3 &placement = geom_data.oMg[i];
        const MeshGroup &mg = meshGroups[i];
        SDL_BindGPUGraphicsPipeline(render_pass, mesh_pipeline);

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
        SDL_PushGPUFragmentUniformData(command_buffer, 1, &myLight,
                                       sizeof(myLight));

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
        SDL_BindGPUGraphicsPipeline(render_pass, mesh_pipeline);
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
        SDL_PushGPUFragmentUniformData(command_buffer, 1, &myLight,
                                       sizeof(myLight));
        SDL_PushGPUFragmentUniformData(command_buffer, 0, &material,
                                       sizeof(PbrMaterialUniform));

        SDL_GPUBufferBinding vertex_binding{
            .buffer = plane.mesh.vertexBuffers[0],
            .offset = plane.mesh.vertexBufferOffsets[0],
        };
        SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);

        if (plane.mesh.isIndexed()) {
          SDL_GPUBufferBinding index_binding{
              .buffer = plane.mesh.indexBuffer,
              .offset = plane.mesh.indexBufferOffset,
          };
          SDL_BindGPUIndexBuffer(render_pass, &index_binding,
                                 SDL_GPU_INDEXELEMENTSIZE_32BIT);
          SDL_DrawGPUIndexedPrimitives(render_pass, plane.mesh.count, 1, 0, 0,
                                       0);
        } else {
          SDL_DrawGPUPrimitives(render_pass, plane.mesh.count, 1, 0, 0);
        }
      }

      SDL_EndGPURenderPass(render_pass);
    }

    SDL_SubmitGPUCommandBuffer(command_buffer);
    frameNo++;
  }

  for (auto &mg : meshGroups) {
    mg.releaseBuffers(device);
  }
  SDL_ReleaseGPUGraphicsPipeline(device, mesh_pipeline);

  ExampleTeardown(ctx);
  return 0;
}
