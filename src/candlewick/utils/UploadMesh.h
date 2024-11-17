#pragma once

#include "../core/Core.h"

namespace candlewick {
struct MeshData;

/// \brief Upload mesh contents to GPU device.
void uploadMeshToDevice(const Device &device, const Mesh &mesh,
                        const MeshData &meshData);

} // namespace candlewick
