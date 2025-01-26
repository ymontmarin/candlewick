#pragma once

#include "Multibody.h"
#include "../core/Device.h"
#include "../core/Scene.h"
#include "../core/LightUniforms.h"
#include "../utils/MeshData.h"
#include "../third-party/magic_enum.hpp"

#include <entt/entity/registry.hpp>
#include <coal/fwd.hh>
#include <pinocchio/multibody/fwd.hpp>

namespace candlewick {
namespace multibody {

struct PinGeomObjComponent {
  pin::GeomIndex geom_index;
  operator pin::GeomIndex() const { return geom_index; }
};

/// \brief A visibility component for ECS.
struct VisibilityComponent {
  bool status;
  operator bool() const { return status; }
};

/// \brief A scene/render system for robot geometries using Pinocchio.
class RobotScene {
public:
  enum PipelineType {
    PIPELINE_TRIANGLEMESH,
    PIPELINE_HEIGHTFIELD,
    PIPELINE_POINTCLOUD,
  };
  static constexpr size_t kNumPipelineTypes =
      magic_enum::enum_count<PipelineType>();
  enum VertexUniformSlots : Uint32 { TRANSFORM = 0 };
  enum FragmentUniformSlots : Uint32 { MATERIAL = 0, LIGHTING = 1 };

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

  /// \brief Tag struct for denoting an entity as opaque, for render pass
  /// organization.
  struct Opaque {};
  struct TransformComponent {
    Mat4f transform;
  };

  struct MeshMaterialComponent {
    Mesh mesh;
    std::vector<PbrMaterialData> materials;
    PipelineType pipeline_type;
    MeshMaterialComponent(Mesh &&mesh, std::vector<PbrMaterialData> &&materials,
                          PipelineType pipelineType)
        : mesh(std::move(mesh)), materials(std::move(materials)),
          pipeline_type(pipelineType) {
      assert(mesh.numViews() == materials.size());
    }
  };

  entt::registry &registry;
  SDL_GPUGraphicsPipeline *renderPipelines[kNumPipelineTypes];
  DirectionalLight directionalLight;

  struct PipelineConfig {
    // shader set
    const char *vertex_shader_path;
    const char *fragment_shader_path;
    SDL_GPUCullMode cull_mode = SDL_GPU_CULLMODE_BACK;
    SDL_GPUFillMode fill_mode = SDL_GPU_FILLMODE_FILL;
  };
  struct Config {
    std::unordered_map<PipelineType, PipelineConfig> pipeline_configs = {
        {PIPELINE_TRIANGLEMESH,
         {
             .vertex_shader_path = "PbrBasic.vert",
             .fragment_shader_path = "PbrBasic.frag",
         }},
        {PIPELINE_HEIGHTFIELD,
         {
             .vertex_shader_path = "Hud3dElement.vert",
             .fragment_shader_path = "Hud3dElement.frag",
         }},
        // {PIPELINE_POINTCLOUD, {}}
    };
    bool enable_msaa = false;
    bool triangle_has_prepass = false;
    SDL_GPUSampleCount msaa_samples = SDL_GPU_SAMPLECOUNT_1;
  };

  RobotScene(entt::registry &registry, const Renderer &renderer,
             const pin::GeometryModel &geom_model,
             const pin::GeometryData &geom_data, Config config);

  entt::entity
  addEnvironmentObject(MeshData &&data, Mat4f placement,
                       PipelineType pipe_type = PIPELINE_TRIANGLEMESH);

  // void addRobot(const pin::GeometryModel &geom_model,
  //               const pin::GeometryData &geom_data);

  [[nodiscard]] static SDL_GPUGraphicsPipeline *
  createPipeline(const Device &device, const MeshLayout &layout,
                 SDL_GPUTextureFormat render_target_format,
                 SDL_GPUTextureFormat depth_stencil_format, PipelineType type,
                 const Config &config);

  void render(Renderer &renderer, const Camera &camera);
  /// \brief PBR render pass for triangle meshes.
  void renderPBRTriangleGeometry(Renderer &renderer, const Camera &camera);
  /// \brief Render pass for other geometry.
  void renderOtherGeometry(Renderer &renderer, const Camera &camera);
  void release();

  inline bool pbrHasPrepass() const { return config.triangle_has_prepass; }

  /// \brief Getter for the referenced pinocchio GeometryData object.
  const pin::GeometryData &geomData() const { return *_geomData; }

private:
  Config config;
  const Device &_device;
  pin::GeometryData const *_geomData;
};
static_assert(Scene<RobotScene>);

} // namespace multibody
} // namespace candlewick
