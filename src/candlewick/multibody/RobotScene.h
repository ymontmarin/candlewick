#pragma once

#include "Multibody.h"
#include "../core/Device.h"
#include "../core/Scene.h"
#include "../core/Shader.h"
#include "../core/Shape.h"
#include "../core/Uniform.h"

#include <pinocchio/multibody/geometry.hpp>

namespace candlewick::multibody {

class RobotScene {
public:
  enum PipelineType {
    PIPELINE_TRIANGLEMESH,
    PIPELINE_HEIGHTFIELD,
    PIPELINE_POINTCLOUD,
  };
  using UniformProvider = std::function<UniformPayload(pin::GeomIndex)>;
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

  std::vector<Shape> robotShapes;
  std::unordered_map<PipelineType, std::vector<pin::GeomIndex>> shapeIndices;
  std::unordered_map<PipelineType, SDL_GPUGraphicsPipeline *> pipelines;
  std::unordered_map<Uint32, UniformProvider> vertexUniformProviders,
      fragmentUniformProviders;

  RobotScene &setVertexUniform(Uint32 binding, UniformProvider &&provider) {
    vertexUniformProviders[binding] = std::move(provider);
    return *this;
  }
  RobotScene &setFragmentUniform(Uint32 binding, UniformProvider &&provider) {
    fragmentUniformProviders[binding] = std::move(provider);
    return *this;
  }

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

  RobotScene(const Renderer &renderer, const pin::GeometryModel &geom_model,
             Config config);

  static SDL_GPUGraphicsPipeline *
  createPipeline(const Device &dev, const MeshLayout &layout,
                 SDL_GPUTextureFormat render_target_format,
                 SDL_GPUTextureFormat depth_stencil_format, PipelineType type,
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

  void
  render(Renderer &renderer, RenderPostCallback post_callback = [](auto *) {});
  void release();

private:
  SDL_GPUDevice *_device;
};
static_assert(Scene<RobotScene>);

} // namespace candlewick::multibody
