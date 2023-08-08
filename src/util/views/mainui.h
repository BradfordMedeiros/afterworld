#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include "../components/router.h"
#include "../components/list.h"
#include "../menus.h"

extern Component mainUI;
void handleDrawMainUi(DrawingTools& drawTools, std::optional<objid> selectedId);
void handleInputMainUi(std::optional<objid> selectedId);
void pushHistory(std::string route);

#endif