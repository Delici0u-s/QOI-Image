#pragma once
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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
  // Will be saved according to QOI specification - 26.02.2025
  qoi = 0,
  // TGA is a lot faster, but its raw data, so a lot more space taken up in
  // storage
  tga,
};

struct Pixel {
  p_color packed;

  constexpr Pixel(p_color p) : packed(p) {}

#if defined(BIG_ENDIAN)
  constexpr Pixel(const color r = 0, const color g = 0, const color b = 0,
                  const color a = 255)
      : packed((r << 24) | (g << 16) | (b << 8) | a) {}

  constexpr color R() const { return packed >> 24; }
  constexpr color G() const { return (packed >> 16) & 0xFF; }
  constexpr color B() const { return (packed >> 8) & 0xFF; }
  constexpr color A() const { return packed & 0xFF; }

  constexpr void setR(color r) { packed = (packed & 0x00FFFFFF) | (r << 24); }
  constexpr void setG(color g) { packed = (packed & 0xFF00FFFF) | (g << 16); }
  constexpr void setB(color b) { packed = (packed & 0xFFFF00FF) | (b << 8); }
  constexpr void setA(color a) { packed = (packed & 0xFFFFFF00) | a; }
#else
  constexpr Pixel(const color r = 0, const color g = 0, const color b = 0,
                  const color a = 255)
      : packed((a << 24) | (b << 16) | (g << 8) | r) {}

  constexpr color A() const { return packed >> 24; }
  constexpr color B() const { return (packed >> 16) & 0xFF; }
  constexpr color G() const { return (packed >> 8) & 0xFF; }
  constexpr color R() const { return packed & 0xFF; }

  constexpr void setA(color r) { packed = (packed & 0x00FFFFFF) | (r << 24); }
  constexpr void setB(color g) { packed = (packed & 0xFF00FFFF) | (g << 16); }
#endif

  // Direct "Packing/Unpacking"
  constexpr p_color Pack() const { return packed; }
  constexpr Pixel &unPack(p_color p) {
    packed = p;
    return *this;
  }

  constexpr size_t Size() const { return sizeof(packed); }
  // Bitwise Operators (Useful for Masks and Effects)
  constexpr Pixel operator|(const Pixel &p) const { return packed | p.packed; }
  constexpr Pixel operator&(const Pixel &p) const { return packed & p.packed; }
  constexpr Pixel operator^(const Pixel &p) const { return packed ^ p.packed; }

  // Arithmetic Operators (Clamped to Avoid Overflow/Underflow)
  constexpr Pixel operator+(const Pixel &p) const {
    return Pixel(std::min(255, R() + p.R()), std::min(255, G() + p.G()),
                 std::min(255, B() + p.B()), std::min(255, A() + p.A()));
  }

  constexpr Pixel operator-(const Pixel &p) const {
    return Pixel(std::max(0, R() - p.R()), std::max(0, G() - p.G()),
                 std::max(0, B() - p.B()), std::max(0, A() - p.A()));
  }

  constexpr Pixel operator*(float scale) const {
    return Pixel(std::min(255, static_cast<int>(R() * scale)),
                 std::min(255, static_cast<int>(G() * scale)),
                 std::min(255, static_cast<int>(B() * scale)),
                 std::min(255, static_cast<int>(A() * scale)));
  }

  // Compound Assignment Operators (Avoids Creating New Objects)
  constexpr Pixel &operator|=(const Pixel &p) {
    packed |= p.packed;
    return *this;
  }
  constexpr Pixel &operator&=(const Pixel &p) {
    packed &= p.packed;
    return *this;
  }
  constexpr Pixel &operator^=(const Pixel &p) {
    packed ^= p.packed;
    return *this;
  }
  constexpr Pixel &operator+=(const Pixel &p) { return *this = *this + p; }
  constexpr Pixel &operator-=(const Pixel &p) { return *this = *this - p; }
  constexpr Pixel &operator*=(float scale) { return *this = *this * scale; }

  // Equality Operators
  constexpr bool operator==(const Pixel &p) const { return packed == p.packed; }
  constexpr bool operator!=(const Pixel &p) const { return packed != p.packed; }
};

class Image {
public:
  Image() = delete;
  Image(const ui width, const ui height)
      : m_width{width}, m_height{height}, m_pixel_data(width * height) {}
  Image(Image &I)
      : m_width{I.m_width}, m_height{I.m_height}, m_pixel_data{I.m_pixel_data} {
  }

  // Set pixel at position
  inline void SetPixel(const Pixel P, const ui width, const ui height);

  // Set pixel at position (no bounds checking)
  inline void fSetPixel(const Pixel P, const ui width, const ui height);

  // Returns reference to pixel at position
  inline Pixel &GetPixel(const ui width, const ui height);

  // Returns reference to pixel at position (no bounds checking)
  inline Pixel &fGetPixel(const ui width, const ui height);

  // Fill Image with given Pixel
  inline void Fill(const Pixel Pixel) {
    std::fill(m_pixel_data.begin(), m_pixel_data.end(), Pixel);
  }

  // Get reference to pixel data (mutable)
  inline std::vector<Pixel> &GetData() { return m_pixel_data; }

  // Get reference to pixel data (read-only)
  inline const std::vector<Pixel> &GetData() const { return m_pixel_data; }

  constexpr ui getWidth() const { return m_width; }
  constexpr ui getHeight() const { return m_height; }

  // Filepath can be realtive to cwd or absolute
  bool GenerateFile(const strv FilePath,
                    const ImageType Type = QOID::ImageType::qoi);

private:
  ui m_width{};
  ui m_height{};
  std::vector<Pixel> m_pixel_data;
};

inline void Image::SetPixel(const Pixel P, const ui width, const ui height) {
  if (width >= m_width || height >= m_height)
    throw std::out_of_range("Pixel coordinates out of bounds");
  fSetPixel(P, width, height);
}

inline void Image::fSetPixel(const Pixel P, const ui width, const ui height) {
  std::memcpy(&m_pixel_data[width + height * m_width], &P, sizeof(P));
}

inline Pixel &Image::GetPixel(const ui width, const ui height) {
  if (m_pixel_data.empty())
    throw std::runtime_error("Image has no pixel data.");
  if (width >= m_width || height >= m_height)
    throw std::out_of_range("Pixel coordinates out of bounds");
  return fGetPixel(width, height);
}

inline Pixel &Image::fGetPixel(const ui width, const ui height) {
  return m_pixel_data[width + height * m_width];
}
// } // namespace QOID

// #include "DataTypes/ImageFunctions/qoi.hpp"
// #include "DataTypes/ImageFunctions/tiff.hpp"
// namespace QOID {
// namespace qoi {
// bool GenerateFile(const Image &image, const strv FilePath); // Declare the
// function
// }
// } // namespace QOID

// namespace QOID {

namespace qoi { // forward declare the function
bool GenerateFile(const Image &image, const strv FilePath);
}
namespace tga { // forward declare the function
bool GenerateFile(const Image &image, const strv FilePath);
}

inline bool Image::GenerateFile(const strv FilePath, const ImageType Type) {
  if (FilePath.empty())
    throw std::invalid_argument("Filename is empty");

  switch (Type) {
  case ImageType::qoi:
    return qoi::GenerateFile(*this, FilePath);
  case ImageType::tga:
    return tga::GenerateFile(*this, FilePath);
  default:
    return false;
  }
}

namespace qoi {

static inline bool writeTrail(std::ostream &file) {
#if defined(BIG_ENDIAN)
  static constexpr uint64_t end_marker{0x0000000000000001};
#else
  static constexpr uint64_t end_marker{0x0100000000000000};
#endif
  return !!file.write(reinterpret_cast<const char *>(&end_marker),
                      sizeof(end_marker));
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

  return !!file.write(reinterpret_cast<const char *>(buffer.data()),
                      buffer.size());
}

namespace {

static inline bool writeDataNonCompressedNonOptimized(std::ostream &file,
                                                      const Image &image) {
  static uint8_t tmp{0xFF};
  for (auto i : image.GetData()) {
    file.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
    file.write(reinterpret_cast<const char *>(&i), sizeof(Pixel));
  }
  return true;
}

static inline void updateIndex(std::unordered_map<p_color, uint8_t> &SeenPixels,
                               const Pixel &px, unsigned int &SwapNum) {
  if (SeenPixels.size() >= 64) {
    auto it = SeenPixels.begin();
    std::advance(it, SwapNum); // Move iterator to SwapNum-th element
    SeenPixels.erase(it);      // Erase it in O(1) (on average)
  }

  SeenPixels[px.Pack()] =
      (px.R() * 3 + px.G() * 5 + px.B() * 7 + px.A() * 11) % 64;
  SwapNum = (SwapNum + 1) % 64; // Proper circular increment
}
static inline void
WriteFirstCol(std::vector<std::byte> &buffer,
              std::vector<Pixel>::const_iterator &DataIterator,
              std::unordered_map<p_color, uint8_t> &SeenPixels,
              size_t &bufferIndex, unsigned int &SwapNum) {
  static constexpr uint8_t Uint8Tmp{0xFF};
  std::memcpy(buffer.data() + bufferIndex, &Uint8Tmp, sizeof(Uint8Tmp));
  std::memcpy(buffer.data() + bufferIndex + 1, &(*DataIterator), sizeof(Pixel));
  updateIndex(SeenPixels, *DataIterator, SwapNum);
  ++DataIterator;
  bufferIndex += sizeof(Pixel) + sizeof(Uint8Tmp);
  return;
}

static inline void
WriteToBuffer(std::vector<std::byte> &buffer,
              std::vector<Pixel>::const_iterator &DataIterator,
              std::unordered_map<p_color, uint8_t> &SeenPixels,
              size_t &bufferIndex, const auto &DataEndIt,
              unsigned int &SwapNum) {
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
    if ((diffR >= -2 && diffR <= 1) && (diffG >= -2 && diffG <= 1) &&
        (diffB >= -2 && diffB <= 1)) { // Diff
      // Compute current minus previous.
      uint8_t diffR = static_cast<uint8_t>(current.R() - previous.R() + 2);
      uint8_t diffG = static_cast<uint8_t>(current.G() - previous.G() + 2);
      uint8_t diffB = static_cast<uint8_t>(current.B() - previous.B() + 2);
      const uint8_t comb{
          static_cast<uint8_t>(0x40 | (diffR << 4 | diffG << 2 | diffB))};
      std::memcpy(buffer.data() + bufferIndex, &comb, sizeof(comb));
      bufferIndex += 1;
      updateIndex(SeenPixels, current, SwapNum);
      ++DataIterator;
      return;
    }
    diffR -= diffG;
    diffB -= diffG;
    if ((diffG >= -32 && diffG <= 31) && (diffR >= -8 && diffR <= 7) &&
        (diffB >= -8 && diffB <= 7)) { // LUMA
      int8_t dg = static_cast<int8_t>(current.G() - previous.G());
      int8_t dr = static_cast<int8_t>(current.R() - previous.R()) - dg;
      int8_t db = static_cast<int8_t>(current.B() - previous.B()) - dg;
      uint8_t encodedDG = static_cast<uint8_t>(dg + 32); // Range: 0 to 63.
      uint8_t encodedDR = static_cast<uint8_t>(dr + 8);  // Range: 0 to 15.
      uint8_t encodedDB = static_cast<uint8_t>(db + 8);  // Range: 0 to 15.
#if defined(BIG_ENDIAN)
      const uint16_t comb{static_cast<uint16_t>(0x8000 | (encodedDG << 8) |
                                                (encodedDR << 4) | encodedDB)};
#else
      const uint16_t comb{std::byteswap(static_cast<uint16_t>(
          0x8000 | (encodedDG << 8) | (encodedDR << 4) | encodedDB))};
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
    std::memcpy(buffer.data() + bufferIndex + 1, &(*DataIterator),
                sizeof(Pixel));
    updateIndex(SeenPixels, *DataIterator, SwapNum);
    ++DataIterator;
    bufferIndex += sizeof(Pixel) + sizeof(Uint8Tmp);
    return;
  }
}

static inline bool writeData(std::ostream &file, const Image &image) {
  const size_t ImageSize{image.getHeight() * image.getWidth()};
  const auto &RawDataVec{image.GetData()};
  std::vector<std::byte> buffer(
      ImageSize * 5); // max possible size  this has to because of memcpy
  std::unordered_map<p_color, uint8_t> SeenPixels;
  auto DataIterator{RawDataVec.begin()};
  size_t bufferIndex = 0;
  unsigned int SwapNum{0};

  // The first pixel is always new.
  WriteFirstCol(buffer, DataIterator, SeenPixels, bufferIndex, SwapNum);
  for (; DataIterator != image.GetData().end();) {
    WriteToBuffer(buffer, DataIterator, SeenPixels, bufferIndex,
                  RawDataVec.end(), SwapNum);
  }

  // Write out the buffer in chunks.
  constexpr size_t chunkSize = 4096; // 4KB
  size_t chunkCount = bufferIndex / chunkSize;
  for (size_t pos = 0; pos < chunkCount; ++pos) {
    file.write(reinterpret_cast<const char *>(buffer.data() + pos * chunkSize),
               chunkSize);
  }
  // Write the remaining bytes.
  size_t remainder = bufferIndex % chunkSize;
  if (remainder)
    file.write(
        reinterpret_cast<const char *>(buffer.data() + chunkCount * chunkSize),
        remainder);
  return true;
}

} // namespace

inline bool GenerateFile(const Image &image, const strv FilePath) {
  std::ofstream file{FilePath.ends_with(".qoi")
                         ? FilePath.data()
                         : std::string(FilePath) + ".qoi",
                     std::ios::binary | std::ios::out};
  return writeHeader(file, image) && writeData(file, image) && writeTrail(file);
}

static inline bool GenerateFileNonCompressed(const Image &image,
                                             const strv FilePath) {
  std::ofstream file{FilePath.ends_with(".qoi")
                         ? FilePath.data()
                         : std::string(FilePath) + ".qoi",
                     std::ios::binary | std::ios::out};
  return writeHeader(file, image) &&
         writeDataNonCompressedNonOptimized(file, image) && writeTrail(file);
}

} // namespace qoi

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

  header[16] = 32; // Pixel depth: 32 bits per pixel (8 bits per channel)
  header[17] =
      0x28; // Image descriptor: 8-bit alpha, top-left origin (bit 5 set)

  return !!file.write(reinterpret_cast<const char *>(header.data()),
                      header.size());
}

// Writes the image data in BGRA order (TGA expects pixels stored as Blue,
// Green, Red, Alpha). Assumes that image.GetData() returns a container of
// Pixels in RGBA order.
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

// Generates a TGA file from the provided image. If FilePath does not end with
// ".tga", it will be appended.
inline bool GenerateFile(const Image &image, const strv FilePath) {
  std::string filePath;
  if (std::string(FilePath).ends_with(".tga"))
    filePath = FilePath;
  else
    filePath = std::string(FilePath) + ".tga";
  std::ofstream file{filePath, std::ios::binary | std::ios::out};
  if (!file)
    return false;
  return writeHeader(file, image) && writeData(file, image);
}

} // namespace tga
} // namespace QOID
