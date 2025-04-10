#pragma once
#include <cstdint>

#if defined(__cpp_lib_byteswap)
#include <bit> // for std::byteswap (C++23 and later)
#endif

// Do not enforce a specific size for p_color. It can be redefined to a wider or
// narrower type as needed.
using p_color = uint64_t;
using color = uint8_t;

namespace QOID {


// The Pixel union gives two views of the pixel:
//  - as a variable–size packed representation (member RGBA)
//  - as individual color channels through an anonymous struct.
union Pixel {
  p_color RGBA; // canonical packed representation

  // The anonymous struct gives direct access to the four channels.
  // The order in memory is determined by the endian–ness flag.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
  struct {
#if defined(BIG_ENDIAN)
    color R;
    color G;
    color B;
    color A;
#else
    // On little endian the order is reversed.
    color A;
    color B;
    color G;
    color R;
#endif
  };
#pragma GCC diagnostic pop

  // ---------------------------------------------------------------------------
  // Compile–time constants used to calculate shift positions.
  static constexpr int num_channels = 4;
  static constexpr int channel_bits = sizeof(color) * 8; // bits per channel
  static constexpr int total_channel_bits =
      num_channels * channel_bits; // logical pixel bits
  static constexpr int total_bits =
      sizeof(p_color) * 8; // total bits available in p_color
  static constexpr int base_shift =
      total_bits -
      total_channel_bits; // if p_color is larger, channels are right–aligned

  // ---------------------------------------------------------------------------
  // Constructors

  // Construct from a raw packed value.
  constexpr Pixel(p_color p) : RGBA(p) {}

  // Construct from four channels.
  // Note that the initializer order for the anonymous struct must follow the
  // layout declared above (dependent on BIG_ENDIAN).
  constexpr Pixel(color r = 0, color g = 0, color b = 0, color a = 255)
#if defined(BIG_ENDIAN)
      : R(r), G(g), B(b), A(a)
#else
      : A(a), B(b), G(g), R(r)
#endif
  {
  }

  // ---------------------------------------------------------------------------
  // Component accessors and setters

  constexpr color getR() const { return R; }
  constexpr color getG() const { return G; }
  constexpr color getB() const { return B; }
  constexpr color getA() const { return A; }

  constexpr void setR(color r) { R = r; }
  constexpr void setG(color g) { G = g; }
  constexpr void setB(color b) { B = b; }
  constexpr void setA(color a) { A = a; }

  // ---------------------------------------------------------------------------
  // Getter for the canonical packed value.
  // (This is the same as reading member RGBA.)
  constexpr p_color getRGBA() const { return RGBA; }

  // ---------------------------------------------------------------------------
  // Alternate ordering getters
  //
  // All of these functions build a p_color value by reassembling the four
  // channels using shift–amounts computed from our constants. This approach
  // avoids hardcoding literal values so that changes to the size of p_color or
  // color are automatically handled.
  //
  // The intended orders are:
  //   ARGB: A in highest bits, then R, then G, then B.
  //   ABGR: A in highest bits, then B, then G, then R.
  //   BGRA: B in highest bits, then G, then R, then A.

  constexpr p_color ARGB() const {
    p_color out = 0;
    out |= (static_cast<p_color>(getA()) << (base_shift + 3 * channel_bits));
    out |= (static_cast<p_color>(getR()) << (base_shift + 2 * channel_bits));
    out |= (static_cast<p_color>(getG()) << (base_shift + 1 * channel_bits));
    out |= (static_cast<p_color>(getB()) << (base_shift + 0 * channel_bits));
    return out;
  }

  constexpr p_color ABGR() const {
    p_color out = 0;
    out |= (static_cast<p_color>(getA()) << (base_shift + 3 * channel_bits));
    out |= (static_cast<p_color>(getB()) << (base_shift + 2 * channel_bits));
    out |= (static_cast<p_color>(getG()) << (base_shift + 1 * channel_bits));
    out |= (static_cast<p_color>(getR()) << (base_shift + 0 * channel_bits));
    return out;
  }

  constexpr p_color BGRA() const {
    p_color out = 0;
    out |= (static_cast<p_color>(getB()) << (base_shift + 3 * channel_bits));
    out |= (static_cast<p_color>(getG()) << (base_shift + 2 * channel_bits));
    out |= (static_cast<p_color>(getR()) << (base_shift + 1 * channel_bits));
    out |= (static_cast<p_color>(getA()) << (base_shift + 0 * channel_bits));
    return out;
  }

#if defined(__cpp_lib_byteswap)
  static constexpr bool can_byteswap =
      (sizeof(p_color) == num_channels * sizeof(color));
  // When can_byteswap is true, std::byteswap(RGBA) reverses the byte order.
  // Depending on your underlying union ordering and system endianess this may
  // yield one of the common layouts. Here we provide example overloads.
  constexpr p_color ARGB_byteswap() const {
    return can_byteswap ? std::byteswap(RGBA) : ARGB();
  }
#endif

  // ---------------------------------------------------------------------------
  // Operator overloads

  // Bitwise Operators (for masks or effects)
  constexpr Pixel operator|(const Pixel &other) const {
    return Pixel(RGBA | other.RGBA);
  }
  constexpr Pixel operator&(const Pixel &other) const {
    return Pixel(RGBA & other.RGBA);
  }
  constexpr Pixel operator^(const Pixel &other) const {
    return Pixel(RGBA ^ other.RGBA);
  }

  // Arithmetic Operators with clamping to avoid overflow/underflow.
  constexpr Pixel operator+(const Pixel &other) const {
    return Pixel(static_cast<color>((static_cast<int>(getR()) +
                                     static_cast<int>(other.getR())) > 255
                                        ? 255
                                        : (getR() + other.getR())),
                 static_cast<color>((static_cast<int>(getG()) +
                                     static_cast<int>(other.getG())) > 255
                                        ? 255
                                        : (getG() + other.getG())),
                 static_cast<color>((static_cast<int>(getB()) +
                                     static_cast<int>(other.getB())) > 255
                                        ? 255
                                        : (getB() + other.getB())),
                 static_cast<color>((static_cast<int>(getA()) +
                                     static_cast<int>(other.getA())) > 255
                                        ? 255
                                        : (getA() + other.getA())));
  }
  constexpr Pixel operator-(const Pixel &other) const {
    return Pixel(static_cast<color>((static_cast<int>(getR()) -
                                     static_cast<int>(other.getR())) < 0
                                        ? 0
                                        : (getR() - other.getR())),
                 static_cast<color>((static_cast<int>(getG()) -
                                     static_cast<int>(other.getG())) < 0
                                        ? 0
                                        : (getG() - other.getG())),
                 static_cast<color>((static_cast<int>(getB()) -
                                     static_cast<int>(other.getB())) < 0
                                        ? 0
                                        : (getB() - other.getB())),
                 static_cast<color>((static_cast<int>(getA()) -
                                     static_cast<int>(other.getA())) < 0
                                        ? 0
                                        : (getA() - other.getA())));
  }
  constexpr Pixel operator*(float scale) const {
    auto clamp = [](int value) -> color {
      return static_cast<color>((value > 255) ? 255 : (value < 0 ? 0 : value));
    };
    return Pixel(clamp(static_cast<int>(getR() * scale)),
                 clamp(static_cast<int>(getG() * scale)),
                 clamp(static_cast<int>(getB() * scale)),
                 clamp(static_cast<int>(getA() * scale)));
  }

  // Compound assignment operators
  constexpr Pixel &operator|=(const Pixel &other) {
    RGBA |= other.RGBA;
    return *this;
  }
  constexpr Pixel &operator&=(const Pixel &other) {
    RGBA &= other.RGBA;
    return *this;
  }
  constexpr Pixel &operator^=(const Pixel &other) {
    RGBA ^= other.RGBA;
    return *this;
  }
  constexpr Pixel &operator+=(const Pixel &other) {
    return *this = *this + other;
  }
  constexpr Pixel &operator-=(const Pixel &other) {
    return *this = *this - other;
  }
  constexpr Pixel &operator*=(float scale) { return *this = *this * scale; }

  // Equality operators
  constexpr bool operator==(const Pixel &other) const {
    return RGBA == other.RGBA;
  }
  constexpr bool operator!=(const Pixel &other) const {
    return RGBA != other.RGBA;
  }

  // ---------------------------------------------------------------------------
  // Size utility

  // Return the size in bytes used by the packed value.
  constexpr size_t Size() const { return sizeof(RGBA); }
};

} // namespace QOID

