#pragma once

#include "Multibody.h"
#include "../utils/MeshData.h"

#include <pinocchio/multibody/geometry-object.hpp>

namespace candlewick::multibody {

/// \brief Load an invidual Pinocchio GeometryObject's component geometries into
/// an array of \c MeshData.
void loadGeometryObject(const pin::GeometryObject &gobj,
                        std::vector<MeshData> &meshData);

/// \brief Load the component geometries of a Pinocchio GeometryModel (e.g. the
/// visual geometry) into an array of \c MeshData.
/// \sa loadGeometryObject() (loads invidual \c pinocchio::GeometryObject
/// objects).
void loadGeometryModel(const pin::GeometryModel &model,
                       std::vector<MeshData> &meshData);

} // namespace candlewick::multibody
