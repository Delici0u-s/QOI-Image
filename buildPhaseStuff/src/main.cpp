#include "QOID/image.hpp"

#include "Timer.h"
#include <iostream>

int main() {
  // QOID::Image I{2048, 4024};
  QOID::Image I{4024, 2048};

  // for (QOID::ui i = 0; i < I.getWidth(); ++i)
  //   for (QOID::ui j = 0; j < I.getHeight(); ++j)
  //     I.SetPixel({static_cast<uint8_t>(i % 256), static_cast<uint8_t>(j % 256), 128, 255}, i, j);

  // I.Fill({QOID::min, QOID::min, QOID::max, QOID::max});
  // // set upper right quadrant yellow
  // for (QOID::ui i{0}; i < I.getWidth() / 2; ++i)
  //   for (QOID::ui j{0}; j < I.getHeight() / 2; ++j)
  //     I.SetPixel({255, 255, 0}, i, j);

  I.Fill({0, 0, 0, 255}); // Clear image to black (or any base color)
  for (QOID::ui i = 0; i < I.getWidth(); ++i) {
    for (QOID::ui j = 0; j < I.getHeight(); ++j) {
      // Scale 'i' and 'j' so that the values range from 0 to 255 over the entire image.
      uint8_t r = static_cast<uint8_t>((i * 255) / (I.getWidth() - 1));
      uint8_t g = static_cast<uint8_t>((j * 255) / (I.getHeight() - 1));
      uint8_t b = 128; // Constant blue value for example.
      I.SetPixel({r, g, b, 255}, i, j);
    }
  }

  // I.GenerateFile(QOID::ImageType::qoi, "Test");
  Timer T{};
  I.GenerateFile("Compressed");
  std::cout << "Compressed elapsed: " << T.delapsed() << '\n';
  // I.GenerateFile("tgaTest", QOID::ImageType::tga);
  // T.reset();
  // std::cout << "tga elapsed: " << T.delapsed() << '\n';

  return 0;
}
