#include "Mesh.h"
#include <span>

namespace candlewick {

// fwd
struct MeshData;

/// \brief A grouping of meshes with single shared vertex and index buffers.
struct MeshGroup {
  SDL_GPUBuffer *masterVertexBuffer;
  SDL_GPUBuffer *masterIndexBuffer;
  std::vector<Mesh> meshes;
  std::span<MeshData> meshDatas;

  MeshGroup(const Device &device, std::span<MeshData> meshDatas);
  void releaseBuffers(const Device &device);
  size_t size() const { return meshes.size(); }
};

void uploadMeshGroupToDevice(const Device &device, const MeshGroup &group);

} // namespace candlewick
