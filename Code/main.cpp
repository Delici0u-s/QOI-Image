#include "QOID/QOID_General.hpp"
#include "QOID/rawimage.hpp"

int main() {
  QOID::RawImage I{1, 1}; // RawFile max size is total pixels of 362*362=131044, idfk why ;-;
  I.Fill({0, 0, 255, 255});
  // Timer T{};
  // I.FillRegion({QOID::max, QOID::min, 10}, 0, 0, 250, 250);
  I.GenerateFile(QOID::RawImage::ImageType::qoi, "snapchat");
  I.GenerateFile(QOID::RawImage::ImageType::tiff, "snapchat");

  return 0;
}
