#pragma once
#include "rawimage.hpp"
#include "QOID_General.hpp"
#include "pixel.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ios>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace QOID {
namespace tiff {

void writeHeaderTiff(std::ofstream &file, const uint32_t offset) {
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
  constexpr uint16_t numEntries = 9; // Number of IFD tags

  IFDEntry entries[] = {
      {256, 4, 1, imageWidth},          // ImageWidth (LONG)
      {257, 4, 1, imageHeight},         // ImageLength (LONG)
      {258, 3, 4, BitsPerSampleOffset}, // BitsPerSample (Pointer to extra values)
      {259, 3, 1, 1},                   // Compression (1 = None)
      {262, 3, 1, 2},                   // PhotometricInterpretation (2 = RGB)
      {273, 4, 1, stripOffset},         // StripOffsets
      {277, 3, 1, 4},                   // SamplesPerPixel (RGBA)
      {279, 4, 1, stripByteCount},      // StripByteCounts
      {338, 3, 1, 1},                   // ExtraSamples (1 = Unassociated alpha)
  };

  file.write(reinterpret_cast<const char *>(&numEntries), sizeof(numEntries));
  file.write(reinterpret_cast<const char *>(entries), sizeof(entries));
  uint32_t nextIFDOffset = 0; // No more IFDs
  file.write(reinterpret_cast<const char *>(&nextIFDOffset), sizeof(nextIFDOffset));
}

bool GenerateRawFile(const RawImage &image, std::filesystem::path &FileName) {
  std::ofstream file{FileName.replace_extension(".tiff"), std::ios::binary | std::ios::out};
  if (!file.is_open()) throw std::runtime_error{"file couldn't be opened"};

  static constexpr uint32_t headerSize{8};
  static uint32_t imageSize = image.GetWidth() * image.GetHeight() * sizeof(Pixel);
  // write header
  writeHeaderTiff(file, headerSize + imageSize);

  // buffer of each row of data.
  constexpr size_t chunkSize = 4096; // 4KB chunk size
  static std::vector<Pixel> buffer(std::min(chunkSize / sizeof(Pixel), static_cast<size_t>(image.GetWidth())));

  for (ui row = 0; row < image.GetHeight(); ++row) {
    std::memcpy(buffer.data(), &image.GetRawVector()[row * image.GetWidth()], buffer.size() * sizeof(Pixel));
    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size() * sizeof(Pixel));
  }

  static constexpr uint16_t bitsPerSample[4] = {8, 8, 8, 8}; // RGBA, 8 bits per channel
  uint32_t bitsPerSampleOffset = headerSize + imageSize + sizeof(IFDEntry) * 9 + sizeof(uint16_t);

  file.seekp(headerSize + imageSize);
  writeIFD(file, image.GetWidth(), image.GetHeight(), headerSize, imageSize, bitsPerSampleOffset);
  file.seekp(bitsPerSampleOffset);
  file.write(reinterpret_cast<const char *>(bitsPerSample), sizeof(bitsPerSample));

  file.flush();
  file.close();
  return true;
}
} // namespace tiff

} // namespace QOID