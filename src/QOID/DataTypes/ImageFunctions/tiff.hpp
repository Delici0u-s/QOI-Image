#pragma once
#include "../../QOID_General.hpp"
#include "../../image.hpp"

#include <fstream>

// to be implemented
namespace QOID {
namespace tiff {

inline bool GenerateFile(const Image &image, const strv FilePath) {
  std::ofstream file{FilePath.ends_with(".tiff") ? FilePath.data() : std::string(FilePath) + ".tiff",
                     std::ios::binary | std::ios::out};
  file << "no" << image.getHeight();
  return true;
}

} // namespace tiff

} // namespace QOID