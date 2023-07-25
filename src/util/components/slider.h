#ifndef MOD_AFTERWORLD_COMPONENTS_SLIDER
#define MOD_AFTERWORLD_COMPONENTS_SLIDER

#include "./common.h"
#include "../../global.h"

struct Slider {
  float min;
  float max;
  float percentage;
  objid mappingId;
};

extern Slider slider;

extern Component sliderSelector;

#endif