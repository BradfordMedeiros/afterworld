#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_IMAGE
#define MOD_AFTERWORLD_COMPONENTS_DOCK_IMAGE

#include "../dock_util.h"
#include "../../../components/basic/textbox.h"

struct DockImageConfig {
  std::string label;
  std::function<void(std::string)> onImageSelect;
};

Component createDockImage(DockImageConfig& dockFile);

#endif

