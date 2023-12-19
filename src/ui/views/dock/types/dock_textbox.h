#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_TEXTBOX
#define MOD_AFTERWORLD_COMPONENTS_DOCK_TEXTBOX
#include "../dock_util.h"

struct DockTextboxConfig {
  std::string label;
  std::function<std::string(void)> text;
  std::function<void(std::string)> onEdit;
};

#endif

