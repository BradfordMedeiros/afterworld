#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include "../components/router.h"
#include "../components/list.h"
#include "../menus.h"
#include "./pausemenu.h"

struct PauseContext {
  float elapsedTime;
  std::function<void()> pause;
  std::function<void()> resume;
  bool shouldShowPauseMenu;
  bool showAnimationMenu;
};
Props pauseMenuProps(std::optional<objid> mappingId, PauseContext pauseContext);

extern Component mainUI;
void handleDrawMainUi(PauseContext& pauseContext, DrawingTools& drawTools, std::optional<objid> selectedId);
void handleInputMainUi(PauseContext& pauseContext, std::optional<objid> selectedId);
void pushHistory(std::string route);

#endif