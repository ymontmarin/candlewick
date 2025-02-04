#include "SSAO.h"

#include "../core/Shader.h"
#include "../core/Camera.h"
#include "../core/Renderer.h"
#include <random>

namespace candlewick {
namespace ssao {

  float lerp(float a, float b, float f) { return a + f * (b - a); }

  // https://sudonull.com/post/102169-Normal-oriented-Hemisphere-SSAO-for-Dummies
  std::vector<GpuVec4> generateSsaoKernel(size_t kernelSize = 64ul) {
    std::random_device rd;
    Uint32 seed = rd();
    std::mt19937 gen{seed};
    std::uniform_real_distribution<float> rfloat{0., 1.};

    std::vector<GpuVec4> kernel;
    kernel.resize(kernelSize);

    for (size_t i = 0; i < kernelSize; i++) {
      GpuVec3 sample;
      sample.setRandom();
      sample.head<2>() *= 2.f;
      sample.head<2>().array() -= 1.f;
      sample.normalize();
      sample *= rfloat(gen);

      float scale = float(i) / float(kernelSize);
      scale = lerp(0.1f, 10.f, scale * scale);
      sample *= scale;

      kernel[i].head<3>() = sample;
      kernel[i].w() = 1.f;
    }
    return kernel;
  }

  static const std::vector KERNEL_SAMPLES = generateSsaoKernel();

  Texture create_noise_texture(const Device &device, Uint32 size) {
    return Texture{device,
                   {.type = SDL_GPU_TEXTURETYPE_2D,
                    .format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
                    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                    .width = size,
                    .height = size,
                    .layer_count_or_depth = 1,
                    .num_levels = 1,
                    .sample_count = SDL_GPU_SAMPLECOUNT_1,
                    .props = 0}};
  }

  auto generate_noise_texture_values() {
    std::vector<GpuVec4> noise;
    Uint32 noise_size = 16u;
    noise.resize(noise_size);
    std::uniform_real_distribution<float> rfloat{-1., 1.};
    std::random_device rd;
    std::mt19937 gen{rd()};

    for (size_t i = 0; i < noise_size; i++) {
      GpuVec4 sample{2.f * rfloat(gen) - 1.f, 2.f * rfloat(gen) - 1.f, 0.f,
                     0.f};
      noise[i] = sample.normalized();
    }
    return noise;
  }

  SsaoPass::SsaoNoise createSsaoNoise(const Device &dev) {
    Texture tex = create_noise_texture(dev, 4u);
    SDL_GPUSamplerCreateInfo sampler_ci{
        .min_filter = SDL_GPU_FILTER_NEAREST,
        .mag_filter = SDL_GPU_FILTER_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
    };

    return {std::move(tex), SDL_CreateGPUSampler(dev, &sampler_ci)};
  }

  bool pushSsaoNoiseData(const SsaoPass::SsaoNoise &noise) {
    auto values = generate_noise_texture_values();
    auto &tex = noise.tex;
    SDL_GPUDevice *dev = tex.device();
    SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(dev);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    auto payload_size = Uint32(values.size() * sizeof(GpuVec4));
    SDL_GPUTransferBufferCreateInfo tb_ci{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = payload_size,
        .props = 0};
    SDL_GPUTransferBuffer *texTransferBuffer =
        SDL_CreateGPUTransferBuffer(dev, &tb_ci);
    auto *data =
        (GpuVec4 *)SDL_MapGPUTransferBuffer(dev, texTransferBuffer, false);
    std::copy(values.begin(), values.end(), data);
    SDL_UnmapGPUTransferBuffer(dev, texTransferBuffer);

    SDL_EndGPUCopyPass(copy_pass);
    return SDL_SubmitGPUCommandBuffer(command_buffer);
  }

  SsaoPass::SsaoPass(const Renderer &renderer, const MeshLayout &layout,
                     SDL_GPUTexture *normalMap)
      : inDepthMap(renderer.depth_texture), inNormalMap(normalMap) {
    const auto &device = renderer.device;

    SDL_GPUSamplerCreateInfo samplers_ci{
        .min_filter = SDL_GPU_FILTER_NEAREST,
        .mag_filter = SDL_GPU_FILTER_NEAREST,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .compare_op = SDL_GPU_COMPAREOP_NEVER,
    };
    texSampler = SDL_CreateGPUSampler(device, &samplers_ci);

    int width, height;
    SDL_GetWindowSize(renderer.window, &width, &height);
    SDL_GPUTextureCreateInfo texture_desc{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R32_FLOAT,
        .usage =
            SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = Uint32(width),
        .height = Uint32(height),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
        .props = 0,
    };
    ssaoMap = Texture{device, texture_desc, "SSAO Map"};

    auto vertexShader = Shader::fromMetadata(device, "DrawQuad.vert");
    auto fragmentShader = Shader::fromMetadata(device, "SSAO.frag");
    SDL_GPUColorTargetDescription color_desc;
    SDL_zero(color_desc);
    // render AO map to 16-bit float texture
    color_desc.format = texture_desc.format;
    SDL_GPUGraphicsPipelineCreateInfo pipeline_desc{
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state = layout.toVertexInputState(),
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state{.fill_mode = SDL_GPU_FILLMODE_FILL,
                          .cull_mode = SDL_GPU_CULLMODE_BACK},
        .target_info{.color_target_descriptions = &color_desc,
                     .num_color_targets = 1,
                     .has_depth_stencil_target = false},
    };
    pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_desc);

    // Now, we create the noise texture
    ssaoNoise = createSsaoNoise(device);
    pushSsaoNoiseData(ssaoNoise);
  }

  void SsaoPass::render(Renderer &renderer, const Camera &camera) {
    GpuMat4 proj = camera.projection;
    SDL_GPUColorTargetInfo color_info{
        .texture = ssaoMap,
        .layer_or_depth_plane = 0,
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
    };
    SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
        renderer.command_buffer, &color_info, 1, nullptr);

    renderer.bindFragmentSampler(
        render_pass, 0, {.texture = inDepthMap, .sampler = texSampler});
    renderer.bindFragmentSampler(
        render_pass, 1, {.texture = inNormalMap, .sampler = texSampler});
    // renderer.bindFragmentSampler(
    //     render_pass, 2,
    //     {.texture = ssaoNoise.tex, .sampler = ssaoNoise.sampler});
    auto SAMPLES_PAYLOAD_BYTES =
        Uint32(KERNEL_SAMPLES.size() * sizeof(GpuVec4));
    renderer.pushFragmentUniform(0, KERNEL_SAMPLES.data(),
                                 SAMPLES_PAYLOAD_BYTES);
    renderer.pushFragmentUniform(1, &proj, sizeof(proj));
    SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
    SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
    SDL_EndGPURenderPass(render_pass);
  }

  void SsaoPass::release() {
    auto *device = ssaoMap.device();
    // release neither input texture because they are **borrowed**.

    if (texSampler)
      SDL_ReleaseGPUSampler(device, texSampler);
    if (pipeline)
      SDL_ReleaseGPUGraphicsPipeline(device, pipeline);

    ssaoMap.release();

    ssaoNoise.tex.release();
    if (ssaoNoise.sampler)
      SDL_ReleaseGPUSampler(device, ssaoNoise.sampler);
  }

} // namespace ssao
} // namespace candlewick
