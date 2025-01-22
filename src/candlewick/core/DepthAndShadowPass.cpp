#include "DepthAndShadowPass.h"
#include "Renderer.h"
#include "Shader.h"
#include "BoundingBox.h"
#include "CameraControl.h"

#include <stdexcept>
#include <format>

namespace candlewick {

DepthPassInfo DepthPassInfo::create(const Renderer &renderer,
                                    const MeshLayout &layout,
                                    SDL_GPUTexture *depth_texture) {
  if (depth_texture == nullptr)
    depth_texture = renderer.depth_texture;
  const Device &device = renderer.device;
  auto vertexShader = Shader::fromMetadata(device, "ShadowCast.vert");
  auto fragmentShader = Shader::fromMetadata(device, "ShadowCast.frag");
  SDL_GPUGraphicsPipelineCreateInfo pipeline_desc{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .vertex_input_state = layout.toVertexInputState(),
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .rasterizer_state{.fill_mode = SDL_GPU_FILLMODE_FILL,
                        .cull_mode = SDL_GPU_CULLMODE_BACK},
      .depth_stencil_state{.compare_op = SDL_GPU_COMPAREOP_LESS,
                           .enable_depth_test = true,
                           .enable_depth_write = true},
      .target_info{.color_target_descriptions = nullptr,
                   .num_color_targets = 0,
                   .depth_stencil_format = renderer.depth_format,
                   .has_depth_stencil_target = true},
  };
  auto *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_desc);
  return {depth_texture, pipeline};
}

DepthPassInfo createShadowPass(const Renderer &renderer,
                               const MeshLayout &layout,
                               const ShadowPassConfig &config) {
  const Device &device = renderer.device;

  // TEXTURE
  // 2k x 2k texture
  SDL_GPUTextureCreateInfo texInfo{
      .type = SDL_GPU_TEXTURETYPE_2D,
      .format = renderer.depth_format,
      .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET |
               SDL_GPU_TEXTUREUSAGE_SAMPLER,
      .width = config.width,
      .height = config.height,
      .layer_count_or_depth = 1,
      .num_levels = 1,
      .sample_count = SDL_GPU_SAMPLECOUNT_1,
      .props = 0,
  };

  SDL_GPUTexture *shadowMap = SDL_CreateGPUTexture(device, &texInfo);
  SDL_SetGPUTextureName(device, shadowMap, "Shadow map");
  if (!shadowMap) {
    auto msg =
        std::format("Failed to create shadow map texture: %s", SDL_GetError());
    throw std::runtime_error(msg);
  }

  auto passInfo = DepthPassInfo::create(renderer, layout, shadowMap);
  if (!passInfo.pipeline) {
    SDL_ReleaseGPUTexture(device, shadowMap);
    auto msg =
        std::format("Failed to create shadow map pipeline: %s", SDL_GetError());
    throw std::runtime_error(msg);
  }

  return passInfo;
}

void renderDepthOnlyPass(Renderer &renderer, const DepthPassInfo &passInfo,
                         const GpuMat4 &viewProj,
                         std::span<const OpaqueCastable> castables) {
  SDL_GPUDepthStencilTargetInfo depth_info;
  SDL_zero(depth_info);
  depth_info.load_op = SDL_GPU_LOADOP_CLEAR;
  depth_info.store_op = SDL_GPU_STOREOP_STORE;
  depth_info.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
  depth_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
  depth_info.clear_depth = 1.0f;
  // depth texture may be the renderer shared depth texture,
  // or a specially created texture.
  depth_info.texture = passInfo.depthTexture;

  SDL_GPURenderPass *render_pass =
      SDL_BeginGPURenderPass(renderer.command_buffer, nullptr, 0, &depth_info);

  assert(passInfo.pipeline);
  SDL_BindGPUGraphicsPipeline(render_pass, passInfo.pipeline);

  GpuMat4 mvp;
  for (auto &cs : castables) {
    assert(validateMeshView(cs.mesh));
    renderer.bindMeshView(render_pass, cs.mesh);
    mvp.noalias() = viewProj * cs.transform;
    renderer.pushVertexUniform(DepthPassInfo::TRANSFORM_SLOT, &mvp,
                               sizeof(mvp));
    renderer.drawView(render_pass, cs.mesh);
  }

  SDL_EndGPURenderPass(render_pass);
}

static Mat4f orthoFromAABB(const AABB &sceneBounds) {
  return orthographicMatrix(sceneBounds.min.x(), sceneBounds.max.x(),
                            sceneBounds.min.y(), sceneBounds.max.y(),
                            sceneBounds.min.z(), sceneBounds.max.z());
}

void renderShadowPass(Renderer &renderer, const DepthPassInfo &shadowPass,
                      const DirectionalLight &dirLight,
                      std::span<const OpaqueCastable> castables,
                      const AABB &sceneBounds) {
  // not sure if - or +
  Float3 eye = dirLight.direction;
  GpuMat4 lightView = lookAt(eye, Float3::Zero());
  GpuMat4 lightProj = orthoFromAABB(sceneBounds);
  GpuMat4 viewProj = lightProj * lightView;

  renderDepthOnlyPass(renderer, shadowPass, viewProj, castables);
}
} // namespace candlewick
