#pragma once
#include "../../QOID_General.hpp"
#include "../../image.hpp"
#include "../pixel.hpp"
#include "src/QOID/QOID_General.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <map>

namespace QOID {
namespace qoi {

enum class Opcodes {
  index = 0,
  diff,
  LUMA,
  RUN,
  New,
};

static inline bool writeTrail(std::ostream &file) {
#if defined(BIG_ENDIAN)
  static constexpr uint64_t end_marker{0x0000000000000001};
#else
  static constexpr uint64_t end_marker{0x0100000000000000};
#endif
  return !!file.write(reinterpret_cast<const char *>(&end_marker),
                      sizeof(end_marker)); // !!stuff makes stuff a bool lol
}

static inline bool writeHeader(std::ostream &file, const Image &image) {
  std::array<std::byte, 14> buffer{};

  static constexpr uint8_t channels{4}; // will be optimized out anyways
  static constexpr uint8_t colorspace{1};

  // Write the "qoif" magic number
  std::memcpy(buffer.data(), "qoif", 4); // Fixed to 4 bytes for "qoif"

  // write Width and height according to endian
#if defined(BIG_ENDIAN)
  static constexpr uint16_t combined{channels << 8 | colorspace};
  std::memcpy(buffer.data() + 4, image.getWidthR(), sizeof(image.getWidth()));
  std::memcpy(buffer.data() + 8, image.getHeightR(), sizeof(image.getHeight()));
#else
  static constexpr uint16_t combined{colorspace << 8 | channels};
  const uint32_t swappedWidth = std::byteswap(image.getWidth());
  const uint32_t swappedHeight = std::byteswap(image.getHeight());
  std::memcpy(buffer.data() + 4, &swappedWidth, sizeof(swappedWidth));
  std::memcpy(buffer.data() + 8, &swappedHeight, sizeof(swappedHeight));
#endif

  // Write the combined channels/colorspace value
  std::memcpy(buffer.data() + 12, &combined, sizeof(combined));

  return !!file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
}

static inline bool writeDataNonCompressedNonOptimized(std::ostream &file, const Image &image) {
  for (auto i : image.GetData()) {
    if ((file << 0xFF).write(reinterpret_cast<const char *>(&i), sizeof(Pixel))) return false;
  }

  return true;
}

static inline void WriteToBuffer(const Opcodes op, std::vector<std::byte> &buffer,
                                 std::vector<Pixel>::const_iterator &RawVecPointer,
                                 std::map<p_color, uint8_t> &SeenPixels, size_t &bufferIndex) {
  switch (op) {
  case Opcodes::New:

    std::memcpy(buffer.data() + bufferIndex, &(*RawVecPointer), sizeof(Pixel));
    bufferIndex += sizeof(Pixel);

    SeenPixels[RawVecPointer->Pack()] = static_cast<uint8_t>(SeenPixels.size()); // Assign index
    ++RawVecPointer;
    return;
  case Opcodes::index: break;
  case Opcodes::diff: break;
  case Opcodes::LUMA: break;
  case Opcodes::RUN: break;
  }
}

static inline bool writeData([[maybe_unused]] std::ostream &file, const Image &image) {
  const size_t ImageSize{image.getHeight() * image.getWidth()};

  const auto &RawDataVec{image.GetData()};
  std::vector<std::byte> buffer(ImageSize * 5); // max possible size
  std::map<p_color, uint8_t> SeenPixels;        // actual index is only 6 bytes

  auto DataIterator{RawDataVec.begin()};
  size_t bufferIndex = 0; // Track the write position

  WriteToBuffer(Opcodes::New, buffer, DataIterator, SeenPixels, bufferIndex);

  return true;
}

static inline bool GenerateFile(const Image &image, const strv FilePath) {

  std::ofstream file{FilePath.ends_with(".qoi") ? FilePath.data() : std::string(FilePath) + ".qoi",
                     std::ios::binary | std::ios::out};

  return writeHeader(file, image) && writeDataNonCompressedNonOptimized(file, image) && writeTrail(file);
  //   return writeHeader(file, image) && writeData(file, image) && writeTrail(file);
}

static inline bool GenerateFileNonCompressed(const Image &image, const strv FilePath) {

  std::ofstream file{FilePath.ends_with(".qoi") ? FilePath.data() : std::string(FilePath) + ".qoi",
                     std::ios::binary | std::ios::out};

  return writeHeader(file, image) && writeDataNonCompressedNonOptimized(file, image) && writeTrail(file);
}

} // namespace qoi

} // namespace QOID