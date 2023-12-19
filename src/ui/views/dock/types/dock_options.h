#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_OPTIONS
#define MOD_AFTERWORLD_COMPONENTS_DOCK_OPTIONS

#include "../dock_util.h"

struct DockOptionConfig {
  std::vector<const char*> options;
  std::function<void(std::string&, int)> onClick;
  std::function<int(void)> getSelectedIndex;
};

#endif

