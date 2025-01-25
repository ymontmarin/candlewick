#include "Frustum.h"

#include "../Renderer.h"
#include "../Shader.h"
#include "../CameraControl.h"
#include "../OBB.h"

namespace candlewick {

FrustumAndBoundsDebug FrustumAndBoundsDebug::create(const Renderer &renderer) {
  const auto &device = renderer.device;
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
  return {SDL_CreateGPUGraphicsPipeline(device, &info)};
}

struct alignas(16) ubo_t {
  GpuMat4 invProj;
  alignas(16) GpuMat4 mvp;
  alignas(16) GpuVec4 color;
};

void render_impl(Renderer &renderer, const FrustumAndBoundsDebug &passInfo,
                 const Mat4f &invProj, const Mat4f &mvp, const Float4 &color) {
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

  SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
      renderer.command_buffer, &color_target, 1, &depth_target);

  SDL_BindGPUGraphicsPipeline(render_pass, passInfo.pipeline);

  ubo_t ubo{
      invProj,
      mvp,
      color,
  };
  renderer.pushVertexUniform(0, &ubo, sizeof(ubo));

  SDL_DrawGPUPrimitives(render_pass, 24, 1, 0, 0);

  SDL_EndGPURenderPass(render_pass);
}

void FrustumAndBoundsDebug::renderLightFrustum(Renderer &renderer,
                                               const Camera &camera,
                                               const Camera &otherCam,
                                               const Float4 &color) {
  Mat4f invProj = otherCam.projection.inverse();
  GpuMat4 mvp = camera.viewProj() * otherCam.pose().matrix();
  render_impl(renderer, *this, invProj, mvp, color);
}

void FrustumAndBoundsDebug::renderOBB(Renderer &renderer, const Camera &camera,
                                      const OBB &obb, const Float4 &color) {
  Mat4f transform = obb.toTransformationMatrix();
  Mat4f mvp = camera.viewProj() * transform;
  render_impl(renderer, *this, Mat4f::Identity(), mvp, color);
}

void FrustumAndBoundsDebug::renderBounds(Renderer &renderer,
                                         const Camera &camera, const AABB &aabb,
                                         const Float4 &color) {
  Mat4f transform = aabb.toTransformationMatrix();
  Mat4f mvp = camera.viewProj() * transform;
  render_impl(renderer, *this, Mat4f::Identity(), mvp, color);
}

void FrustumAndBoundsDebug::release(const Device &device) {
  if (pipeline)
    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}

} // namespace candlewick
