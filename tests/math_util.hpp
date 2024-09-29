#pragma once

#include <Eigen/Core>

using Float2 = Eigen::Vector2f;
using Float3 = Eigen::Vector3f;
using Float4 = Eigen::Vector4f;

Eigen::Matrix4f lookAt(const Float3 &eye, const Float3 &center,
                       const Float3 &up);

/// Compute perspective matrix, from clipping plane parameters
/// (left, right, top, bottom).
Eigen::Matrix4f perspectiveMatrix(float left, float right, float top,
                                  float bottom, float near, float far);

Eigen::Matrix4f orthographicMatrix(const Float2 &size, float, float far);
