#pragma once

#include "Core.h"

namespace candlewick {
struct MeshData;

/// \brief Upload mesh contents to GPU device.
void uploadMesh(const Device &device, const Mesh &mesh,
                const MeshData &meshData);

} // namespace candlewick
