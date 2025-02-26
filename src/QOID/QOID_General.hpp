#pragma once
// based on https://qoiformat.org/qoi-specification.pdf  | accessed on 2026.02.2025
#include <cstdint>
#include <string>
#include <string_view>

// assumes little endian if not big endian
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define BIG_ENDIAN
#endif

namespace QOID {
// since qoi is using 255 colorspace 8 bits
using color = uint8_t;
// R, G, B, A packed into a single
using p_color = uint32_t;

using str = std::string;
using strv = std::string_view;
// using ui = unsigned int;
using ui = uint32_t; // uint32_t because qoi-specifiation uses that

inline color min{0};
inline color max{255};

// imagetypes supported by image class
enum class ImageType {
  // Will be saved in raw format (IFD: compression 1)
  tiff = 0,
  // Will be saved according to QOI specification - 26.02.2025
  qoi,
};
} // namespace QOID
