#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_GAMEOBJ
#define MOD_AFTERWORLD_COMPONENTS_DOCK_GAMEOBJ

#include "../dock_util.h"
#include "../../../components/basic/textbox.h"

struct DockGameObjSelector {
  std::optional<std::string> label;
  std::function<void(std::string&)> onSelect;
};

Component createDockGameobj(DockGameObjSelector& dockFile);

#endif

