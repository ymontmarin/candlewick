#pragma once
#include <Eigen/Core>
#include <concepts>

namespace candlewick {

using Float2 = Eigen::Vector2f;
using Float3 = Eigen::Vector3f;
using Float4 = Eigen::Vector4f;
using Mat3f = Eigen::Matrix3f;
using Mat4f = Eigen::Matrix4f;
using Vec3u8 = Eigen::Matrix<uint8_t, 3, 1>;
using Vec4u8 = Eigen::Matrix<uint8_t, 4, 1>;

/** GPU typedefs and adapters **/

using GpuVec2 = Eigen::Matrix<float, 2, 1, Eigen::DontAlign>;
struct GpuVec3 : public Eigen::Matrix<float, 3, 1, Eigen::DontAlign> {
  using Matrix::Matrix;
};
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

/* ANGLES */

namespace constants {
inline constexpr double Pi = 3.1415926535897932;
inline constexpr float Pif = 3.141592654f;
inline constexpr double Pi_2 = 1.5707963267948966;
inline constexpr double Pi_2f = 1.5707963267f;
} // namespace constants

inline constexpr double deg2rad(double t) { return t * constants::Pi / 180.0; }
inline constexpr float deg2rad(float t) { return t * constants::Pif / 180.0f; }
inline constexpr float rad2deg(float t) { return t * 180.0f / constants::Pif; }

template <std::floating_point T> struct Rad;
template <std::floating_point T> struct Deg;

/// \brief Strong type for floating-point variables representing angles (in
/// **radians**).
/// \sa Deg
template <std::floating_point T> struct Rad {
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
template <std::floating_point T> Rad(T) -> Rad<T>;

/// \brief Strong type for floating-point variables representing angles (in
/// **degrees**).
/// \sa Rad
template <std::floating_point T> struct Deg {
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
template <std::floating_point T> Deg(T) -> Deg<T>;

template <std::floating_point T>
constexpr Rad<T> operator*(const Rad<T> &left, const T &right) {
  return Rad<T>{T(left) * right};
}

template <std::floating_point T>
constexpr Rad<T> operator*(const T &left, const Rad<T> &right) {
  return Rad<T>{left * T(right)};
}

inline constexpr auto operator""_radf(long double t) {
  return Rad<float>(deg2rad(static_cast<float>(t)));
}

inline constexpr auto operator""_rad(long double t) {
  return Rad<double>(deg2rad(static_cast<double>(t)));
}

inline constexpr auto operator""_degf(long double t) {
  return Deg<float>{static_cast<float>(t)};
}

using Radf = Rad<float>;
using Degf = Deg<float>;

} // namespace candlewick
