#pragma once

#include "Utils.h"
#include "VertexDataBlob.h"
#include "../core/MaterialUniform.h"
#include "../core/Tags.h"
#include <SDL3/SDL_gpu.h>

namespace candlewick {

template <typename Derived> struct MeshDataBase {
  Derived &derived() { return static_cast<Derived &>(*this); }
  const Derived &derived() const { return static_cast<const Derived &>(*this); }

  Uint32 numVertices() const {
    return static_cast<Uint32>(derived().vertexData.size());
  }
  Uint32 numIndices() const {
    return static_cast<Uint32>(derived().indexData.size());
  }
  bool isIndexed() const { return numIndices() > 0; }
};

struct MeshData : MeshDataBase<MeshData> {
  using IndexType = Uint32;
  SDL_GPUPrimitiveType primitiveType; //< Geometry primitive for the mesh
  VertexDataBlob vertexData;          //< Vertices
  std::vector<IndexType> indexData;   //< Indices for indexed mesh. Optional.
  PbrMaterial material;               //< PBR material

  const MeshLayout &layout() const { return vertexData.layout(); }

  explicit MeshData(NoInitT);
  template <IsVertexType VertexT>
  explicit MeshData(SDL_GPUPrimitiveType primitiveType,
                    std::vector<VertexT> vertexData,
                    std::vector<IndexType> indexData);
  explicit MeshData(SDL_GPUPrimitiveType primitiveType, MeshLayout layout,
                    std::vector<char> vertexData,
                    std::vector<IndexType> indexData);
  MeshData(MeshData &&) noexcept = default;
  MeshData &operator=(MeshData &&) noexcept = default;
  MeshData &operator=(const MeshData &) noexcept = delete;

  /// \brief Explicit copy function, uses private copy ctor.
  static MeshData copy(const MeshData &other) { return MeshData{other}; };

private:
  MeshData(const MeshData &) = default;
};

template <IsVertexType VertexT>
MeshData::MeshData(SDL_GPUPrimitiveType primitiveType,
                   std::vector<VertexT> vertexData,
                   std::vector<IndexType> indexData)
    : primitiveType(primitiveType), vertexData(std::move(vertexData)),
      indexData(std::move(indexData)) {}

/// \brief Convert MeshData to a GPU Mesh object. This creates the
/// required vertex buffer and index buffer (if required).
/// \warning This does *not* upload the mesh data to the vertex and index
/// buffers.
/// \sa uploadMeshToDevice()
[[nodiscard]] Mesh createMesh(const Device &device, const MeshData &meshData,
                              bool upload = false);

/// \brief Create a Mesh object from given mesh data, as a view into existing
/// vertex and index buffers.
/// \warning The constructed Mesh will **take ownership** of the buffers.
[[nodiscard]] Mesh createMesh(const Device &device, const MeshData &meshData,
                              SDL_GPUBuffer *vertexBuffer,
                              SDL_GPUBuffer *indexBuffer);

/// \brief Create a Mesh from a batch of MeshData.
/// \param[in] device GPU device
/// \param[in] meshDatas Batch of meshes
/// \param[in] upload Whether to upload the resulting Mesh to the device.
/// \sa createMesh()
/// \sa uploadMeshToDevice()
[[nodiscard]] Mesh createMeshFromBatch(const Device &device,
                                       std::span<const MeshData> meshDatas,
                                       bool upload);

/// \brief Upload the contents of a single, individual mesh to the GPU device.
///
/// This will upload the mesh data through a MeshView.
void uploadMeshToDevice(const Device &device, const MeshView &meshView,
                        const MeshData &meshData);

void uploadMeshToDevice(const Device &device, const Mesh &mesh,
                        const MeshData &meshData);

inline std::vector<PbrMaterial>
extractMaterials(std::span<const MeshData> meshDatas) {
  std::vector<PbrMaterial> out;
  out.reserve(meshDatas.size());
  for (size_t i = 0; i < meshDatas.size(); i++) {
    out.push_back(meshDatas[i].material);
  }
  return out;
}

} // namespace candlewick
