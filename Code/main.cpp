#include "QOID/rawimage.hpp"

int main() {
  QOID::RawImage I{4096, 2160};
  // QOID::RawImage I{100, 100};
  I.GenerateRawFile("testimage.DNG");

  return 0;
}
