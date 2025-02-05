#pragma once

#include "Core.h"
#include "Tags.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

struct Texture {
  Texture(NoInitT);
  Texture(const Device &device, SDL_GPUTextureCreateInfo texture_desc,
          const char *name = nullptr);

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;
  Texture(Texture &&other) noexcept;
  Texture &operator=(Texture &&other) noexcept;

  operator SDL_GPUTexture *() const noexcept { return _texture; }

  bool hasValue() const { return bool(_texture); }
  const auto &description() const { return _description; }
  SDL_GPUTextureType type() const { return _description.type; }
  SDL_GPUTextureFormat format() const { return _description.format; }
  SDL_GPUTextureUsageFlags usage() const { return _description.usage; }
  Uint32 width() const { return _description.width; }
  Uint32 height() const { return _description.height; }
  Uint32 depth() const { return _description.layer_count_or_depth; }
  Uint32 layerCount() const { return _description.layer_count_or_depth; }

  SDL_GPUBlitRegion blitRegion(Uint32 offset_x, Uint32 y_offset,
                               Uint32 layer_or_depth_plane = 0) const;

  Uint32 textureSize() const;

  SDL_GPUDevice *device() const { return _device; }

  void release() noexcept;

private:
  SDL_GPUTexture *_texture;
  SDL_GPUDevice *_device;
  SDL_GPUTextureCreateInfo _description;
};

inline void Texture::release() noexcept {
  if (_device && _texture) {
    SDL_ReleaseGPUTexture(_device, _texture);
    _texture = nullptr;
    _device = nullptr;
  }
}
} // namespace candlewick
