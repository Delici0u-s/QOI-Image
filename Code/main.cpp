#include "QOID/QOID_General.hpp"
#include "QOID/rawimage.hpp"

int main() {
  QOID::RawImage I{500, 500}; // RawFile max size is total pixels of 362*362=131044, idfk why ;-;
  // Timer T{};
  I.FillRegion({QOID::max, QOID::min, 10}, 0, 0, 50, 50);
  I.GenerateRawFile("testimage.tiff");

  return 0;
}
