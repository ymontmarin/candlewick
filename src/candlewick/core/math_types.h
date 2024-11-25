#pragma once
#include <Eigen/Core>

namespace candlewick {

using Float2 = Eigen::Vector2f;
using Float3 = Eigen::Vector3f;
using Float4 = Eigen::Vector4f;

/** GPU typedefs and adapters **/

using GpuVec3 = Eigen::Matrix<float, 3, 1, Eigen::DontAlign>;
using GpuVec4 = Eigen::Matrix<float, 4, 1, Eigen::DontAlign>;
// adapter type instead of typedef, to fit GLSL layout
struct GpuMat3 {
  /* implicit */ GpuMat3(Eigen::Matrix3f value) : _data() {
    _data.topRows<3>() = value;
  }
  operator auto &() { return _data; }
  operator const auto &() const { return _data; }

private:
  Eigen::Matrix<float, 4, 3, Eigen::DontAlign> _data;
};
using GpuMat4 = Eigen::Matrix<float, 4, 4, Eigen::ColMajor | Eigen::DontAlign>;

template <typename T> struct Rad {
  constexpr Rad(T value) : _value(value) {}
  constexpr operator T() const { return _value; }

  T _value;
};

template <typename T>
constexpr Rad<T> operator*(const Rad<T> &left, const T &right) {
  return Rad<T>{T(left) * right};
}

template <typename T>
constexpr Rad<T> operator*(const T &left, const Rad<T> &right) {
  return Rad<T>{left * T(right)};
}

} // namespace candlewick
