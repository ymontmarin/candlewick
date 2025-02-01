#include "ScreenSpaceShadows.h"

#include "../core/math_types.h"
#include "../core/Renderer.h"
#include "../core/MeshLayout.h"
#include "../core/Shader.h"
#include "../core/Camera.h"

namespace candlewick {
namespace effects {
  ScreenSpaceShadowPass::ScreenSpaceShadowPass(const Renderer &renderer,
                                               const MeshLayout &layout,
                                               const Config &config)
      : config(config) {
    const Device &device = renderer.device;
    this->depthTexture = renderer.depth_texture;

    auto vertexShader = Shader::fromMetadata(device, "ScreenSpaceShadows.vert");
    auto fragmentShader =
        Shader::fromMetadata(device, "ScreenSpaceShadows.frag");
    SDL_GPUGraphicsPipelineCreateInfo pipeline_desc{
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state = layout.toVertexInputState(),
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state{.depth_bias_constant_factor = config.shadowBias},
        .depth_stencil_state{.compare_op = SDL_GPU_COMPAREOP_LESS,
                             .enable_depth_test = true,
                             .enable_depth_write = true},
        .target_info{.color_target_descriptions = nullptr,
                     .num_color_targets = 0,
                     .depth_stencil_format = renderer.depth_format,
                     .has_depth_stencil_target = true},
    };

    this->pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_desc);

    auto *window = renderer.window;
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    SDL_GPUTextureCreateInfo texture_desc{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D24_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET |
                 SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = Uint32(width),
        .height = Uint32(height),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
        .props = 0,
    };
    this->targetTexture = SDL_CreateGPUTexture(device, &texture_desc);
    SDL_GPUSamplerCreateInfo sampler_desc{};
    sampler_desc.min_filter = SDL_GPU_FILTER_NEAREST;
    sampler_desc.mag_filter = SDL_GPU_FILTER_NEAREST;
    sampler_desc.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_desc.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_desc.enable_compare = false;
    sampler_desc.enable_anisotropy = false;
    sampler_desc.props = 0;
    this->depthSampler = SDL_CreateGPUSampler(device, &sampler_desc);
  }

  struct alignas(16) ScreenSpaceShadowsUniform {
    GpuMat4 projection;
    GpuMat4 invProj;
    GpuVec3 viewSpaceLightDir;
    float maxDist;
    float stepSize;
    int numSteps;
    float bias;
  };

  void ScreenSpaceShadowPass::release(SDL_GPUDevice *device) noexcept {
    if (targetTexture)
      SDL_ReleaseGPUTexture(device, targetTexture);

    if (pipeline)
      SDL_ReleaseGPUGraphicsPipeline(device, pipeline);

    if (depthSampler)
      SDL_ReleaseGPUSampler(device, depthSampler);
  }

  void ScreenSpaceShadowPass::render(Renderer &renderer, const Camera &camera,
                                     const DirectionalLight &light) {
    SDL_GPUDepthStencilTargetInfo dsti{
        .texture = this->targetTexture,
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
        .cycle = false,
    };

    SDL_GPURenderPass *render_pass =
        SDL_BeginGPURenderPass(renderer.command_buffer, nullptr, 0u, &dsti);
    SDL_BindGPUGraphicsPipeline(render_pass, pipeline);

    Mat4f invProj = camera.projection.inverse();
    ScreenSpaceShadowsUniform ubo{
        camera.projection,
        invProj,
        camera.transformVector(light.direction),
        config.maxDist,
        config.stepSize,
        config.numSteps,
        config.shadowBias,
    };
    renderer.pushVertexUniform(0, &ubo, sizeof(ubo));
    renderer.bindFragmentSampler(render_pass, 0,
                                 {
                                     .texture = depthTexture,
                                     .sampler = depthSampler,
                                 });

    SDL_DrawGPUPrimitives(render_pass, 4u, 1, 0, 0);

    SDL_EndGPURenderPass(render_pass);
  }

} // namespace effects
} // namespace candlewick
