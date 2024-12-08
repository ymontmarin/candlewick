#include "VideoRecorder.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_filesystem.h>
#include <format>

namespace candlewick::media {

VideoRecorder::~VideoRecorder() noexcept {
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

VideoRecorder::VideoRecorder(Uint32 width, Uint32 height,
                             const std::string &filename, Settings settings)
    : m_width(width), m_height(height) {
  avformat_network_init();
  codec = avcodec_find_encoder(AV_CODEC_ID_H264);

  int ret = avformat_alloc_output_context2(&formatContext, nullptr, nullptr,
                                           filename.c_str());
  char errbuf[AV_ERROR_MAX_STRING_SIZE]{0};
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    auto msg = std::format("Could not create output context: {:s}", errbuf);
    throw std::runtime_error(msg);
  }

  videoStream = avformat_new_stream(formatContext, codec);

  codecContext = avcodec_alloc_context3(codec);

  codecContext->width = settings.outputWidth;
  codecContext->height = settings.outputHeight;
  codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
  codecContext->time_base = AVRational{1, settings.fps};
  codecContext->framerate = AVRational{settings.fps, 1};
  codecContext->gop_size = 10;
  codecContext->max_b_frames = 1;
  codecContext->bit_rate = settings.bit_rate;

  ret = avcodec_parameters_from_context(videoStream->codecpar, codecContext);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(
        std::format("Couldn't copy codec params: {:s}", errbuf));
  }

  ret = avcodec_open2(codecContext, codec, nullptr);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(std::format("Couldn't open codec: {:s}", errbuf));
  }

  ret = avio_open(&formatContext->pb, filename.c_str(), AVIO_FLAG_WRITE);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(
        std::format("Couldn't open output stream: {:s}", errbuf));
  }

  ret = avformat_write_header(formatContext, nullptr);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(
        std::format("Couldn't write output header: {:s}", errbuf));
  }

  frame = av_frame_alloc();
  packet = av_packet_alloc();
  frame->format = codecContext->pix_fmt;
  frame->width = codecContext->width;
  frame->height = codecContext->height;

  ret = av_frame_get_buffer(frame, 0);
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(
        std::format("Failed to allocate frame: {:s}", errbuf));
  }
}

void VideoRecorder::writeFrame(const Uint8 *data, size_t payloadSize,
                               AVPixelFormat avPixelFormat) {
  AVFrame *tmpFrame = av_frame_alloc();
  tmpFrame->format = codecContext->pix_fmt;
  tmpFrame->width = int(m_width);
  tmpFrame->height = int(m_height);

  int ret = av_frame_get_buffer(tmpFrame, 0);
  char errbuf[AV_ERROR_MAX_STRING_SIZE]{0};
  if (ret < 0) {
    av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
    throw std::runtime_error(
        std::format("Failed to allocate frame: {:s}", errbuf));
  }

  SDL_memcpy(tmpFrame->data[0], data, payloadSize);

  swsContext =
      sws_getContext(tmpFrame->width, tmpFrame->height, avPixelFormat,
                     frame->width, frame->height, codecContext->pix_fmt,
                     SWS_BILINEAR, nullptr, nullptr, nullptr);

  frame->pts = m_frameCounter++;

  sws_scale(swsContext, tmpFrame->data, tmpFrame->linesize, 0, m_height,
            frame->data, frame->linesize);

  ret = avcodec_send_frame(codecContext, frame);
  if (ret < 0) {
    av_frame_free(&tmpFrame);
    sws_freeContext(swsContext);
    throw std::runtime_error(std::format("Error sending frame {:s}", errbuf));
  }

  while (ret >= 0) {
    ret = avcodec_receive_packet(codecContext, packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      break;
    }
    if (ret < 0) {
      av_frame_free(&tmpFrame);
      sws_freeContext(swsContext);
      throw std::runtime_error("Error receiving packet from encoder");
    }

    av_packet_rescale_ts(packet, codecContext->time_base,
                         videoStream->time_base);
    packet->stream_index = videoStream->index;
    av_interleaved_write_frame(formatContext, packet);
    av_packet_unref(packet);
  }

  av_frame_free(&tmpFrame);
  sws_freeContext(swsContext);
}

} // namespace candlewick::media
