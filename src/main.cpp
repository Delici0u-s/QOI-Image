#include "QOID/DataTypes/ImageFunctions/qoi.hpp"
#include "QOID/QOID_General.hpp"
#include "QOID/image.hpp"

#include "Timer.h"
#include <iostream>

int main() {
  QOID::Image I{20, 40};
  I.Fill({QOID::min, QOID::min, QOID::max, QOID::max});
  // set upper right quadrant yellow
  for (QOID::ui i{0}; i < I.getWidth() / 2; ++i)
    for (QOID::ui j{0}; j < I.getHeight() / 2; ++j)
      I.SetPixel({0x3C, 0x4C, 0x24}, i, j);
  // I.GenerateFile(QOID::ImageType::qoi, "Test");

  Timer T{};
  QOID::qoi::GenerateFile(I, "Compressed");
  std::cout << "Compressed Time: " << T.delapsed() << '\n';
  T.reset();
  QOID::qoi::GenerateFileNonCompressed(I, "Uncompr");
  std::cout << "Uncompressed Time: " << T.delapsed() << '\n';

  return 0;
}
