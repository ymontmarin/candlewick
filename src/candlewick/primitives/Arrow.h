#pragma once

#include "../utils/MeshData.h"

namespace candlewick {

MeshData createArrow(bool include_normals = false, float shaft_length = 0.4f,
                     float shaft_radius = 0.01f, float head_length = 0.1f,
                     float head_radius = 0.02f, Uint32 segments = 32);

// Create a 3D triad
std::array<MeshData, 3> createTriad(float shaft_length = 0.4f,
                                    float shaft_radius = 0.01f,
                                    float head_length = 0.1f,
                                    float head_radius = 0.02f,
                                    Uint32 segments = 32);

} // namespace candlewick
