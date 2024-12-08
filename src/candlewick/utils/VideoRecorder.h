#pragma once

#include <SDL3/SDL_gpu.h>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace candlewick {
namespace media {

class VideoRecorder {
private:
  Uint32 m_width;  //< Width of incoming frames
  Uint32 m_height; //< Height of incoming frames
  Uint32 m_frameCounter;

  AVFormatContext *formatContext = nullptr;
  const AVCodec *codec = nullptr;
  AVCodecContext *codecContext = nullptr;
  AVStream *videoStream = nullptr;
  SwsContext *swsContext = nullptr;
  AVFrame *frame = nullptr;
  AVPacket *packet = nullptr;

public:
  struct Settings {
    int fps = 30;
    long bit_rate = 2500000u;
    int outputWidth;
    int outputHeight;
  };

  VideoRecorder(Uint32 width, Uint32 height, const std::string &filename,
                Settings settings);

  VideoRecorder(Uint32 width, Uint32 height, const std::string &filename)
      : VideoRecorder(width, height, filename,
                      Settings{
                          .outputWidth = int(width),
                          .outputHeight = int(height),
                      }) {}

  Uint32 frameCounter() const { return m_frameCounter; }
  void writeFrame(const Uint8 *data, size_t payloadSize,
                  AVPixelFormat avPixelFormat);

  ~VideoRecorder() noexcept;
};

} // namespace media
} // namespace candlewick
