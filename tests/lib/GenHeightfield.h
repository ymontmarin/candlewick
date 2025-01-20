#pragma once

#include "PerlinNoise.h"
#include <coal/hfield.h>

/// \brief Generate a heightfield using Perlin noise.
inline auto generatePerlinNoiseHeightfield(Uint32 seed, Uint32 nx,
                                           float x_dim) {
  PerlinNoise noise{seed};

  const Uint32 ny = nx;
  const float y_dim = x_dim;
  Eigen::MatrixXf heights{nx, ny};

  float x, y;

  for (Uint32 i = 0; i < nx; i++) {
    x = float(i) * x_dim;
    for (Uint32 j = 0; j < ny; j++) {
      y = float(j) * y_dim;
      heights(i, j) = noise.noise(x, y);
    }
  }
  return std::make_shared<coal::HeightField<coal::AABB>>(
      x_dim, y_dim, heights.cast<double>());
}
