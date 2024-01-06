#ifndef MOD_AFTERWORLD_COMPONENTS_DOCK_SELECT
#define MOD_AFTERWORLD_COMPONENTS_DOCK_SELECT

#include "../dock_util.h"
#include "../../../components/basic/select.h"

struct DockSelectConfig {
	SelectOptions selectOptions;
};

Component createDockSelect(DockSelectConfig& dockOption);

#endif

