#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_FILE
#define MOD_AFTERWORLD_COMPONENTS_DOCK_FILE

#include "../dock_util.h"
#include "../../../components/basic/textbox.h"

struct DockFileConfig {
  std::string label;
  std::optional<int> displayLimit;
  std::function<void(std::string&)> onFileSelected;
  std::function<bool(bool, std::string&)> filterFilter;
};

Component createDockFile(DockFileConfig& dockFile);

#endif

