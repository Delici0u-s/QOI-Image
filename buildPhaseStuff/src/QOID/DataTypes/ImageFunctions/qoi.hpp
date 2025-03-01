#pragma once
#include "../../QOID_General.hpp"
#include "../pixel.hpp"
#include "../../image.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <unordered_map>
#include <vector>

namespace QOID {
class Image;
namespace qoi {

static inline bool writeTrail(std::ostream &file) {
#if defined(BIG_ENDIAN)
  static constexpr uint64_t end_marker{0x0000000000000001};
#else
  static constexpr uint64_t end_marker{0x0100000000000000};
#endif
  return !!file.write(reinterpret_cast<const char *>(&end_marker), sizeof(end_marker));
}

static inline bool writeHeader(std::ostream &file, const Image &image) {
  std::array<std::byte, 14> buffer{};

  static constexpr uint8_t channels{4}; // will be optimized out anyways
  static constexpr uint8_t colorspace{1};

  // Write the "qoif" magic number
  std::memcpy(buffer.data(), "qoif", 4);

  // write Width and height according to endian
#if defined(BIG_ENDIAN)
  const uint32_t swappedWidth = (image.getWidth());
  const uint32_t swappedHeight = (image.getHeight());
  static constexpr uint16_t combined{channels << 8 | colorspace};
#else
  static constexpr uint16_t combined{colorspace << 8 | channels};
  const uint32_t swappedWidth = std::byteswap(image.getWidth());
  const uint32_t swappedHeight = std::byteswap(image.getHeight());
#endif
  std::memcpy(buffer.data() + 4, &swappedWidth, sizeof(swappedWidth));
  std::memcpy(buffer.data() + 8, &swappedHeight, sizeof(swappedHeight));

  // Write the combined channels/colorspace value
  std::memcpy(buffer.data() + 12, &combined, sizeof(combined));

  return !!file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
}

namespace {

static inline bool writeDataNonCompressedNonOptimized(std::ostream &file, const Image &image) {
  static uint8_t tmp{0xFF};
  for (auto i : image.GetData()) {
    file.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
    file.write(reinterpret_cast<const char *>(&i), sizeof(Pixel));
  }
  return true;
}

static inline void updateIndex(std::unordered_map<p_color, uint8_t> &SeenPixels, const Pixel &px,
                               unsigned int &SwapNum) {
  if (SeenPixels.size() >= 64) {
    auto it = SeenPixels.begin();
    std::advance(it, SwapNum); // Move iterator to SwapNum-th element
    SeenPixels.erase(it);      // Erase it in O(1) (on average)
  }

  SeenPixels[px.Pack()] = (px.R() * 3 + px.G() * 5 + px.B() * 7 + px.A() * 11) % 64;
  SwapNum = (SwapNum + 1) % 64; // Proper circular increment
}
static inline void WriteFirstCol(std::vector<std::byte> &buffer, std::vector<Pixel>::const_iterator &DataIterator,
                                 std::unordered_map<p_color, uint8_t> &SeenPixels, size_t &bufferIndex,
                                 unsigned int &SwapNum) {
  static constexpr uint8_t Uint8Tmp{0xFF};
  std::memcpy(buffer.data() + bufferIndex, &Uint8Tmp, sizeof(Uint8Tmp));
  std::memcpy(buffer.data() + bufferIndex + 1, &(*DataIterator), sizeof(Pixel));
  updateIndex(SeenPixels, *DataIterator, SwapNum);
  ++DataIterator;
  bufferIndex += sizeof(Pixel) + sizeof(Uint8Tmp);
  return;
}

static inline void WriteToBuffer(std::vector<std::byte> &buffer, std::vector<Pixel>::const_iterator &DataIterator,
                                 std::unordered_map<p_color, uint8_t> &SeenPixels, size_t &bufferIndex,
                                 const auto &DataEndIt, unsigned int &SwapNum) {
  if (DataIterator->packed == (DataIterator - 1)->packed) // RUN
  {
    constexpr size_t maxRunLength = 62;
    size_t runCount = 1;
    while (runCount < maxRunLength && (DataIterator + runCount) != DataEndIt &&
           (DataIterator + runCount)->packed == DataIterator->packed) {
      ++runCount;
    }
    uint8_t runMarker = static_cast<uint8_t>((runCount - 1) | 0xC0);
    std::memcpy(buffer.data() + bufferIndex, &runMarker, sizeof(runMarker));
    bufferIndex += 1;
    DataIterator += runCount;
    return;
  }

  const Pixel &current{*DataIterator};
  const Pixel &previous{*(DataIterator - 1)};
  if (DataIterator->A() == (DataIterator - 1)->A()) {
    int diffG = static_cast<int>(current.G()) - static_cast<int>(previous.G());
    int diffR = static_cast<int>(current.R()) - static_cast<int>(previous.R());
    int diffB = static_cast<int>(current.B()) - static_cast<int>(previous.B());
    if ((diffR >= -2 && diffR <= 1) && (diffG >= -2 && diffG <= 1) && (diffB >= -2 && diffB <= 1)) { // Diff
      // Compute current minus previous.
      uint8_t diffR = static_cast<uint8_t>(current.R() - previous.R() + 2);
      uint8_t diffG = static_cast<uint8_t>(current.G() - previous.G() + 2);
      uint8_t diffB = static_cast<uint8_t>(current.B() - previous.B() + 2);
      const uint8_t comb{static_cast<uint8_t>(0x40 | (diffR << 4 | diffG << 2 | diffB))};
      std::memcpy(buffer.data() + bufferIndex, &comb, sizeof(comb));
      bufferIndex += 1;
      updateIndex(SeenPixels, current, SwapNum);
      ++DataIterator;
      return;
    }
    diffR -= diffG;
    diffB -= diffG;
    if ((diffG >= -32 && diffG <= 31) && (diffR >= -8 && diffR <= 7) && (diffB >= -8 && diffB <= 7)) { // LUMA
      int8_t dg = static_cast<int8_t>(current.G() - previous.G());
      int8_t dr = static_cast<int8_t>(current.R() - previous.R()) - dg;
      int8_t db = static_cast<int8_t>(current.B() - previous.B()) - dg;
      uint8_t encodedDG = static_cast<uint8_t>(dg + 32); // Range: 0 to 63.
      uint8_t encodedDR = static_cast<uint8_t>(dr + 8);  // Range: 0 to 15.
      uint8_t encodedDB = static_cast<uint8_t>(db + 8);  // Range: 0 to 15.
#if defined(BIG_ENDIAN)
      const uint16_t comb{static_cast<uint16_t>(0x8000 | (encodedDG << 8) | (encodedDR << 4) | encodedDB)};
#else
      const uint16_t comb{
          std::byteswap(static_cast<uint16_t>(0x8000 | (encodedDG << 8) | (encodedDR << 4) | encodedDB))};
#endif
      std::memcpy(buffer.data() + bufferIndex, &comb, sizeof(comb));
      bufferIndex += 2;
      updateIndex(SeenPixels, current, SwapNum);
      ++DataIterator;
      return;
    }
  }
  if (SeenPixels.count(DataIterator->packed)) { // index
    const uint8_t comb{SeenPixels[DataIterator->packed]};
    std::memcpy(buffer.data() + bufferIndex, &comb, sizeof(comb));
    ++bufferIndex;
    ++DataIterator;
    return;
  } else { // NEW
    static constexpr uint8_t Uint8Tmp{0xFF};
    std::memcpy(buffer.data() + bufferIndex, &Uint8Tmp, sizeof(Uint8Tmp));
    std::memcpy(buffer.data() + bufferIndex + 1, &(*DataIterator), sizeof(Pixel));
    updateIndex(SeenPixels, *DataIterator, SwapNum);
    ++DataIterator;
    bufferIndex += sizeof(Pixel) + sizeof(Uint8Tmp);
    return;
  }
}

static inline bool writeData(std::ostream &file, const Image &image) {
  const size_t ImageSize{image.getHeight() * image.getWidth()};
  const auto &RawDataVec{image.GetData()};
  std::vector<std::byte> buffer(ImageSize * 5); // max possible size  this has to because of memcpy
  std::unordered_map<p_color, uint8_t> SeenPixels;
  auto DataIterator{RawDataVec.begin()};
  size_t bufferIndex = 0;
  unsigned int SwapNum{0};

  // The first pixel is always new.
  WriteFirstCol(buffer, DataIterator, SeenPixels, bufferIndex, SwapNum);
  for (; DataIterator != image.GetData().end();) {
    WriteToBuffer(buffer, DataIterator, SeenPixels, bufferIndex, RawDataVec.end(), SwapNum);
  }

  // Write out the buffer in chunks.
  constexpr size_t chunkSize = 4096; // 4KB
  size_t chunkCount = bufferIndex / chunkSize;
  for (size_t pos = 0; pos < chunkCount; ++pos) {
    file.write(reinterpret_cast<const char *>(buffer.data() + pos * chunkSize), chunkSize);
  }
  // Write the remaining bytes.
  size_t remainder = bufferIndex % chunkSize;
  if (remainder) file.write(reinterpret_cast<const char *>(buffer.data() + chunkCount * chunkSize), remainder);
  return true;
}

} // namespace

inline bool GenerateFile(const Image &image, const strv FilePath) {
  std::ofstream file{FilePath.ends_with(".qoi") ? FilePath.data() : std::string(FilePath) + ".qoi",
                     std::ios::binary | std::ios::out};
  return writeHeader(file, image) && writeData(file, image) && writeTrail(file);
}

static inline bool GenerateFileNonCompressed(const Image &image, const strv FilePath) {
  std::ofstream file{FilePath.ends_with(".qoi") ? FilePath.data() : std::string(FilePath) + ".qoi",
                     std::ios::binary | std::ios::out};
  return writeHeader(file, image) && writeDataNonCompressedNonOptimized(file, image) && writeTrail(file);
}

} // namespace qoi

} // namespace QOID
