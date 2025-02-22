#include "rawimage.hpp"
#include "QOID_General.hpp"
#include "pixel.hpp"
#include "qoi.hpp"
#include <bit>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <ios>
#include <fstream>
#include <ostream>
#include <stdexcept>

namespace QOID {

namespace qoi {
bool writeHeader(std::ofstream &file, uint32_t size_x, uint32_t size_y) {
  // unoptimized, still temporary
  uint32_t sx{std::byteswap(size_x)};
  uint32_t sy{std::byteswap(size_y)};

  if (!file.write("qoif", 4)) return false;
  if (!file.write(reinterpret_cast<const char *>(&sx), sizeof(size_x))) return false;
  if (!file.write(reinterpret_cast<const char *>(&sy), sizeof(size_y))) return false;
  static constexpr uint8_t channels{4};
  static constexpr uint8_t colorspace{1};
  if (!file.write(reinterpret_cast<const char *>(&channels), sizeof(uint8_t))) return false;
  if (!file.write(reinterpret_cast<const char *>(&colorspace), sizeof(uint8_t))) return false;
  return true;
}

bool writeTrail(std::ostream &file) {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  static constexpr uint64_t b{0x0000000000000001};
#else
  static constexpr uint64_t b{0x0100000000000000};
#endif
  if (!file.write(reinterpret_cast<const char *>(&b), sizeof(b))) return false;
  return true;
}

bool GenerateQoiFile(const QOID::RawImage &RawImage, std::filesystem::path &FileName) {
  std::ofstream file{FileName.replace_extension(".qoi"), std::ios::binary | std::ios::out};
  if (!file.is_open()) throw std::runtime_error{"file couldn't be opened"};

  // if either greater than numeric max size of uint32_t its not possible to create the file
  if ((RawImage.GetWidth() + RawImage.GetHeight()) < 2) throw std::runtime_error("Image cant be too small");
  if (!qoi::writeHeader(file, RawImage.GetWidth(), RawImage.GetHeight()))
    throw std::runtime_error("Couldnt write header to file");

  // stuff
  file.write(reinterpret_cast<const char *>(&qoi::NewColIndic), sizeof(qoi::NewColIndic));
  constexpr auto tmp{QOID::Pixel(0, 0, 255)};
  file.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
  // here

  if (!qoi::writeTrail(file)) throw std::runtime_error("Couldnt write header to file");

  return true;
}
} // namespace qoi
} // namespace QOID