#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_FILE
#define MOD_AFTERWORLD_COMPONENTS_DOCK_FILE

#include "../dock_util.h"
#include "../../../components/basic/textbox.h"

struct DockFileConfig {
  std::string label;
};

Component createDockFile(DockFileConfig& dockFile);

#endif

