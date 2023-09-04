#ifndef MOD_AFTERWORLD_COMPONENTS_UIWINDOW
#define MOD_AFTERWORLD_COMPONENTS_UIWINDOW

#include "./common.h"
#include "./layout.h"
#include "./listitem.h"

Component createUiWindow(std::vector<Component>& components, std::string& titleValue, std::function<void()> onClick);
#endif