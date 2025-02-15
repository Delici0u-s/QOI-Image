#include "rawimage.hpp"
#include <cstring>
#include <ios>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace QOID {
bool RawImage::GenerateRawFile(const std::string_view Name) {
  if (!Name.ends_with(".DNG")) throw std::invalid_argument{"Not a .DNG file"};
  std::ofstream file{Name.data(), std::ios::binary | std::ios::out};
  if (!file.is_open()) throw std::runtime_error{"fine couldnt be opened"};

  // buffer of each row of data.
  constexpr size_t chunkSize = 4096; // 4KB chunk size
  std::vector<Pixel> buffer(std::min(chunkSize / sizeof(Pixel), static_cast<size_t>(size_x)));

  for (ui row = 0; row < size_y; ++row) {
    std::memcpy(buffer.data(), &Data[row * size_x], buffer.size() * sizeof(Pixel));
    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size() * sizeof(Pixel));
  }

  file.close();
  return true;
}

} // namespace QOID