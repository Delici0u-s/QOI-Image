#pragma once
#include "../../QOID_General.hpp"
#include "../pixel.hpp"
#include "../../image.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <string>
#include <cstring>

namespace QOID {
namespace tga {

// Writes the 18-byte TGA header for a 32-bit (8-bit per channel RGBA) image.
static inline bool writeHeader(std::ostream &file, const Image &image) {
  // TGA header (18 bytes):
  // Byte 0: ID length = 0
  // Byte 1: Color map type = 0 (no color map)
  // Byte 2: Image type = 2 (uncompressed true-color image)
  // Bytes 3-7: Color map specification (unused, set to 0)
  // Bytes 8-9: X-origin (0)
  // Bytes 10-11: Y-origin (0)
  // Bytes 12-13: Width (little-endian)
  // Bytes 14-15: Height (little-endian)
  // Byte 16: Pixel depth = 32 (bits per pixel)
  // Byte 17: Image descriptor = 0x28
  //          (bits 0-3: 8 bits of alpha, bit 5: top-left origin)
  std::array<std::uint8_t, 18> header{};
  header[0] = 0; // ID length
  header[1] = 0; // Color map type
  header[2] = 2; // Image type (uncompressed true-color)

  // Color map specification: bytes 3-7 already zeroed.
  // X-origin (bytes 8-9)
  header[8] = 0;
  header[9] = 0;
  // Y-origin (bytes 10-11)
  header[10] = 0;
  header[11] = 0;

  // Image width (little-endian)
  uint16_t width = static_cast<uint16_t>(image.getWidth());
  header[12] = static_cast<std::uint8_t>(width & 0xFF);
  header[13] = static_cast<std::uint8_t>((width >> 8) & 0xFF);

  // Image height (little-endian)
  uint16_t height = static_cast<uint16_t>(image.getHeight());
  header[14] = static_cast<std::uint8_t>(height & 0xFF);
  header[15] = static_cast<std::uint8_t>((height >> 8) & 0xFF);

  header[16] = 32;   // Pixel depth: 32 bits per pixel (8 bits per channel)
  header[17] = 0x28; // Image descriptor: 8-bit alpha, top-left origin (bit 5 set)

  return !!file.write(reinterpret_cast<const char *>(header.data()), header.size());
}

// Writes the image data in BGRA order (TGA expects pixels stored as Blue, Green, Red, Alpha).
// Assumes that image.GetData() returns a container of Pixels in RGBA order.
static inline bool writeData(std::ostream &file, const Image &image) {
  const auto &pixels = image.GetData();
  // Each pixel is written as 4 bytes: BGRA
  for (const auto &px : pixels) {
    // Convert from RGBA (your internal format) to BGRA.
    uint8_t bgra[4] = {px.B(), px.G(), px.R(), px.A()};
    file.write(reinterpret_cast<const char *>(bgra), sizeof(bgra));
  }
  return true;
}

// Generates a TGA file from the provided image. If FilePath does not end with ".tga",
// it will be appended.
inline bool GenerateFile(const Image &image, const strv FilePath) {
  std::string filePath;
  if (std::string(FilePath).ends_with(".tga")) filePath = FilePath;
  else filePath = std::string(FilePath) + ".tga";
  std::ofstream file{filePath, std::ios::binary | std::ios::out};
  if (!file) return false;
  return writeHeader(file, image) && writeData(file, image);
}

} // namespace tga
} // namespace QOID
