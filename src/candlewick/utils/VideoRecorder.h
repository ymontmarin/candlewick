#pragma once

#include "../core/Core.h"
#include <SDL3/SDL_gpu.h>
#include <memory>
#include <string>

namespace candlewick {
namespace media {

class VideoRecorder {
private:
  Uint32 m_width;
  Uint32 m_height;
  Uint32 m_frameCounter;

  struct State;
  std::unique_ptr<State> m_state;

public:
  struct Settings {
    int fps = 30;
    long bit_rate = 2500000u;
  };

  VideoRecorder(Uint32 width, Uint32 height, const std::string &filename,
                Settings settings);

  VideoRecorder(Uint32 width, Uint32 height, const std::string &filename)
      : VideoRecorder(width, height, filename, Settings{}) {}

  Uint32 frameCounter() const { return m_frameCounter; }
};

} // namespace media
} // namespace candlewick
