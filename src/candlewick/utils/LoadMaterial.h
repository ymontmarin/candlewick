#pragma once

#include "../core/MaterialUniform.h"

#include "Utils.h"

#include <assimp/material.h>

namespace candlewick {

/// \brief Load our PBR material data from an assimp material.
///
/// If the aiMaterial contains, in fact, a Phong material (\ref
/// PhongMaterial), then a PBR material will be approximated.
PbrMaterial loadFromAssimpMaterial(aiMaterial *material);

} // namespace candlewick
