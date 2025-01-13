#pragma once

#include "Multibody.h"
#include "../core/Device.h"
#include "../core/Scene.h"
#include "../core/Shape.h"
#include "../core/LightUniforms.h"

#include <pinocchio/multibody/geometry.hpp>

namespace candlewick {
namespace multibody {

class RobotScene {
public:
  enum PipelineType {
    PIPELINE_TRIANGLEMESH,
    PIPELINE_HEIGHTFIELD,
    PIPELINE_POINTCLOUD,
  };
  enum VertexUniformSlots : Uint32 { TRANSFORM = 0 };
  enum FragmentUniformSlots : Uint32 { LIGHTING = 1 };
  using RenderPostCallback = std::function<void(SDL_GPURenderPass *)>;

  /// Map hpp-fcl/coal collision geometry to desired pipeline type.
  static PipelineType pinGeomToPipeline(const coal::CollisionGeometry &geom);

  /// Map pipeline type to geometry primitive.
  static constexpr SDL_GPUPrimitiveType
  getPrimitiveTopologyForType(PipelineType type) {
    switch (type) {
    case PIPELINE_TRIANGLEMESH:
      return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    case PIPELINE_HEIGHTFIELD:
      return SDL_GPU_PRIMITIVETYPE_LINELIST;
    case PIPELINE_POINTCLOUD:
      return SDL_GPU_PRIMITIVETYPE_POINTLIST;
    }
  }

  std::vector<std::pair<PipelineType, Shape>> robotShapes;
  std::unordered_map<PipelineType, SDL_GPUGraphicsPipeline *> pipelines;
  struct EnvironmentData {
    bool status;
    Mesh mesh;
    MeshData data;
    Mat4f transform;
  };
  std::vector<EnvironmentData> environmentMeshes;
  DirectionalLight directionalLight;

  struct Config {
    struct PipelineConfig {
      // shader set
      const char *vertex_shader_path;
      const char *fragment_shader_path;
      Uint32 num_vertex_uniforms;
      Uint32 num_frag_uniforms;
      SDL_GPUCullMode cull_mode = SDL_GPU_CULLMODE_BACK;
      SDL_GPUFillMode fill_mode = SDL_GPU_FILLMODE_FILL;
    };
    std::unordered_map<PipelineType, PipelineConfig> pipeline_configs = {
        {PIPELINE_TRIANGLEMESH,
         {.vertex_shader_path = "PbrBasic.vert",
          .fragment_shader_path = "PbrBasic.frag",
          .num_vertex_uniforms = 1,
          .num_frag_uniforms = 2}},
        {PIPELINE_HEIGHTFIELD,
         {.vertex_shader_path = "Hud3dElement.vert",
          .fragment_shader_path = "Hud3dElement.frag",
          .num_vertex_uniforms = 1,
          .num_frag_uniforms = 1}},
        // {PIPELINE_POINTCLOUD, {}}
    };
    bool enable_msaa = false;
    SDL_GPUSampleCount msaa_samples = SDL_GPU_SAMPLECOUNT_1;
  };

  EnvironmentData &addEnvironmentObject(Mesh &&mesh, MeshData &&data,
                                        Mat4f placement) {
    return environmentMeshes.emplace_back(true, std::move(mesh),
                                          std::move(data), placement);
  }
  void setEnvironmentObjectStatus(size_t i, bool status) {
    environmentMeshes[i].status = status;
  }

  RobotScene(const Renderer &renderer, const pin::GeometryModel &geom_model,
             const pin::GeometryData &geom_data, Config config);

  static SDL_GPUGraphicsPipeline *
  createPipeline(const Device &dev, const MeshLayout &layout,
                 SDL_GPUTextureFormat render_target_format,
                 SDL_GPUTextureFormat depth_stencil_format, PipelineType type,
                 const Config::PipelineConfig &config);

  void render(
      Renderer &renderer, const Camera &cameraState,
      RenderPostCallback post_callback = [](auto *) {});
  void release();

private:
  const Device &_device;
  const pin::GeometryModel &_geomModel;
  const pin::GeometryData &_geomData;
};
static_assert(Scene<RobotScene>);

} // namespace multibody
} // namespace candlewick
