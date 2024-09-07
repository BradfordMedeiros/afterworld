#ifndef MOD_AFTERWORLD_COMPONENTS_PAUSEMENU
#define MOD_AFTERWORLD_COMPONENTS_PAUSEMENU

#include "../components/common.h"
#include "../components/basic/listitem.h"
#include "./uicontext.h"

extern Component pauseMenuComponent;

Props pauseMenuProps(std::optional<objid> mappingId, UiContext& uiContext);
Props deadMenuProps(std::optional<objid> mappingId, UiContext& uiContext);

#endif

