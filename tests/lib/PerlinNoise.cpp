#include "PerlinNoise.h"
#include <random>
#include <algorithm>

PerlinNoise::PerlinNoise(uint32_t seed) {
  // Initialize permutation array with indices
  for (auto i = 0ul; i < 256ul; i++) {
    perm[i] = i;
  }

  // Shuffle with given seed
  std::mt19937 engine(seed);
  std::shuffle(std::begin(perm), std::begin(perm) + 256, engine);

  // Duplicate array to avoid index wrapping
  for (auto i = 0ul; i < 256ul; i++) {
    perm[256ul + i] = perm[i];
  }
}

float PerlinNoise::noise(float x, float y) const {
  // Find unit square that contains point
  auto X = static_cast<Uint64>(std::floor(x)) & 255ul;
  auto Y = static_cast<Uint64>(std::floor(y)) & 255ul;

  // Find relative x,y of point in square
  x -= std::floor(x);
  y -= std::floor(y);

  // Compute fade curves for x,y
  float u = fade(x);
  float v = fade(y);

  // Hash coordinates of cube corners
  Uint64 A = perm[X] + Y;
  Uint64 AA = perm[A];
  Uint64 AB = perm[A + 1];
  Uint64 B = perm[X + 1] + Y;
  Uint64 BA = perm[B];
  Uint64 BB = perm[B + 1];

  // Interpolate gradients
  return lerp(v, lerp(u, grad(perm[AA], x, y), grad(perm[BA], x - 1, y)),
              lerp(u, grad(perm[AB], x, y - 1), grad(perm[BB], x - 1, y - 1)));
}
