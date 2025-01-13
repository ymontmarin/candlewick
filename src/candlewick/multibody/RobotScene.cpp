#include "RobotScene.h"
#include "LoadPinocchioGeometry.h"
#include "../third-party/magic_enum.hpp"
#include "../utils/CameraControl.h"

#include <coal/BVH/BVH_model.h>

namespace candlewick::multibody {
RobotScene::PipelineType
RobotScene::pinGeomToPipeline(const coal::CollisionGeometry &geom) {
  using enum coal::OBJECT_TYPE;
  const auto objType = geom.getObjectType();
  switch (objType) {
  case OT_GEOM:
    return PIPELINE_TRIANGLEMESH;
  case OT_HFIELD:
    return PIPELINE_HEIGHTFIELD;
  case OT_BVH: {
    using enum coal::BVHModelType;
    // cast to BVHModelBase, check if mesh or points
    const auto &bvh = dynamic_cast<const coal::BVHModelBase &>(geom);
    switch (bvh.getModelType()) {
    case BVH_MODEL_POINTCLOUD:
      return PIPELINE_POINTCLOUD;
    case BVH_MODEL_TRIANGLES:
      return PIPELINE_TRIANGLEMESH;
    case BVH_MODEL_UNKNOWN:
      throw InvalidArgument("Unknown BVH model type.");
    }
  }
  case OT_COUNT:
  case OT_OCTREE:
  case OT_UNKNOWN:
    char errbuf[32];
    SDL_snprintf(errbuf, sizeof(errbuf), "Unsupported object type %s",
                 magic_enum::enum_name(objType).data());
    throw InvalidArgument(errbuf);
  }
}

RobotScene::RobotScene(const Renderer &renderer,
                       const pin::GeometryModel &geom_model,
                       const pin::GeometryData &geom_data, Config config)
    : _device(renderer.device), _geomModel(geom_model), _geomData(geom_data) {
  for (const auto type : {
           PIPELINE_TRIANGLEMESH, PIPELINE_HEIGHTFIELD,
           //  PIPELINE_POINTCLOUD
       }) {
    if (!config.pipeline_configs.contains(type)) {
      char errbuf[64];
      size_t maxlen = sizeof(errbuf);
      SDL_snprintf(errbuf, maxlen,
                   "Missing pipeline config for pipeline type %s",
                   magic_enum::enum_name(type).data());
      throw InvalidArgument(errbuf);
    }
  }

  auto &dev = renderer.device;
  auto swapchain_format =
      SDL_GetGPUSwapchainTextureFormat(dev, renderer.window);

  for (pin::GeomIndex i = 0; i < geom_model.ngeoms; i++) {
    const auto &geom_obj = geom_model.geometryObjects[i];
    auto mesh_datas = loadGeometryObject(geom_obj);
    PipelineType pipeline_type = pinGeomToPipeline(*geom_obj.geometry);
    auto shape = Shape::createShapeFromDatas(dev, mesh_datas, true);
    const auto &item =
        robotShapes.emplace_back(pipeline_type, std::move(shape));
    if (!pipelines.contains(pipeline_type)) {
      auto pipe_config = config.pipeline_configs.at(pipeline_type);
      SDL_Log("%s(): building pipeline for type %s", __FUNCTION__,
              magic_enum::enum_name(pipeline_type).data());
      auto *pipeline =
          createPipeline(dev, item.second.layout(), swapchain_format,
                         renderer.depth_format, pipeline_type, pipe_config);
      pipelines.emplace(pipeline_type, pipeline);
    }
  }
}

void RobotScene::render(Renderer &renderer, const Camera &cameraState,
                        RenderPostCallback post_callback) {
  SDL_GPUColorTargetInfo color_target{
      .texture = renderer.swapchain,
      .clear_color{},
      .load_op = SDL_GPU_LOADOP_CLEAR,
      .store_op = SDL_GPU_STOREOP_STORE,
  };
  SDL_GPUDepthStencilTargetInfo depth_target{
      .texture = renderer.depth_texture,
      .clear_depth = 1.0f,
      .load_op = SDL_GPU_LOADOP_CLEAR,
      .store_op = SDL_GPU_STOREOP_STORE,
      .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
      .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
  };

  struct alignas(16) LightUniform {
    DirectionalLight a;
    GpuVec3 viewPos;
  };
  const Mat4f viewMat = cameraState.viewMatrix();
  const LightUniform lightUbo{directionalLight, cameraViewPos(viewMat)};
  const Mat4f projView = cameraState.projection * viewMat;
  SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
      renderer.command_buffer, &color_target, 1, &depth_target);
  renderer.pushFragmentUniform(FragmentUniformSlots::LIGHTING, &lightUbo,
                               sizeof(lightUbo));

  // iterate over primitive types in the keys
  for (const auto &[pipeline_type, pipeline] : pipelines) {
    SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
    for (pin::GeomIndex i = 0; i < _geomModel.ngeoms; i++) {
      if (robotShapes[i].first != pipeline_type)
        continue;
      const auto &shape = robotShapes[i].second;
      const auto &placement = _geomData.oMg[i].cast<float>();
      const Mat4f modelMat = placement.toHomogeneousMatrix();
      const Mat3f normalMatrix =
          modelMat.topLeftCorner<3, 3>().inverse().transpose();
      const Mat4f mvp = projView * modelMat;
      TransformUniformData data{
          .model = modelMat,
          .mvp = mvp,
          .normalMatrix = normalMatrix,
      };
      renderer.pushVertexUniform(VertexUniformSlots::TRANSFORM, &data,
                                 sizeof(data));
      renderer.render(render_pass, shape);
    }

    for (const auto &[status, mesh, meshData, modelMat] : environmentMeshes) {
      if (!status)
        continue;
      const Mat3f normalMatrix =
          modelMat.topLeftCorner<3, 3>().inverse().transpose();
      const Mat4f mvp = projView * modelMat;
      TransformUniformData data{
          .model = modelMat,
          .mvp = mvp,
          .normalMatrix = normalMatrix,
      };
      renderer.pushVertexUniform(VertexUniformSlots::TRANSFORM, &data,
                                 sizeof(data));
      auto materialUniform = meshData.material.toUniform();
      renderer.pushFragmentUniform(Shape::MATERIAL_SLOT, &materialUniform,
                                   sizeof(materialUniform));
      renderer.render(render_pass, mesh);
    }

    post_callback(render_pass);
    SDL_EndGPURenderPass(render_pass);
  }
}

void RobotScene::release() {
  for (auto &shape : robotShapes) {
    shape.second.release();
  }
  if (!_device)
    return;
  robotShapes.clear();
  for (auto &[status, mesh, meshdata, tr] : environmentMeshes) {
    mesh.releaseOwnedBuffers(_device);
  }
  environmentMeshes.clear();

  for (auto &[primType, pipeline] : pipelines) {
    SDL_ReleaseGPUGraphicsPipeline(_device, pipeline);
    pipeline = nullptr;
  }
}

SDL_GPUGraphicsPipeline *
RobotScene::createPipeline(const Device &dev, const MeshLayout &layout,
                           SDL_GPUTextureFormat render_target_format,
                           SDL_GPUTextureFormat depth_stencil_format,
                           PipelineType type,
                           const Config::PipelineConfig &config) {

  Shader vertex_shader{dev, config.vertex_shader_path,
                       config.num_vertex_uniforms};
  Shader fragment_shader{dev, config.fragment_shader_path,
                         config.num_frag_uniforms};

  SDL_GPUColorTargetDescription color_target;
  SDL_zero(color_target);
  color_target.format = render_target_format;
  SDL_GPUGraphicsPipelineCreateInfo desc{
      .vertex_shader = vertex_shader,
      .fragment_shader = fragment_shader,
      .vertex_input_state = layout.toVertexInputState(),
      .primitive_type = getPrimitiveTopologyForType(type),
      .depth_stencil_state{
          .compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
          .enable_depth_test = true,
          .enable_depth_write = true,
      },
      .target_info{
          .color_target_descriptions = &color_target,
          .num_color_targets = 1,
          .depth_stencil_format = depth_stencil_format,
          .has_depth_stencil_target = true,
      },
  };
  desc.rasterizer_state.cull_mode = config.cull_mode;
  desc.rasterizer_state.fill_mode = config.fill_mode;
  return SDL_CreateGPUGraphicsPipeline(dev, &desc);
}
} // namespace candlewick::multibody
