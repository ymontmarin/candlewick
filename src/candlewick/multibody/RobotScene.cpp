#include "RobotScene.h"
#include "LoadPinocchioGeometry.h"
#include "../third-party/magic_enum.hpp"

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
                       const pin::GeometryModel &geom_model, Config config)
    : _device(renderer.device) {
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
  for (pin::GeomIndex i = 0; i < geom_model.ngeoms; i++) {
    const auto &geom_obj = geom_model.geometryObjects[i];
    auto mesh_datas = loadGeometryObject(geom_obj);
    PipelineType pipeline_type = pinGeomToPipeline(*geom_obj.geometry);
    robotShapes.push_back(Shape::createShapeFromDatas(dev, mesh_datas, true));
    shapeIndices[pipeline_type].push_back(i);
  }

  auto swapchain_format =
      SDL_GetGPUSwapchainTextureFormat(dev, renderer.window);
  for (const auto &[pipeline_type, pipe_config] : config.pipeline_configs) {
    // skip pipeline if not needed
    if (!shapeIndices.contains(pipeline_type))
      continue;
    const auto &sample_shape = robotShapes[shapeIndices[pipeline_type][0]];
    SDL_Log("%s(): building pipeline for type %s", __FUNCTION__,
            magic_enum::enum_name(pipeline_type).data());
    pipelines.emplace(pipeline_type,
                      createPipeline(dev, sample_shape.layout(),
                                     swapchain_format, renderer.depth_format,
                                     pipeline_type, pipe_config));
  }
}

void RobotScene::render(Renderer &renderer, RenderPostCallback post_callback) {
  SDL_GPUColorTargetInfo color_target{
      .texture = renderer.swapchain,
      .clear_color{},
      .load_op = SDL_GPU_LOADOP_CLEAR,
      .store_op = SDL_GPU_STOREOP_STORE,
  };
  SDL_GPUDepthStencilTargetInfo depth_target;
  SDL_zero(depth_target);
  depth_target.clear_depth = 1.0f;
  depth_target.load_op = SDL_GPU_LOADOP_CLEAR;
  depth_target.store_op = SDL_GPU_STOREOP_STORE;
  depth_target.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
  depth_target.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
  depth_target.texture = renderer.depth_texture;
  SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
      renderer.command_buffer, &color_target, 1, &depth_target);

  // iterate over primitive types in the keys
  for (const auto &[primitive_type, pipeline] : pipelines) {
    SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
    const auto &indices = shapeIndices[primitive_type];
    for (const auto id : indices) {
      for (auto &[binding, provider] : vertexUniformProviders) {
        auto payload = provider(id);
        SDL_PushGPUVertexUniformData(renderer.command_buffer, binding,
                                     payload.data(), payload.size());
      }
      for (auto &[binding, provider] : fragmentUniformProviders) {
        auto payload = provider(id);
        SDL_PushGPUFragmentUniformData(renderer.command_buffer, binding,
                                       payload.data(), payload.size());
      }
      renderer.renderShape(render_pass, robotShapes[id]);
    }
  }

  post_callback(render_pass);
  SDL_EndGPURenderPass(render_pass);
}

void RobotScene::release() {
  for (auto &shape : robotShapes) {
    shape.release();
  }
  robotShapes.clear();

  for (auto &[primType, pipeline] : pipelines) {
    SDL_ReleaseGPUGraphicsPipeline(_device, pipeline);
    pipeline = nullptr;
  }
}
} // namespace candlewick::multibody
