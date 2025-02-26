#pragma once
#include "../QOID_General.hpp"

namespace QOID {
struct Pixel {
  p_color packed;

  constexpr Pixel(p_color p) : packed(p) {}

#if defined(BIG_ENDIAN)
  constexpr Pixel(const color r = 0, const color g = 0, const color b = 0, const color a = 255) :
      packed((r << 24) | (g << 16) | (b << 8) | a) {}

  constexpr color R() const { return packed >> 24; }
  constexpr color G() const { return (packed >> 16) & 0xFF; }
  constexpr color B() const { return (packed >> 8) & 0xFF; }
  constexpr color A() const { return packed & 0xFF; }

  constexpr void setR(color r) { packed = (packed & 0x00FFFFFF) | (r << 24); }
  constexpr void setG(color g) { packed = (packed & 0xFF00FFFF) | (g << 16); }
  constexpr void setB(color b) { packed = (packed & 0xFFFF00FF) | (b << 8); }
  constexpr void setA(color a) { packed = (packed & 0xFFFFFF00) | a; }
#else
  constexpr Pixel(const color r = 0, const color g = 0, const color b = 0, const color a = 255) :
      packed((a << 24) | (b << 16) | (g << 8) | r) {}

  constexpr color A() const { return packed >> 24; }
  constexpr color B() const { return (packed >> 16) & 0xFF; }
  constexpr color G() const { return (packed >> 8) & 0xFF; }
  constexpr color R() const { return packed & 0xFF; }

  constexpr void setA(color r) { packed = (packed & 0x00FFFFFF) | (r << 24); }
  constexpr void setB(color g) { packed = (packed & 0xFF00FFFF) | (g << 16); }
#endif

  // Direct "Packing/Unpacking"
  constexpr p_color Pack() const { return packed; }
  constexpr Pixel &unPack(p_color p) {
    packed = p;
    return *this;
  }

  constexpr size_t Size() const { return sizeof(packed); }
  // Bitwise Operators (Useful for Masks and Effects)
  constexpr Pixel operator|(const Pixel &p) const { return packed | p.packed; }
  constexpr Pixel operator&(const Pixel &p) const { return packed & p.packed; }
  constexpr Pixel operator^(const Pixel &p) const { return packed ^ p.packed; }

  // Arithmetic Operators (Clamped to Avoid Overflow/Underflow)
  constexpr Pixel operator+(const Pixel &p) const {
    return Pixel(std::min(255, R() + p.R()), std::min(255, G() + p.G()), std::min(255, B() + p.B()),
                 std::min(255, A() + p.A()));
  }

  constexpr Pixel operator-(const Pixel &p) const {
    return Pixel(std::max(0, R() - p.R()), std::max(0, G() - p.G()), std::max(0, B() - p.B()),
                 std::max(0, A() - p.A()));
  }

  constexpr Pixel operator*(float scale) const {
    return Pixel(std::min(255, static_cast<int>(R() * scale)), std::min(255, static_cast<int>(G() * scale)),
                 std::min(255, static_cast<int>(B() * scale)), std::min(255, static_cast<int>(A() * scale)));
  }

  // Compound Assignment Operators (Avoids Creating New Objects)
  constexpr Pixel &operator|=(const Pixel &p) {
    packed |= p.packed;
    return *this;
  }
  constexpr Pixel &operator&=(const Pixel &p) {
    packed &= p.packed;
    return *this;
  }
  constexpr Pixel &operator^=(const Pixel &p) {
    packed ^= p.packed;
    return *this;
  }
  constexpr Pixel &operator+=(const Pixel &p) { return *this = *this + p; }
  constexpr Pixel &operator-=(const Pixel &p) { return *this = *this - p; }
  constexpr Pixel &operator*=(float scale) { return *this = *this * scale; }

  // Equality Operators
  constexpr bool operator==(const Pixel &p) const { return packed == p.packed; }
  constexpr bool operator!=(const Pixel &p) const { return packed != p.packed; }
};

} // namespace QOID