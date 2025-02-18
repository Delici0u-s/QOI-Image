#include "QOID/QOID_General.hpp"
#include "QOID/rawimage.hpp"

#include "D:\progremming\.Synced\Languages\cpp\ownlibs\LearnCpp.com\Timer(chronos)\Timer.h"
#include <iostream>

int main() {
  QOID::RawImage I{362, 361}; // RawFile max size is total pixels of 362*362=131044, idfk why ;-;
  Timer T{};
  I.FillRegion({QOID::max, QOID::min, 10}, 0, 0, 50, 50);
  T.reset();
  I.GenerateRawFile("testimage.DNG");
  std::cout << "MakingRawFile: " << T.delapsed() << '\n';

  return 0;
}
