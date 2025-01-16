#include "math_util.h"

#include <Eigen/Geometry>

namespace candlewick {

Vec3u8 hexToRgbi(unsigned long hex) {
  unsigned rem = unsigned(hex); // red = quotient of hex by 256^2
  unsigned r = rem >> 16;       // remainder
  rem %= 1 << 16;
  unsigned g = rem >> 8; // green = quotient of rem by 256
  rem %= 1 << 8;
  unsigned b = rem; // blue = remainder
  return {uint8_t(r), uint8_t(g), uint8_t(b)};
};

Vec4u8 hexToRgbai(unsigned long hex) {
  unsigned rem = unsigned(hex);
  unsigned r = rem >> 24; // red = quotient of hex by 256^3 = 2^24
  rem %= 1 << 24;
  unsigned g = rem >> 16; // green = quotient by 256^2 = 2^16
  rem %= 1 << 16;
  unsigned b = rem >> 8; // blue = quotient
  rem %= 1 << 8;
  unsigned a = rem; // alpha = remainder
  return {uint8_t(r), uint8_t(g), uint8_t(b), uint8_t(a)};
};

} // namespace candlewick
