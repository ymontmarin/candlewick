#pragma once

#include "Utils.h"
#include <SDL3/SDL_stdinc.h>
#include <vector>

namespace candlewick {

/// Return codes for \ref loadSceneMeshes().
enum class mesh_load_retc : Uint16 {
  FAILED_TO_LOAD = 1 << 0,
  NO_MESHES = 1 << 1,

  OK = 1 << 4,
};

/// \brief Load the meshes from the given path.
/// This is implemented using the assimp library.
mesh_load_retc loadSceneMeshes(const char *path,
                               std::vector<MeshData> &meshData);
} // namespace candlewick
