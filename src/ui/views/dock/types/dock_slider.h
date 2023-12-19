#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_SLIDER
#define MOD_AFTERWORLD_COMPONENTS_DOCK_SLIDER

#include "../dock_util.h"

struct DockSliderConfig {
  std::string label;
  float min;
  float max;
  std::function<float()> percentage;
  std::function<void(float)> onSlide;
};

#endif

