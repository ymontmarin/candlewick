#pragma once

namespace candlewick {

struct MeshData;

enum class LoadMeshReturn { OK, FailedToLoad, NoMeshes };

LoadMeshReturn loadMesh(const char *path, MeshData &meshData);
} // namespace candlewick
