#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_BUTTON
#define MOD_AFTERWORLD_COMPONENTS_DOCK_BUTTON
#include "../dock_util.h"

struct DockButtonConfig {
  const char* buttonText;
  std::function<void()> onClick;
};

#endif

