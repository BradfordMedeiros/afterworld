#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_LABEL
#define MOD_AFTERWORLD_COMPONENTS_DOCK_LABEL

#include "../dock_util.h"
#include "../../../components/basic/textbox.h"

struct DockLabelConfig {
  std::function<std::string()> label;
};

Component createDockLabel(DockLabelConfig& dockLabel);


#endif

