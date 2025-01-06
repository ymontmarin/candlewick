#pragma once
#include <Eigen/Core>

namespace candlewick {

using Float2 = Eigen::Vector2f;
using Float3 = Eigen::Vector3f;
using Float4 = Eigen::Vector4f;
using Vec3u8 = Eigen::Matrix<uint8_t, 3, 1>;
using Vec4u8 = Eigen::Matrix<uint8_t, 4, 1>;

/** GPU typedefs and adapters **/

using GpuVec2 = Eigen::Matrix<float, 2, 1, Eigen::DontAlign>;
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

float deg2rad(float);
float rad2deg(float);

template <typename T> struct Rad;
template <typename T> struct Deg;

template <typename T> struct Rad {
  constexpr Rad(T value) : _value(value) {}
  explicit constexpr Rad(Deg<T> value) : _value(deg2rad(value)) {}
  constexpr operator T &() { return _value; }
  constexpr operator T() const { return _value; }
  explicit constexpr operator T *() { return &_value; }
  template <typename U> constexpr bool operator==(const Rad<U> &other) const {
    return _value == other._value;
  }
  template <typename U> constexpr bool operator==(const Deg<U> &other) const {
    return (*this) == Rad(other);
  }

private:
  T _value;
};
template <typename T> Rad(T) -> Rad<T>;

template <typename T> struct Deg {
  constexpr Deg(T value) : _value(value) {}
  explicit constexpr Deg(Rad<T> value) : _value(rad2deg(value)) {}
  constexpr operator T &() { return _value; }
  constexpr operator T() const { return _value; }
  explicit constexpr operator T *() { return &_value; }
  template <typename U> constexpr bool operator==(const Deg<U> &other) const {
    return _value == other._value;
  }
  template <typename U> constexpr bool operator==(const Rad<U> &other) const {
    return _value == Deg(other);
  }

private:
  T _value;
};
template <typename T> Deg(T) -> Deg<T>;

template <typename T>
constexpr Rad<T> operator*(const Rad<T> &left, const T &right) {
  return Rad<T>{T(left) * right};
}

template <typename T>
constexpr Rad<T> operator*(const T &left, const Rad<T> &right) {
  return Rad<T>{left * T(right)};
}

using Radf = Rad<float>;
using Degf = Deg<float>;

} // namespace candlewick
