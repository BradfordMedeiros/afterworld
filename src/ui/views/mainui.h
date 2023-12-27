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
#include "./navbar.h"
#include "./dock/dock.h"
#include "./windowmanager.h"
#include "../components/worldplay.h"
#include "../components/scenemanager.h"
#include "./uicontext.h"
#include "../components/game/weaponwheel.h"
#include "../components/game/compass.h"
#include "../components/console.h"
#include "../components/uiwindow.h"

extern Component mainUI;

struct HandlerFns {
  int minManagedId;
  int maxManagedId;
  std::unordered_map<objid, std::function<void()>> handlerFns;
  std::unordered_map<objid, std::function<void(HandlerCallbackFn&)>> handlerCallbackFns;    
  std::unordered_map<objid, std::function<void(int)>> handlerFns2;
  std::unordered_map<objid, std::function<void(int)>> inputFns;
  std::unordered_map<objid, TrackedLocationData> trackedLocationIds;
};
HandlerFns handleDrawMainUi(UiContext& pauseContext, std::optional<objid> selectedId);
void onMainUiScroll(double amount);
void onMainUiMousePress(HandlerFns& handlerFns, int button, int action, std::optional<objid> selectedId);
void onMainUiKeyPress(HandlerFns& handlerFns, int key);
void onObjectsChanged();
void pushHistory(std::string route, bool replace = false);
void popHistory();
std::string getCurrentPath();
void sendUiAlert(std::string message);

#endif

