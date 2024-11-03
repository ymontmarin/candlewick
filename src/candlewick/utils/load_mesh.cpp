#include "load_mesh.h"
#include "MeshData.h"

#include <SDL3/SDL_log.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace candlewick {

/// Traverse the Assimp scene recursively to build the mesh data.
/// \returns Number of vertices.
Uint32 buildMeshImpl(const aiScene *scene, const aiNode *node,
                     aiMatrix4x4 parentWorldTransform, Uint32 vertexOffset,
                     MeshData &meshData) {
  auto &vertices = meshData.vertices;
  auto &faces = meshData.faces;
  auto &normals = meshData.normals;

  Uint32 nbVtx = 0;
  if (node) {
    aiMatrix4x4 tr = parentWorldTransform * node->mTransformation;

    for (Uint32 i = 0; i < node->mNumMeshes; i++) {
      const Uint32 meshIdx = node->mMeshes[i];
      aiMesh *inMesh = scene->mMeshes[meshIdx];

      for (Uint32 vertex_id = 0; vertex_id < inMesh->mNumVertices;
           vertex_id++) {
        aiVector3D pos = inMesh->mVertices[vertex_id];
        pos = tr * pos;
        vertices.push_back(Float3::Map(&pos.x));

        if (inMesh->HasNormals()) {
          aiVector3D n_ = inMesh->mNormals[vertex_id];
          normals.push_back(Float3::Map(&n_.x));
        }
      }

      for (Uint32 fid = 0; fid < inMesh->mNumFaces; fid++) {
        aiFace &f = inMesh->mFaces[fid];
        Vec3u::MapType face_indices{f.mIndices};
        faces.push_back(face_indices.array() + vertexOffset);
      }

      nbVtx += inMesh->mNumVertices;
    }

    for (Uint32 k = 0; k < node->mNumChildren; k++) {
      const aiNode *cNode = node->mChildren[k];
      nbVtx += buildMeshImpl(scene, cNode, tr, nbVtx, meshData);
    }
  }
  return nbVtx;
}

LoadMeshReturn loadMesh(const char *path, MeshData &meshData) {

  ::Assimp::Importer import;
  // remove point primitives
  import.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
                            aiPrimitiveType_LINE | aiPrimitiveType_POINT);
  unsigned int pFlags = aiProcess_CalcTangentSpace | aiProcess_Triangulate |
                        aiProcess_GenSmoothNormals | aiProcess_SortByPType |
                        aiProcess_GenUVCoords | aiProcess_OptimizeMeshes |
                        aiProcess_FindDegenerates;
  const aiScene *scene = import.ReadFile(path, pFlags);
  if (!scene) {
    SDL_Log("%s: Warning: Failed to load resource.", __FUNCTION__);
    return LoadMeshReturn::FailedToLoad;
  }

  if (!scene->HasMeshes())
    return LoadMeshReturn::NoMeshes;

  aiMatrix4x4 rootTr = scene->mRootNode->mTransformation;
  Uint32 numVertices =
      buildMeshImpl(scene, scene->mRootNode, rootTr, 0, meshData);
  (void)numVertices;
  assert(numVertices == meshData.numVertices());

  return LoadMeshReturn::OK;
}

} // namespace candlewick
