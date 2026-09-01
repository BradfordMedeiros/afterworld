#ifndef MOD_AFTERWORLD_COMPONENTS_UICONTEXT
#define MOD_AFTERWORLD_COMPONENTS_UICONTEXT

#include "../components/console.h"

struct Level {
  std::string scene;
  std::string name;
};

struct LevelUIInterface {
  std::function<void(Level&)> goToLevel;
  std::function<void()> goToMenu;
};


struct TerminalConfig;
struct ZoomOptions;
struct DebugConfig;
struct RouterHistory;

struct UiContext {
  // UI Options
  std::function<bool()> isDebugMode;
  std::function<bool()> showConsole;
  std::function<bool()> showScreenspaceGrid;
  std::function<std::optional<ZoomOptions>()> showZoomOverlay;
  std::function<bool()> showKeyboard;
  std::function<std::optional<DebugConfig>()> debugConfig;

  // api for the ui
  LevelUIInterface levels;

  std::function<void()> playSound;

  ConsoleInterface consoleInterface;
};


struct UiManagerContext {
	UiContext* uiContext;
};

#endif
