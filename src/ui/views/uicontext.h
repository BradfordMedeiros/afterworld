#ifndef MOD_AFTERWORLD_COMPONENTS_UICONTEXT
#define MOD_AFTERWORLD_COMPONENTS_UICONTEXT

#include "../components/worldplay.h"
#include "../components/console.h"

struct Level {
  std::string scene;
  std::string name;
};
struct LevelUIInterface {
  std::function<void(Level&)> goToLevel;
  std::function<void()> goToMenu;
};
struct PauseInterface {
  std::function<float()> elapsedTime;
  std::function<void()> pause;
  std::function<void()> resume;
};

struct TerminalConfig;
struct UiContext {
  std::function<bool()> isDebugMode;
  std::function<bool()> showEditor;
  std::function<bool()> showConsole;
  std::function<bool()> showScreenspaceGrid;
  std::function<bool()> showGameHud;
  std::function<std::optional<TerminalConfig>()> showTerminal;
  std::function<bool()> showZoomOverlay;

  LevelUIInterface levels;
  PauseInterface pauseInterface;
  WorldPlayInterface worldPlayInterface;

  std::function<std::vector<std::string>()> listScenes;
  std::function<void(std::string)> loadScene;
  std::function<void()> saveScene;
  std::function<void(std::string)> newScene;
  std::function<void()> resetScene;
  std::function<std::optional<objid>()> activeSceneId;

  std::function<void()> showPreviousModel;
  std::function<void()> showNextModel;

  ConsoleInterface consoleInterface;
};


struct UiMainContext {
	std::function<void(std::function<void(bool closedWithoutInput, std::string input)>)> openNewSceneMenu;
};

struct UiManagerContext {
	UiContext* uiContext;
	UiMainContext uiMainContext;

};

#endif
