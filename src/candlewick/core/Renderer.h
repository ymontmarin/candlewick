#pragma once

#include "Device.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "Mesh.h"

#include <span>
#include <SDL3/SDL_gpu.h>

namespace candlewick {

/// \brief The Renderer class provides a rendering context for a graphical
/// application.
///
/// \sa Scene
/// \sa Device
/// \sa Mesh
struct Renderer {
  Device device;
  SDL_Window *window;
  SDL_GPUTexture *swapchain;
  Texture depth_texture{NoInit};

  Renderer(NoInitT) : device(NoInit), window(nullptr), swapchain(nullptr) {}
  /// \brief Constructor without a depth format.
  Renderer(Device &&device, SDL_Window *window);
  /// \brief Constructor with a depth format. This will create a depth texture.
  Renderer(Device &&device, SDL_Window *window,
           SDL_GPUTextureFormat suggested_depth_format);

  bool initialized() { return bool(device); }

  /// Acquire the command buffer, starting a frame.
  CommandBuffer acquireCommandBuffer() {
    return CommandBuffer::acquire(device);
  }

  /// \brief Wait until swapchain is available, then acquire it.
  /// \sa acquireSwapchain()
  bool waitAndAcquireSwapchain(CommandBuffer &command_buffer);

  /// \brief Acquire GPU swapchain.
  /// \warning This can only be called from the main thread (see SDL docs for
  /// the meaning of "main thread").
  bool acquireSwapchain(CommandBuffer &command_buffer);

  bool waitForSwapchain() { return SDL_WaitForGPUSwapchain(device, window); }

  SDL_GPUTextureFormat getSwapchainTextureFormat() const {
    return SDL_GetGPUSwapchainTextureFormat(device, window);
  }

  bool hasDepthTexture() const { return depth_texture.hasValue(); }
  SDL_GPUTextureFormat depthFormat() const { return depth_texture.format(); }

  void destroy() {
    SDL_ReleaseWindowFromGPUDevice(device, window);
    depth_texture.release();
    device.destroy();
  }
};

namespace rend {

  /// \brief Bind a Mesh object.
  /// \sa bindMeshView()
  void bindMesh(SDL_GPURenderPass *pass, const Mesh &mesh);

  /// \brief Bind a MeshView object.
  /// \warning This performs the same logic as bindMesh(). The entire buffer is
  /// bound, because drawView() uses the vertex and index offsets to draw.
  /// Binding offset buffers would, then, be incorrect.
  /// \sa bindMesh()
  /// \sa drawView()
  /// \sa drawViews()
  void bindMeshView(SDL_GPURenderPass *pass, const MeshView &view);

  /// \brief Render a collection of MeshView.
  /// This collection must satisfy the invariant that they all have the same
  /// parent Mesh object, which allows batching draw calls like this without
  /// having to rebind the Mesh.
  /// \param pass The GPU render pass
  /// \param meshViews Collection of MeshView objects with the same parent Mesh.
  /// \sa drawView() (for individual views)
  void drawViews(SDL_GPURenderPass *pass, std::span<const MeshView> meshViews,
                 Uint32 numInstances = 1);

  /// Render an individual Mesh as part of a render pass, using a provided first
  /// index or vertex offset.
  /// \details This routine will bind the vertex and index buffer. Prefer one of
  /// the other routines to e.g. batch draw calls that use the same buffer
  /// bindings.
  /// \warning Call bindMesh() first!
  /// \param pass The GPU render pass
  /// \param mesh The Mesh to draw.
  inline void draw(SDL_GPURenderPass *pass, const Mesh &mesh,
                   Uint32 numInstances = 1) {
    drawViews(pass, mesh.views(), numInstances);
  }

  /// \brief Draw a MeshView.
  /// \warning Call bindMesh() first!
  /// \param pass The GPU render pass.
  /// \param mesh MeshView object to be drawn.
  /// \sa draw()
  void drawView(SDL_GPURenderPass *pass, const MeshView &mesh,
                Uint32 numInstances = 1);

  /// \brief Bind single texture-sampler pair for vertex shader.
  inline void bindVertexSampler(SDL_GPURenderPass *pass, Uint32 first_slot,
                                const SDL_GPUTextureSamplerBinding &binding) {
    SDL_BindGPUVertexSamplers(pass, first_slot, &binding, 1);
  }

  /// \brief Bind multiple fragment shader samplers.
  /// \sa bindVertexSampler()
  inline void
  bindVertexSamplers(SDL_GPURenderPass *pass, Uint32 first_slot,
                     std::span<const SDL_GPUTextureSamplerBinding> bindings) {
    SDL_BindGPUVertexSamplers(pass, first_slot, bindings.data(),
                              Uint32(bindings.size()));
  }

  /// \copybrief bindFragmentSamplers()
  /// This overload exists to enable taking a brace-initialized array.
  template <std::size_t N>
  void bindVertexSamplers(
      SDL_GPURenderPass *pass, Uint32 first_slot,
      const SDL_GPUTextureSamplerBinding (&sampler_bindings)[N]) {
    SDL_BindGPUVertexSamplers(pass, first_slot, sampler_bindings, N);
  }

  /// \brief Bind single texture-sampler pair for fragment shader.
  inline void bindFragmentSampler(SDL_GPURenderPass *pass, Uint32 first_slot,
                                  const SDL_GPUTextureSamplerBinding &binding) {
    SDL_BindGPUFragmentSamplers(pass, first_slot, &binding, 1);
  }

  /// \brief Bind multiple fragment shader samplers.
  /// \sa bindFragmentSampler()
  inline void
  bindFragmentSamplers(SDL_GPURenderPass *pass, Uint32 first_slot,
                       std::span<const SDL_GPUTextureSamplerBinding> bindings) {
    SDL_BindGPUFragmentSamplers(pass, first_slot, bindings.data(),
                                Uint32(bindings.size()));
  }

  /// \copybrief bindFragmentSamplers()
  /// This overload exists to enable taking a brace-initialized array.
  template <std::size_t N>
  void bindFragmentSamplers(
      SDL_GPURenderPass *pass, Uint32 first_slot,
      const SDL_GPUTextureSamplerBinding (&sampler_bindings)[N]) {
    SDL_BindGPUFragmentSamplers(pass, first_slot, sampler_bindings, N);
  }

} // namespace rend
} // namespace candlewick
