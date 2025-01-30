#include "Frustum.h"

#include "../Renderer.h"
#include "../Shader.h"
#include "../Camera.h"
#include "../AABB.h"
#include "../OBB.h"

namespace candlewick {

FrustumAndBoundsDebug::FrustumAndBoundsDebug(const Renderer &renderer)
    : device(renderer.device), pipeline(nullptr) {
  auto vertexShader = Shader::fromMetadata(device, "FrustumDebug.vert");
  auto fragmentShader = Shader::fromMetadata(device, "VertexColor.frag");

  SDL_GPUColorTargetDescription color_target;
  SDL_zero(color_target);
  color_target.format = renderer.getSwapchainTextureFormat();

  SDL_GPUGraphicsPipelineCreateInfo info{
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST,
      .depth_stencil_state{.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
                           .enable_depth_test = true,
                           .enable_depth_write = true},
      .target_info{.color_target_descriptions = &color_target,
                   .num_color_targets = 1,
                   .depth_stencil_format = renderer.depth_format,
                   .has_depth_stencil_target = true},
  };
  pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
}

struct alignas(16) ubo_t {
  GpuMat4 invProj;
  alignas(16) GpuMat4 mvp;
  alignas(16) GpuVec4 color;
  GpuVec3 eyePos;
};

static constexpr Uint32 NUM_VERTICES = 36u;

SDL_GPURenderPass *getDefaultRenderPass(Renderer &renderer) {
  SDL_GPUColorTargetInfo color_target;
  SDL_zero(color_target);
  color_target.texture = renderer.swapchain;
  color_target.load_op = SDL_GPU_LOADOP_LOAD;
  color_target.store_op = SDL_GPU_STOREOP_STORE;
  SDL_GPUDepthStencilTargetInfo depth_target;
  SDL_zero(depth_target);
  depth_target.texture = renderer.depth_texture;
  depth_target.load_op = SDL_GPU_LOADOP_LOAD;
  depth_target.store_op = SDL_GPU_STOREOP_STORE;
  depth_target.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
  depth_target.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
  depth_target.cycle = false;

  return SDL_BeginGPURenderPass(renderer.command_buffer, &color_target, 1,
                                &depth_target);
}

void renderImpl(Renderer &renderer, SDL_GPURenderPass *render_pass,
                SDL_GPUGraphicsPipeline *pipeline, const Mat4f &invProj,
                const Mat4f &mvp, const Float3 eyePos, const Float4 &color) {

  SDL_BindGPUGraphicsPipeline(render_pass, pipeline);

  ubo_t ubo{
      invProj,
      mvp,
      color,
      eyePos,
  };
  renderer.pushVertexUniform(0, &ubo, sizeof(ubo));

  SDL_DrawGPUPrimitives(render_pass, NUM_VERTICES, 1, 0, 0);
}

void FrustumAndBoundsDebug::renderFrustum(Renderer &renderer,
                                          const Camera &camera,
                                          const Camera &otherCam,
                                          const Float4 &color) {
  Mat4f invProj = otherCam.projection.inverse();
  GpuMat4 mvp = camera.viewProj() * otherCam.pose().matrix();
  SDL_GPURenderPass *render_pass = getDefaultRenderPass(renderer);
  renderImpl(renderer, render_pass, pipeline, invProj, mvp, otherCam.position(),
             color);
  SDL_EndGPURenderPass(render_pass);
}

void FrustumAndBoundsDebug::renderOBB(Renderer &renderer, const Camera &camera,
                                      const OBB &obb, const Float4 &color) {
  Mat4f transform = obb.toTransformationMatrix();
  Mat4f mvp = camera.viewProj() * transform;
  SDL_GPURenderPass *render_pass = getDefaultRenderPass(renderer);
  renderImpl(renderer, render_pass, pipeline, Mat4f::Identity(), mvp,
             obb.center, color);
  SDL_EndGPURenderPass(render_pass);
}

void FrustumAndBoundsDebug::renderAABB(Renderer &renderer, const Camera &camera,
                                       const AABB &aabb, const Float4 &color) {
  Mat4f transform = aabb.toTransformationMatrix();
  Mat4f mvp = camera.viewProj() * transform;
  SDL_GPURenderPass *render_pass = getDefaultRenderPass(renderer);
  renderImpl(renderer, render_pass, pipeline, Mat4f::Identity(), mvp,
             aabb.center(), color);
  SDL_EndGPURenderPass(render_pass);
}

void FrustumAndBoundsDebug::release() {
  if (pipeline)
    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}

} // namespace candlewick
