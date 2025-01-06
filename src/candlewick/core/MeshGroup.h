/// \file
/// \brief Grouped operations on sets of meshes.
#include "Mesh.h"
#include <span>

namespace candlewick {

// fwd
struct MeshData;

using MeshGroup = std::vector<Mesh>;

/// Create a group of meshes and optionally upload to device.
MeshGroup createMeshGroup(const Device &device, std::span<MeshData> meshDatas,
                          bool uploadToDevice);

void releaseMeshGroup(const Device &device, MeshGroup &group);

} // namespace candlewick
