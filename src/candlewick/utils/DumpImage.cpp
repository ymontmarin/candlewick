#include "DumpImage.h"
#include "../core/Device.h"
#include "../third-party/stb_image_write.h"
#include "../utils/PixelFormatConversion.h"

#include <SDL3/SDL_assert.h>
#include <stdexcept>

namespace candlewick {

void dumpTextureImgToFile(const Device &device, SDL_GPUTexture *texture,
                          SDL_GPUTextureFormat format, const Uint32 width,
                          const Uint32 height, const char *filename) {

  SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(device);
  SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(command_buffer);

  SDL_GPUTextureRegion source{
      .texture = texture,
      .layer = 0,
      .w = width,
      .h = height,
  };

  // pixel size, in bytes
  Uint32 pixel_size;
  if (format == SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM) {
    pixel_size = 4;
  } else {
    throw std::runtime_error("Unsupported texture format.");
  }

  SDL_GPUTransferBuffer *download_transfer_buffer;
  {
    SDL_GPUTransferBufferCreateInfo createInfo{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD,
        .size = width * height * pixel_size,
    };
    download_transfer_buffer = SDL_CreateGPUTransferBuffer(device, &createInfo);
  }

  SDL_GPUTextureTransferInfo destination{
      .transfer_buffer = download_transfer_buffer,
      .offset = 0,
  };

  SDL_DownloadFromGPUTexture(copy_pass, &source, &destination);
  SDL_EndGPUCopyPass(copy_pass);

  SDL_GPUFence *fence =
      SDL_SubmitGPUCommandBufferAndAcquireFence(command_buffer);
  SDL_WaitForGPUFences(device, true, &fence, 1);
  SDL_ReleaseGPUFence(device, fence);

  auto *raw_pixels = reinterpret_cast<Uint32 *>(
      SDL_MapGPUTransferBuffer(device, download_transfer_buffer, false));

  Uint32 img_num_bytes = width * height * pixel_size;
  auto *rgba_pixels = (Uint32 *)std::malloc(img_num_bytes);
  if (format == SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM) {
    bgraToRgbaConvert(raw_pixels, rgba_pixels, width * height);
  } else {
    SDL_memcpy(rgba_pixels, raw_pixels, img_num_bytes);
  }

  stbi_write_png(filename, int(width), int(height), 4, rgba_pixels, 0);
  std::free(rgba_pixels);
}

} // namespace candlewick
