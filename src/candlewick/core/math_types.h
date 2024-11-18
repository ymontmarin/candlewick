#pragma once
#include <Eigen/Core>

namespace candlewick {

using Float2 = Eigen::Vector2f;
using Float3 = Eigen::Vector3f;
using Float4 = Eigen::Vector4f;
using GpuVec3 = Eigen::Matrix<float, 3, 1, Eigen::DontAlign>;
using GpuVec4 = Eigen::Matrix<float, 4, 1, Eigen::DontAlign>;
using GpuMat3 = Eigen::Matrix<float, 3, 3, Eigen::ColMajor | Eigen::DontAlign>;
using GpuMat4 = Eigen::Matrix<float, 4, 4, Eigen::ColMajor | Eigen::DontAlign>;

} // namespace candlewick
