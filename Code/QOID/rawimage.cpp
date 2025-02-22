#include "rawimage.hpp"
#include "QOID_General.hpp"
#include "pixel.hpp"
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include "qoi.hpp"
#include "tiff.hpp"

namespace QOID {
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

bool RawImage::GenerateFile(const ImageType Type, const strv FileName, const std::filesystem::path Location) {
  // check if inputs are valid, create a filepath
  if (FileName.empty()) throw std::invalid_argument("Filename is empty");
  if (!std::filesystem::is_directory(Location)) throw std::invalid_argument("Location is not valid");
  std::filesystem::path tmp{Location / FileName};

  switch (Type) {
  case QOID::RawImage::ImageType::tiff: return tiff::GenerateRawFile(*this, tmp);
  case QOID::RawImage::ImageType::qoi: return qoi::GenerateQoiFile(*this, tmp);
  }
  return {};
}
} // namespace QOID