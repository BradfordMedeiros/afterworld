#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include "../components/router.h"
#include "../components/list.h"
#include "../components/slider.h"
#include "../components/radiobutton.h"
#include "../components/imagelist.h"
#include "../components/dialog.h"
#include "../components/fileexplorer.h"
#include "../components/colorpicker.h"
#include "../components/scenegraph.h"
#include "./debuglist.h"
#include "./pausemenu.h"
#include "./navbar.h"
#include "./dock.h"
#include "./windowmanager.h"

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
};

extern Component mainUI;

std::map<objid, std::function<void()>> handleDrawMainUi(UiContext& pauseContext, std::optional<objid> selectedId);
void onMainUiScroll(double amount);
void onMainUiMousePress(std::optional<objid> selectedId);
void onMainUiMouseRelease();
void pushHistory(std::string route);
std::string getCurrentPath();

#endif

