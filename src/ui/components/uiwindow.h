#ifndef MOD_AFTERWORLD_COMPONENTS_UIWINDOW
#define MOD_AFTERWORLD_COMPONENTS_UIWINDOW

#include "./common.h"
#include "./basic/layout.h"
#include "./basic/listitem.h"
#include "../views/windowmanager.h"
#include "../views/style.h"

Component createUiWindow(Component& component, int symbol, std::function<void()> onClickX, std::string titleValue = "Untitled Window", AlignmentParams alignment = defaultAlignment);
#endif