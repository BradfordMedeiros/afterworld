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
#include "./navbar.h"
#include "./dock.h"
#include "./windowmanager.h"
#include "../components/worldplay.h"
#include "../components/scenemanager.h"

struct Level {
  std::string scene;
  std::string name;
};
struct LevelUIInterface {
  std::function<void(Level&)> goToLevel;
  std::function<std::vector<Level>()> getLevels;
  std::function<void()> goToMenu;
};
struct PauseInterface {
  float elapsedTime;
  std::function<void()> pause;
  std::function<void()> resume;
};

struct UiContext {
  std::function<bool()> isDebugMode;
  std::function<bool()> showScreenspaceGrid;
  LevelUIInterface levels;
  PauseInterface pauseInterface;
  WorldPlayInterface worldPlayInterface;

  std::function<std::vector<std::string>()> listScenes;
  std::function<void(std::string)> loadScene;
};

extern Component mainUI;

struct HandlerFns {
  int minManagedId;
  int maxManagedId;
  std::map<objid, std::function<void()>> handlerFns;
  std::map<objid, std::function<void(int)>> handlerFns2;
  std::map<objid, std::function<void(int)>> inputFns;
};
HandlerFns handleDrawMainUi(UiContext& pauseContext, std::optional<objid> selectedId);
void onMainUiScroll(double amount);
void onMainUiMousePress(HandlerFns& handlerFns, int button, int action, std::optional<objid> selectedId);
void onMainUiKeyPress(HandlerFns& handlerFns, int key);
void onObjectsChanged();
void pushHistory(std::string route);
std::string getCurrentPath();
void sendUiAlert(std::string message);


#endif

