#pragma once

#include <vector>

namespace candlewick {

struct MeshData;

/// Return codes for \ref loadSceneMeshes().
enum class LoadMeshReturn { OK, FailedToLoad, NoMeshes };

/// \brief Load the meshes from the given path.
/// This is implemented using the assimp library.
LoadMeshReturn loadSceneMeshes(const char *path,
                               std::vector<MeshData> &meshData);
} // namespace candlewick
