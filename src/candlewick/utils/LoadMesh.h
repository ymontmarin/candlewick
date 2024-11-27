#pragma once

#include <SDL3/SDL_stdinc.h>
#include <vector>

namespace candlewick {

struct MeshData;

/// Return codes for \ref loadSceneMeshes().
enum LoadMeshReturn : Uint16 {
  FailedToLoad = 1 << 0,
  NoMeshes = 1 << 1,

  OK = 1 << 4,
  HasMaterials = 1 << 5,
};

/// \brief Load the meshes from the given path.
/// This is implemented using the assimp library.
LoadMeshReturn loadSceneMeshes(const char *path,
                               std::vector<MeshData> &meshData);
} // namespace candlewick
