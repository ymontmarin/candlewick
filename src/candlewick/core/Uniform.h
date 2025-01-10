#pragma once

#include <SDL3/SDL_stdinc.h>
#include <vector>
#include <cstring>

namespace candlewick {

struct UniformPayload {
  auto size() const { return uint32_t(_data.size()); }
  void *data() { return _data.data(); }

  template <typename U> UniformPayload(U &&value) : _data{} {
    _data.resize(sizeof(U));
    std::memcpy(_data.data(), &value, sizeof(U));
  }
  std::vector<uint8_t> _data;
};

} // namespace candlewick
