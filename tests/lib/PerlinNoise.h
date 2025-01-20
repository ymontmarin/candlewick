#pragma once
#include <array>
#include <SDL3/SDL_stdinc.h>

/// \brief A 2D perlin noise generator
struct PerlinNoise {
  explicit PerlinNoise(Uint32 seed = 1);

  // 2D Perlin noise
  float noise(float x, float y) const;

  // permutation array
  std::array<Uint64, 512ul> perm;

  static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

  static float lerp(float t, float a, float b) { return a + t * (b - a); }

  static float grad(Uint64 hash, float x, float y) {
    // Convert low 4 bits of hash code into 8 gradient directions
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
  }
};
