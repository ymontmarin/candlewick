#pragma once

#include "math_types.h"

namespace candlewick {

struct alignas(16) TransformUniformData {
  GpuMat4 modelView;
  alignas(16) GpuMat4 mvp;
  alignas(16) GpuMat3 normalMatrix;
};

} // namespace candlewick
