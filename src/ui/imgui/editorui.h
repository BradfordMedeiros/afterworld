#ifndef MOD_AFTERWORLD_EDITORUI
#define MOD_AFTERWORLD_EDITORUI

#include "../../../../ModEngine/src/ui/gui.h"
#include "../../scene_routing.h"
#include "../../resources/sound.h"
#include "./widgets/fps.h"
#include "./widgets/sound.h"
#include "./widgets/ball.h"
#include "./widgets/settings.h"
#include "./widgets/util.h"
#include "./common.h"




struct UiSettings {
  bool showMainMenu = false;
  bool showGameSettings = false;
  bool showPauseMenu = false;
  bool showDeadMenu = false;
  bool showLevelSelect = false;
  std::optional<LiveMenuFn> liveMenu;
  BallModeUi* ballModeUi = NULL;
  bool showConsole = false;
  bool showFpsHud = false;
};
UiSettings* getUiSettings();

void initImGuiGameUi();


#endif 