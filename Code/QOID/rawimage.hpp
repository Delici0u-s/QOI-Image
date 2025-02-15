#pragma once
#include "pixel.hpp"
#include <stdexcept>
#include <vector>

namespace QOID {
// unsigned int
using ui = unsigned int;

// Raw image, Has array of Pixels
class RawImage {
public:
  RawImage() = default;
  RawImage(const ui X, const ui Y) : size_x{X}, size_y{Y}, Data(X * Y) {}

  inline void SetPixel(const Pixel &P, const ui X, const ui Y) {
    if (X >= size_x || Y >= size_y) throw std::out_of_range("Pixel coordinates out of bounds");
    Data[X + Y * size_x] = P;
  }

  inline Pixel &GetPixel(const ui X, const ui Y) {
    if (X >= size_x || Y >= size_y) throw std::out_of_range("Pixel coordinates out of bounds");
    return Data[X + Y * size_x];
  }

  // Fill RawImage with given Pixel
  void Fill(const Pixel &P) { std::fill(Data.begin(), Data.end(), P); }

  void Resize(const ui new_x, const ui new_y) {
    std::vector<Pixel> newData(new_x * new_y);

    for (ui y = 0; y < std::min(size_y, new_y); ++y) {
      for (ui x = 0; x < std::min(size_x, new_x); ++x) {
        newData[x + y * new_x] = GetPixel(x, y);
      }
    }

    Data = std::move(newData);
    size_x = new_x;
    size_y = new_y;
  }

  Pixel *GetData() { return Data.data(); }
  const Pixel *GetData() const { return Data.data(); }

  // returns refrence to Pixel vector. nonconst can break stuff
  std::vector<Pixel> &GetRawVector() { return Data; }
  // returns refrence to Pixel vector.
  const std::vector<Pixel> &GetRawVector() const { return Data; }

  ui GetWidth() const { return size_x; }
  ui GetHeight() const { return size_y; }

  // Name input requres a .DNG extension
  // May throw exceptions:
  //    invalid_argument;
  //    runtime_error;
  bool GenerateRawFile(const std::string_view Name);

private:
  ui size_x{};
  ui size_y{};
  std::vector<Pixel> Data;
};

} // namespace QOID