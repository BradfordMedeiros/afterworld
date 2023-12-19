#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_CHECKBOX
#define MOD_AFTERWORLD_COMPONENTS_DOCK_CHECKBOX

#include "../dock_util.h"

struct DockCheckboxConfig {
  std::string label;
  std::function<bool()> isChecked;
  std::function<void(bool)> onChecked;
};

#endif

