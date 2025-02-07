#pragma once
#include "../core/math_types.h"
#include "../core/MeshLayout.h"
#include "../utils/MeshData.h"

namespace candlewick {

struct alignas(16) PosOnlyVertex {
  GpuVec3 pos;
};

template <> struct VertexTraits<PosOnlyVertex> {
  static auto layout() {
    return MeshLayout{}
        .addBinding(0, sizeof(PosOnlyVertex))
        .addAttribute("pos", 0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                      offsetof(PosOnlyVertex, pos));
  }
};

struct alignas(16) PosNormalVertex {
  GpuVec3 pos;
  alignas(16) GpuVec3 normal;
};

template <> struct VertexTraits<PosNormalVertex> {
  static auto layout() {
    return MeshLayout{}
        .addBinding(0, sizeof(PosNormalVertex))
        .addAttribute("pos", 0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                      offsetof(PosNormalVertex, pos))
        .addAttribute("normal", 1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                      offsetof(PosNormalVertex, normal));
  }
};

namespace detail {

  struct ConeCylinderBuilder {
    size_t currentVertices() const { return positions.size(); }
    void add(const Float3 &pos, const Float3 &normal);
    void addFace(const Uint32 (&face)[3]);
    Float3 previousPos(std::size_t offset) const;
    Float3 previousNormal(std::size_t offset) const;

    void addDisk(Uint32 segments, float radius, float z);
    void addCone(Uint32 segments, float radius, float zBottom, float length);
    Uint32 addCylinderFloors(Uint32 numFloors, Uint32 segments,
                             Float2 basePoint, Float2 upDir, Uint32 startIdx);

    std::vector<Float3> positions;
    std::vector<Float3> normals;
    std::vector<MeshData::IndexType> indices;
  };

  inline void ConeCylinderBuilder::add(const Float3 &pos,
                                       const Float3 &normal) {
    positions.push_back(pos);
    normals.push_back(normal.normalized());
  }

  inline void ConeCylinderBuilder::addFace(const Uint32 (&face)[3]) {
    indices.push_back(face[0]);
    indices.push_back(face[1]);
    indices.push_back(face[2]);
  }

  inline Float3 ConeCylinderBuilder::previousPos(std::size_t offset) const {
    return positions[positions.size() - offset];
  }

  inline Float3 ConeCylinderBuilder::previousNormal(std::size_t offset) const {
    return normals[normals.size() - offset];
  }

} // namespace detail
} // namespace candlewick
