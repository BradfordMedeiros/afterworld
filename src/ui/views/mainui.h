#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include "../components/router.h"
#include "../components/basic/list.h"
#include "../components/basic/slider.h"
#include "../components/basic/radiobutton.h"
#include "../components/imagelist.h"
#include "../components/dialog.h"
#include "../components/fileexplorer.h"
#include "../components/colorpicker.h"
#include "../components/scenegraph.h"
#include "./alert.h"
#include "./navlist.h"
#include "./pausemenu.h"
#include "./levelselect.h"
#include "./settings.h"
#include "./modelviewer.h"
#include "./navbar.h"
#include "./dock/dock.h"
#include "./windowmanager.h"
#include "../components/worldplay.h"
#include "../components/scenemanager.h"
#include "./uicontext.h"
#include "../components/game/weaponwheel.h"
#include "../components/game/compass.h"
#include "../components/game/hud.h"
#include "../components/game/terminal.h"
#include "../components/game/zoom.h"
#include "../components/game/score.h"
#include "../components/game/keyboard.h"
#include "../components/console.h"
#include "../components/uiwindow.h"
#include "./debug.h"

//// organize this into a single file for dock config api
#include "../../modelviewer.h"

extern Component mainUI;

struct AutoFocusObj {
  objid id;
  std::string key;
};

struct HandlerFns {
  int minManagedId;
  int maxManagedId;
  std::unordered_map<objid, std::function<void()>> handlerFns;
  std::unordered_map<objid, std::function<void(HandlerCallbackFn&)>> handlerCallbackFns;    
  std::unordered_map<objid, std::function<void(int)>> handlerFns2;
  std::unordered_map<objid, std::function<void(int, int)>> inputFns;
  std::unordered_map<objid, TrackedLocationData> trackedLocationIds;
  std::optional<AutoFocusObj> autofocus;
};
HandlerFns handleDrawMainUi(UiContext& pauseContext, std::optional<objid> selectedId, std::optional<unsigned int> textureId);
void onMainUiScroll(double amount);
void onMainUiMousePress(HandlerFns& handlerFns, int button, int action, std::optional<objid> selectedId);
void onMainUiKeyPress(HandlerFns& handlerFns, int key, int scancode, int action, int mods);
void onMainUiObjectsChanged();
void pushHistory(std::vector<std::string> route, bool replace = false);
void pushHistoryParam(std::string param);
void rmHistoryParam(std::string param);


void popHistory();
std::string getCurrentPath();
std::vector<std::string> historyParams();
std::string fullHistoryStr();
std::optional<std::string> getPathParts(int index);
void sendUiAlert(std::string message);

#endif

