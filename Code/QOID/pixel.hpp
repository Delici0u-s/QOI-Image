#pragma once
#include <cstddef>
#include <algorithm> // For std::min and std::max
#include "QOID_General.hpp"

namespace QOID {

struct Pixel {
  // Constructors
  p_color packed; // Store as a single 32-bit integer
  // Constructors

  constexpr Pixel(p_color p) : packed(p) {}

// check for endianess and then implement based on that
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  // It's a big-endian target architecture
  constexpr Pixel(const color r = 0, const color g = 0, const color b = 0, const color a = 255) :
      packed((r << 24) | (g << 16) | (b << 8) | a) {}

  // Fast Getters (Avoid Bit Shifting Multiple Times)
  constexpr color R() const { return packed >> 24; }
  constexpr color G() const { return (packed >> 16) & 0xFF; }
  constexpr color B() const { return (packed >> 8) & 0xFF; }
  constexpr color A() const { return packed & 0xFF; }

  // Fast Setters (Avoid Unnecessary Computations)
  inline void setR(color r) { packed = (packed & 0x00FFFFFF) | (r << 24); }
  inline void setG(color g) { packed = (packed & 0xFF00FFFF) | (g << 16); }
  inline void setB(color b) { packed = (packed & 0xFFFF00FF) | (b << 8); }
  inline void setA(color a) { packed = (packed & 0xFFFFFF00) | a; }
#else // assumes little endian if not big endian
  // It's a little-endian target architecture
  constexpr Pixel(const color r = 0, const color g = 0, const color b = 0, const color a = 255) :
      packed((a << 24) | (b << 16) | (g << 8) | r) {}

  // Fast Getters (Avoid Bit Shifting Multiple Times)
  constexpr color A() const { return packed >> 24; }
  constexpr color B() const { return (packed >> 16) & 0xFF; }
  constexpr color G() const { return (packed >> 8) & 0xFF; }
  constexpr color R() const { return packed & 0xFF; }

  // Fast Setters (Avoid Unnecessary Computations)
  inline void setA(color r) { packed = (packed & 0x00FFFFFF) | (r << 24); }
  inline void setB(color g) { packed = (packed & 0xFF00FFFF) | (g << 16); }
  inline void setG(color b) { packed = (packed & 0xFFFF00FF) | (b << 8); }
  inline void setR(color a) { packed = (packed & 0xFFFFFF00) | a; }
#endif

  // Direct Packing/Unpacking
  constexpr p_color Pack() const { return packed; }
  constexpr Pixel &unPack(p_color p) {
    packed = p;
    return *this;
  }

  inline size_t Size() const { return 4; }

  // Operators for Fast Pixel Math

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
  inline Pixel &operator|=(const Pixel &p) {
    packed |= p.packed;
    return *this;
  }
  inline Pixel &operator&=(const Pixel &p) {
    packed &= p.packed;
    return *this;
  }
  inline Pixel &operator^=(const Pixel &p) {
    packed ^= p.packed;
    return *this;
  }
  inline Pixel &operator+=(const Pixel &p) { return *this = *this + p; }
  inline Pixel &operator-=(const Pixel &p) { return *this = *this - p; }
  inline Pixel &operator*=(float scale) { return *this = *this * scale; }

  // Equality Operators
  constexpr bool operator==(const Pixel &p) const { return packed == p.packed; }
  constexpr bool operator!=(const Pixel &p) const { return packed != p.packed; }
};

} // namespace QOID