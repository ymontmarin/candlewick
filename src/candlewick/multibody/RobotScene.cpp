#include "RobotScene.h"
#include "LoadPinocchioGeometry.h"
#include "../core/errors.h"
#include "../core/Shader.h"
#include "../core/TransformUniforms.h"
#include "../utils/CameraControl.h"
#include "../third-party/magic_enum.hpp"

#include <coal/BVH/BVH_model.h>
#include <pinocchio/multibody/data.hpp>

namespace candlewick::multibody {
auto RobotScene::pinGeomToPipeline(const coal::CollisionGeometry &geom)
    -> PipelineType {
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

entt::entity RobotScene::addEnvironmentObject(MeshData &&data, Mat4f placement,
                                              PipelineType pipe_type) {
  Mesh mesh = createMesh(_device, data);
  uploadMeshToDevice(_device, mesh.toView(), data);
  auto entity = registry.create();
  registry.emplace<TransformComponent>(entity, placement);
  if (pipe_type != PIPELINE_POINTCLOUD)
    registry.emplace<Opaque>(entity);
  registry.emplace<VisibilityComponent>(entity, true);
  registry.emplace<MeshMaterialComponent>(
      entity, std::move(mesh), std::vector{mesh.toView()},
      std::vector{std::move(data.material)}, pipe_type);
  return entity;
}

RobotScene::RobotScene(entt::registry &registry, const Renderer &renderer,
                       const pin::GeometryModel &geom_model,
                       const pin::GeometryData &geom_data, Config config)
    : registry(registry), config(config), _device(renderer.device),
      _geomData(&geom_data) {
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
  auto swapchain_format = renderer.getSwapchainTextureFormat();

  for (pin::GeomIndex geom_id = 0; geom_id < geom_model.ngeoms; geom_id++) {

    const auto &geom_obj = geom_model.geometryObjects[geom_id];
    auto meshDatas = loadGeometryObject(geom_obj);
    PipelineType pipeline_type = pinGeomToPipeline(*geom_obj.geometry);
    auto [mesh, views] = createMeshFromBatch(dev, meshDatas, true);
    assert(validateMesh(mesh));

    auto entity = registry.create();
    registry.emplace<PinGeomObjComponent>(entity, geom_id);
    if (pipeline_type != PIPELINE_POINTCLOUD)
      registry.emplace<Opaque>(entity);
    registry.emplace<MeshMaterialComponent>(
        entity, std::move(mesh), std::move(views), extractMaterials(meshDatas),
        pipeline_type);

    if (!renderPipelines.contains(pipeline_type)) {
      auto pipe_config = config.pipeline_configs.at(pipeline_type);
      SDL_Log("%s(): building pipeline for type %s", __FUNCTION__,
              magic_enum::enum_name(pipeline_type).data());
      const auto &layout = mesh.layout;
      auto *pipeline =
          createPipeline(dev, layout, swapchain_format, renderer.depth_format,
                         pipeline_type, pipe_config);
      assert(pipeline);
      renderPipelines.emplace(pipeline_type, PipelineData{pipeline, layout});
    }
  }
}

void RobotScene::render(Renderer &renderer, const Camera &camera) {
  // render geometry which participated in the prepass
  renderGeometryPass(renderer, camera, true);

  // render geometry which was _not_ in the prepass
  renderGeometryPass(renderer, camera, false);
}

void RobotScene::renderGeometryPass(Renderer &renderer, const Camera &camera,
                                    bool had_prepass) {
  SDL_GPUColorTargetInfo color_target{
      .texture = renderer.swapchain,
      .clear_color{},
      .load_op = SDL_GPU_LOADOP_CLEAR,
      .store_op = SDL_GPU_STOREOP_STORE,
  };
  SDL_GPUDepthStencilTargetInfo depth_target{
      .texture = renderer.depth_texture,
      .clear_depth = 1.0f,
      .load_op = had_prepass ? SDL_GPU_LOADOP_LOAD : SDL_GPU_LOADOP_CLEAR,
      .store_op = SDL_GPU_STOREOP_STORE,
      .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
      .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
  };

  struct alignas(16) light_ubo_t {
    DirectionalLight a;
    GpuVec3 viewPos;
  };
  const light_ubo_t lightUbo{directionalLight, camera.position()};
  const Mat4f viewProj = camera.viewProj();

  SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
      renderer.command_buffer, &color_target, 1, &depth_target);
  renderer.pushFragmentUniform(FragmentUniformSlots::LIGHTING, &lightUbo,
                               sizeof(lightUbo));

  // iterate over primitive types in the keys
  for (const auto &[pipeline_type, pipe_data] : renderPipelines) {
    const auto &pipe_config = config.pipeline_configs[pipeline_type];
    if (pipe_config.has_prepass != had_prepass) {
      continue;
    }

    SDL_BindGPUGraphicsPipeline(render_pass, pipe_data.pipeline);

    auto robot_view =
        registry.view<const PinGeomObjComponent, const MeshMaterialComponent>();

    for (auto [entity, geom_id, obj] : robot_view.each()) {
      if (obj.pipeline_type != pipeline_type)
        continue;
      const auto &mesh = obj.mesh;
      const auto &placement = _geomData->oMg[geom_id].cast<float>();
      const Mat4f modelMat = placement.toHomogeneousMatrix();
      const Mat3f normalMatrix =
          modelMat.topLeftCorner<3, 3>().inverse().transpose();
      const Mat4f mvp = viewProj * modelMat;
      TransformUniformData data{
          .model = modelMat,
          .mvp = mvp,
          .normalMatrix = normalMatrix,
      };
      renderer.pushVertexUniform(VertexUniformSlots::TRANSFORM, &data,
                                 sizeof(data));
      renderer.bindMesh(render_pass, mesh);
      // loop over views and materials
      for (size_t j = 0; j < obj.views.size(); j++) {
        auto material = obj.materials[j].toUniform();
        renderer.pushFragmentUniform(FragmentUniformSlots::MATERIAL, &material,
                                     sizeof(material));
        renderer.drawView(render_pass, obj.views[j]);
      }
    }

    auto env_view =
        registry.view<const VisibilityComponent, const TransformComponent,
                      const MeshMaterialComponent>();
    for (auto [entity, visible, tr, obj] : env_view.each()) {
      if (!visible || (obj.pipeline_type != pipeline_type))
        continue;

      auto material = obj.materials[0].toUniform();
      auto modelMat = tr.transform;
      auto &mesh = obj.mesh;
      const Mat3f normalMatrix =
          modelMat.topLeftCorner<3, 3>().inverse().transpose();
      const Mat4f mvp = viewProj * modelMat;
      TransformUniformData data{
          .model = modelMat,
          .mvp = mvp,
          .normalMatrix = normalMatrix,
      };
      renderer.pushVertexUniform(VertexUniformSlots::TRANSFORM, &data,
                                 sizeof(data));
      renderer.bindMesh(render_pass, mesh);
      renderer.pushFragmentUniform(FragmentUniformSlots::MATERIAL, &material,
                                   sizeof(material));
      renderer.draw(render_pass, mesh);
    }
  }
  SDL_EndGPURenderPass(render_pass);
}

void RobotScene::release() {
  for (auto [entity, obj] : registry.view<MeshMaterialComponent>()->each()) {
    obj.mesh.release(_device);
  }
  if (!_device)
    return;

  for (auto [primType, pipe_data] : renderPipelines) {
    SDL_ReleaseGPUGraphicsPipeline(_device, pipe_data.pipeline);
    pipe_data.pipeline = nullptr;
  }
}

SDL_GPUGraphicsPipeline *
RobotScene::createPipeline(const Device &dev, const MeshLayout &layout,
                           SDL_GPUTextureFormat render_target_format,
                           SDL_GPUTextureFormat depth_stencil_format,
                           PipelineType type,
                           const Config::PipelineConfig &config) {

  SDL_Log("Creating pipeline type %s", magic_enum::enum_name(type).data());
  Shader vertex_shader{dev, config.vertex_shader_path,
                       config.num_vertex_uniforms};
  Shader fragment_shader{dev, config.fragment_shader_path,
                         config.num_frag_uniforms};

  SDL_GPUColorTargetDescription color_target;
  SDL_zero(color_target);
  color_target.format = render_target_format;
  SDL_GPUCompareOp depth_compare_op = config.has_prepass
                                          ? SDL_GPU_COMPAREOP_EQUAL
                                          : SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
  SDL_GPUGraphicsPipelineCreateInfo desc{
      .vertex_shader = vertex_shader,
      .fragment_shader = fragment_shader,
      .vertex_input_state = layout.toVertexInputState(),
      .primitive_type = getPrimitiveTopologyForType(type),
      .depth_stencil_state{
          .compare_op = depth_compare_op,
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
