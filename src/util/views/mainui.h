#ifndef MOD_AFTERWORLD_COMPONENTS_INDEX
#define MOD_AFTERWORLD_COMPONENTS_INDEX

#include "../components/router.h"
#include "../components/list.h"
#include "../menus.h"
#include "./pausemenu.h"

struct Level {
  std::string scene;
  std::string name;
};
struct LevelUIInterface {
  std::function<void(Level&)> goToLevel;
  std::function<std::vector<Level>()> getLevels;
};
struct PauseInterface {
  float elapsedTime;
  std::function<void()> pause;
  std::function<void()> resume;

};

struct UiContext {
  bool showAnimationMenu;
  bool onMainMenu;
  bool showScreenspaceGrid;
  LevelUIInterface levels;
  PauseInterface pauseInterface;
};
Props pauseMenuProps(std::optional<objid> mappingId, UiContext pauseContext);

extern Component mainUI;

//struct InputHandlers {
//	std::map<objid, std::function<void()>> mappingidToFn;
//	std::map<objid, Component> registeredComponentList;
//	std::map<objid, void*> componentData;
//};
//void processInputMainUi(InputHandlers& handlers, std::optional<objid> selectedId);

void handleDrawMainUi(UiContext& pauseContext, DrawingTools& drawTools, std::optional<objid> selectedId);
void handleInputMainUi(UiContext& pauseContext, std::optional<objid> selectedId);
void pushHistory(std::string route);

std::vector<ImListItem> animationMenuItems2();
std::vector<Component> mainMenuItems2();

#endif