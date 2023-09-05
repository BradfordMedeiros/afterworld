#ifndef MOD_AFTERWORLD_COMPONENTS_UIWINDOW
#define MOD_AFTERWORLD_COMPONENTS_UIWINDOW

#include "./common.h"
#include "./layout.h"
#include "./listitem.h"

Component createUiWindow(Component& component, std::string& titleValue, std::function<void()>& onClick, std::optional<std::function<void()>>& onClickX);
#endif