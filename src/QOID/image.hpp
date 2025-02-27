#pragma once
#include "QOID_General.hpp"
#include "DataTypes/pixel.hpp"
#include <cstring>
#include <vector>
#include <stdexcept>

namespace QOID {

class Image {
public:
  Image() = delete;
  Image(const ui width, const ui height) : m_width{width}, m_height{height}, m_pixel_data(width * height) {}
  Image(Image &I) : m_width{I.m_width}, m_height{I.m_height}, m_pixel_data{I.m_pixel_data} {}

  // Set pixel at position
  inline void SetPixel(const Pixel P, const ui width, const ui height);

  // Set pixel at position (no bounds checking)
  inline void fSetPixel(const Pixel P, const ui width, const ui height);

  // Returns reference to pixel at position
  inline Pixel &GetPixel(const ui width, const ui height);

  // Returns reference to pixel at position (no bounds checking)
  inline Pixel &fGetPixel(const ui width, const ui height);

  // Fill Image with given Pixel
  inline void Fill(const Pixel Pixel) { std::fill(m_pixel_data.begin(), m_pixel_data.end(), Pixel); }

  // Get reference to pixel data (mutable)
  inline std::vector<Pixel> &GetData() { return m_pixel_data; }

  // Get reference to pixel data (read-only)
  inline const std::vector<Pixel> &GetData() const { return m_pixel_data; }

  constexpr ui getWidth() const { return m_width; }
  constexpr ui getHeight() const { return m_height; }

  // Filepath can be realtive to cwd or absolute
  bool GenerateFile(const ImageType Type, const strv FilePath);

private:
  ui m_width{};
  ui m_height{};
  std::vector<Pixel> m_pixel_data;
};

inline void Image::SetPixel(const Pixel P, const ui width, const ui height) {
  if (width >= m_width || height >= m_height) throw std::out_of_range("Pixel coordinates out of bounds");
  fSetPixel(P, width, height);
}

inline void Image::fSetPixel(const Pixel P, const ui width, const ui height) {
  std::memcpy(&m_pixel_data[width + height * m_width], &P, sizeof(P));
}

inline Pixel &Image::GetPixel(const ui width, const ui height) {
  if (m_pixel_data.empty()) throw std::runtime_error("Image has no pixel data.");
  if (width >= m_width || height >= m_height) throw std::out_of_range("Pixel coordinates out of bounds");
  return fGetPixel(width, height);
}

inline Pixel &Image::fGetPixel(const ui width, const ui height) { return m_pixel_data[width + height * m_width]; }
// } // namespace QOID

// #include "DataTypes/ImageFunctions/qoi.hpp"
// #include "DataTypes/ImageFunctions/tiff.hpp"
// namespace QOID {
// namespace qoi {
// bool GenerateFile(const Image &image, const strv FilePath); // Declare the function
// }
// } // namespace QOID

// namespace QOID {

namespace qoi { // forward declare the function
bool GenerateFile(const Image &image, const strv FilePath);
}
namespace tiff { // forward declare the function
bool GenerateFile(const Image &image, const strv FilePath);
}

inline bool Image::GenerateFile(const ImageType Type, const strv FilePath) {
  if (FilePath.empty()) throw std::invalid_argument("Filename is empty");

  switch (Type) {
  case ImageType::tiff: return tiff::GenerateFile(*this, FilePath);
  case ImageType::qoi: return qoi::GenerateFile(*this, FilePath);
  default: return false;
  }
}
} // namespace QOID
