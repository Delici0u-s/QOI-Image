#pragma once
#include <cstddef>
#include <cstdint>
#include <algorithm> // For std::min and std::max

namespace QOID {

struct Pixel {
  uint32_t packed; // Store as a single 32-bit integer

  // Constructors
  constexpr Pixel(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255) :
      packed((r << 24) | (g << 16) | (b << 8) | a) {}

  constexpr Pixel(uint32_t p) : packed(p) {}

  // Fast Getters (Avoid Bit Shifting Multiple Times)
  constexpr uint8_t R() const { return packed >> 24; }
  constexpr uint8_t G() const { return (packed >> 16) & 0xFF; }
  constexpr uint8_t B() const { return (packed >> 8) & 0xFF; }
  constexpr uint8_t A() const { return packed & 0xFF; }

  // Fast Setters (Avoid Unnecessary Computations)
  inline void setR(uint8_t r) { packed = (packed & 0x00FFFFFF) | (r << 24); }
  inline void setG(uint8_t g) { packed = (packed & 0xFF00FFFF) | (g << 16); }
  inline void setB(uint8_t b) { packed = (packed & 0xFFFF00FF) | (b << 8); }
  inline void setA(uint8_t a) { packed = (packed & 0xFFFFFF00) | a; }

  // Direct Packing/Unpacking
  constexpr uint32_t Pack() const { return packed; }
  constexpr Pixel &unPack(uint32_t p) {
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