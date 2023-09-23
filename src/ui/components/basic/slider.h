#ifndef MOD_AFTERWORLD_COMPONENTS_SLIDER
#define MOD_AFTERWORLD_COMPONENTS_SLIDER

#include "../common.h"
#include "../../../global.h"
#include "./list.h"

struct Slider {
  float min;
  float max;
  float percentage;
  std::function<void(float)> onSlide;
};

extern Component slider;


#endif