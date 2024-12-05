#pragma once

#include "../core/Core.h"
#include <SDL3/SDL_gpu.h>
#include <memory>

namespace candlewick {
namespace media {

class VideoRecorder {
private:
  int m_width;
  int m_height;
  Uint32 m_frameCounter;

  struct State;
  std::unique_ptr<State> m_state;

public:
  VideoRecorder(Uint32 width, Uint32 height, const std::string &filename,
                int fps);

  Uint32 frameCounter() const { return m_frameCounter; }
};

} // namespace media
} // namespace candlewick
