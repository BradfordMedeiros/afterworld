#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_CHECKBOX
#define MOD_AFTERWORLD_COMPONENTS_DOCK_CHECKBOX

#include "../dock_util.h"
#include "../../../components/basic/checkbox.h"

struct DockCheckboxConfig {
  std::string label;
  std::function<bool()> isChecked;
  std::function<void(bool)> onChecked;
};

Component createDockCheckbox(DockCheckboxConfig& dockCheckbox);

#endif

