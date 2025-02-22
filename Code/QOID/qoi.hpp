#pragma once
#include "rawimage.hpp"
#include "QOID_General.hpp"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ostream>

namespace QOID {

namespace qoi {
inline uint8_t NewColIndic{0xFF};

bool writeHeader(std::ofstream &file, uint32_t size_x, uint32_t size_y);
bool writeTrail(std::ostream &file);
bool GenerateQoiFile(const QOID::RawImage &RawImage, std::filesystem::path &FileLocation);

} // namespace qoi
} // namespace QOID