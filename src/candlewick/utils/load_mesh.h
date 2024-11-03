#pragma once

#include <vector>

namespace candlewick {

struct MeshData;

enum class LoadMeshReturn { OK, FailedToLoad, NoMeshes };

LoadMeshReturn loadSceneMeshes(const char *path,
                               std::vector<MeshData> &meshData);
} // namespace candlewick
