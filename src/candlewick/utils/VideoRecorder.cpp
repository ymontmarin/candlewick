#include "VideoRecorder.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_filesystem.h>

namespace candlewick::media {

struct VideoRecorder::State {
  AVFormatContext *formatContext = nullptr;
  const AVCodec *codec = nullptr;
  AVCodecContext *codecContext = nullptr;
  AVStream *videoStream = nullptr;
  SwsContext *swsContext = nullptr;
  AVFrame *frame = nullptr;
  AVPacket *packet = nullptr;

  ~State() {
    av_write_trailer(formatContext);
    if (codecContext) {
      avcodec_free_context(&codecContext);
    }
    if (formatContext->pb) {
      avio_closep(&formatContext->pb);
    }

    avformat_free_context(formatContext);

    if (frame)
      av_frame_free(&frame);
    if (packet)
      av_packet_free(&packet);
  }
};

VideoRecorder::VideoRecorder(Uint32 width, Uint32 height,
                             const std::string &filename, int fps)
    : m_width(int(width)), m_height(int(height)), m_state(new State()) {
  avformat_network_init();
  m_state->codec = avcodec_find_encoder(AV_CODEC_ID_H264);

  int ret = avformat_alloc_output_context2(&m_state->formatContext, nullptr,
                                           nullptr, filename.c_str());
  if (ret < 0) {
    char errbuf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error("Could not create output context:" +
                             std::string(errbuf));
  }

  m_state->videoStream =
      avformat_new_stream(m_state->formatContext, m_state->codec);

  auto &codecContext = m_state->codecContext;
  codecContext = avcodec_alloc_context3(m_state->codec);

  codecContext->width = m_width;
  codecContext->height = m_height;
  codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
  codecContext->time_base = AVRational{1, fps};
  codecContext->framerate = AVRational{fps, 1};
  codecContext->gop_size = 10;
  codecContext->max_b_frames = 1;
  codecContext->bit_rate = 2500000;

  ret = avcodec_parameters_from_context(m_state->videoStream->codecpar,
                                        codecContext);
  if (ret < 0) {
    throw std::runtime_error("Couldn't copy codec params.");
  }
}

} // namespace candlewick::media
