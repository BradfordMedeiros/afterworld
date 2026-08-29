#ifndef MOD_AFTERWORLD_COMPONENTS_PAUSEMENU
#define MOD_AFTERWORLD_COMPONENTS_PAUSEMENU

#include "../components/common.h"
#include "../components/basic/listitem.h"
#include "./uicontext.h"

extern Component pauseMenuComponent;

Props deadMenuProps(std::optional<objid> mappingId, std::function<void()> goToMenu);

#endif

