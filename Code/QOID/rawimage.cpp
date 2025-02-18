#include "rawimage.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ios>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace QOID {
void writeHeaderDNG(std::ofstream &file, const uint32_t offset) {
  // offset is relative to file beginning. First byte of file has offset 0
  static std::vector<std::byte> buffer(8);

  // Enter first two bytes based on endian
  if constexpr (std::endian::native == std::endian::big) {
    // Big-endian system
    std::memcpy(buffer.data(), "MM", sizeof(char) * 2);
  } else if constexpr (std::endian::native == std::endian::little) {
    // Little-endian system
    std::memcpy(buffer.data(), "II", sizeof(char) * 2);
  } else {
    throw std::runtime_error("Failed to detect endian system");
  }

  static constexpr uint16_t tiffArbNumber{42};
  std::memcpy(buffer.data() + 2, &tiffArbNumber, sizeof(tiffArbNumber));
  std::memcpy(buffer.data() + 4, &offset, sizeof(offset));
  file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size() * sizeof(std::byte));
}

void writeIFD(std::ofstream &file, uint32_t imageWidth, uint32_t imageHeight, uint32_t stripOffset,
              uint32_t stripByteCount, uint32_t BitsPerSampleOffset) {
  constexpr uint16_t numEntries = 8; // Number of IFD tags

  IFDEntry entries[] = {
      {256, 4, 1, imageWidth},          // ImageWidth (LONG)
      {257, 4, 1, imageHeight},         // ImageLength (LONG)
      {258, 3, 4, BitsPerSampleOffset}, // BitsPerSample (Pointer to extra values)
      {259, 3, 1, 1},                   // Compression (1 = None)
      {262, 3, 1, 2},                   // PhotometricInterpretation (2 = RGB)
      {273, 4, 1, stripOffset},         // StripOffsets
      {279, 4, 1, stripByteCount},      // StripByteCounts
      {277, 3, 1, 4},                   // SamplesPerPixel (RGBA)
  };
  file.write(reinterpret_cast<const char *>(&numEntries), sizeof(numEntries));
  file.write(reinterpret_cast<const char *>(entries), sizeof(entries));
  uint32_t nextIFDOffset = 0; // No more IFDs
  file.write(reinterpret_cast<const char *>(&nextIFDOffset), sizeof(nextIFDOffset));
}

bool RawImage::GenerateRawFile(const std::string_view Name) {
  if (!Name.ends_with(".DNG")) throw std::invalid_argument{"Not a .DNG file"};
  std::ofstream file{Name.data(), std::ios::binary | std::ios::out};
  if (!file.is_open()) throw std::runtime_error{"file couldn't be opened"};

  static constexpr uint32_t headerSize{8};
  static uint32_t imageSize = size_x * size_y * sizeof(Pixel);
  // write header
  QOID::writeHeaderDNG(file, headerSize + imageSize);

  // buffer of each row of data.
  constexpr size_t chunkSize = 4096; // 4KB chunk size
  static std::vector<Pixel> buffer(std::min(chunkSize / sizeof(Pixel), static_cast<size_t>(size_x)));

  for (ui row = 0; row < size_y; ++row) {
    std::memcpy(buffer.data(), &Data[row * size_x], buffer.size() * sizeof(Pixel));
    // dont we all love endians
    for (Pixel &p : buffer)
      p = std::byteswap(p.Pack());

    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size() * sizeof(Pixel));
  }

  uint16_t bitsPerSample[4] = {8, 8, 8, 8}; // RGBA, 8 bits per channel
  uint32_t bitsPerSampleOffset = headerSize + imageSize + sizeof(IFDEntry) * 8 + sizeof(uint16_t);

  file.seekp(headerSize + imageSize);
  writeIFD(file, size_x, size_y, headerSize, imageSize, bitsPerSampleOffset);
  file.seekp(bitsPerSampleOffset);
  file.write(reinterpret_cast<const char *>(bitsPerSample), sizeof(bitsPerSample));

  file.close();
  return true;
}
void RawImage::FillRegion(const Pixel &P, const ui x1, const ui y1, const ui x2, const ui y2) {
  if (x1 >= size_x || y1 >= size_y) throw std::out_of_range("Start coordinates out of bounds");
  if (x2 > size_x || y2 > size_y) throw std::out_of_range("End coordinates out of bounds");
  if (x1 > x2 || y1 > y2) throw std::invalid_argument("Invalid region bounds");

  for (ui y = y1; y < y2; ++y) {
    for (ui x = x1; x < x2; ++x) {
      SetPixel(P, x, y);
    }
  }
}

} // namespace QOID