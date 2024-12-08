#include "VideoRecorder.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_filesystem.h>
#include <format>

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
    if (codecContext)
      avcodec_free_context(&codecContext);
    if (formatContext->pb)
      avio_closep(&formatContext->pb);

    avformat_free_context(formatContext);

    if (frame)
      av_frame_free(&frame);
    if (packet)
      av_packet_free(&packet);
  }
};

VideoRecorder::VideoRecorder(Uint32 width, Uint32 height,
                             const std::string &filename, Settings settings)
    : m_width(width), m_height(height), m_state(new State()) {
  avformat_network_init();
  m_state->codec = avcodec_find_encoder(AV_CODEC_ID_H264);

  int ret = avformat_alloc_output_context2(&m_state->formatContext, nullptr,
                                           nullptr, filename.c_str());
  char errbuf[AV_ERROR_MAX_STRING_SIZE]{0};
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    auto msg = std::format("Could not create output context: {:s}", errbuf);
    throw std::runtime_error(msg);
  }

  m_state->videoStream =
      avformat_new_stream(m_state->formatContext, m_state->codec);

  m_state->codecContext = avcodec_alloc_context3(m_state->codec);
  auto codecContext = m_state->codecContext;

  codecContext->width = int(m_width);
  codecContext->height = int(m_height);
  codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
  codecContext->time_base = AVRational{1, settings.fps};
  codecContext->framerate = AVRational{settings.fps, 1};
  codecContext->gop_size = 10;
  codecContext->max_b_frames = 1;
  codecContext->bit_rate = settings.bit_rate;

  ret = avcodec_parameters_from_context(m_state->videoStream->codecpar,
                                        codecContext);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(
        std::format("Couldn't copy codec params: {:s}", errbuf));
  }

  ret = avcodec_open2(codecContext, m_state->codec, nullptr);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(std::format("Couldn't open codec: {:s}", errbuf));
  }

  ret =
      avio_open(&m_state->formatContext->pb, filename.c_str(), AVIO_FLAG_WRITE);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(
        std::format("Couldn't open output stream: {:s}", errbuf));
  }

  ret = avformat_write_header(m_state->formatContext, nullptr);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(
        std::format("Couldn't write output header: {:s}", errbuf));
  }

  m_state->frame = av_frame_alloc();
  m_state->packet = av_packet_alloc();
  m_state->frame->format = codecContext->pix_fmt;
  m_state->frame->width = codecContext->width;
  m_state->frame->height = codecContext->height;

  ret = av_frame_get_buffer(m_state->frame, 0);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(
        std::format("Failed to allocate frame: {:s}", errbuf));
  }
}

} // namespace candlewick::media
