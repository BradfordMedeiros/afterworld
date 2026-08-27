#ifndef MOD_AFTERWORLD_COMPONENTS_UICONTEXT
#define MOD_AFTERWORLD_COMPONENTS_UICONTEXT

#include "../components/console.h"
#include "./ball.h"

struct Level {
  std::string scene;
  std::string name;
};

struct LevelUIInterface {
  std::function<void(Level&)> goToLevel;
  std::function<void()> goToMenu;
};
struct PauseInterface {
  std::function<void()> pause;
  std::function<void()> resume;
};


struct PauseOptions {
   std::function<void()> resume;
   std::function<void()> mainMenu;
};

struct TerminalConfig;
struct ZoomOptions;
struct ScoreOptions;
struct DebugConfig;
struct RouterHistory;

struct UiContext {
  // UI Options
  std::function<bool()> isDebugMode;
  std::function<bool()> showEditor;
  std::function<bool()> showConsole;
  std::function<bool()> showScreenspaceGrid;
  std::function<std::optional<PauseOptions>()> pauseOptions;
  std::function<std::optional<ZoomOptions>()> showZoomOverlay;
  std::function<bool()> showKeyboard;
  std::function<std::optional<DebugConfig>()> debugConfig;

  std::function<std::optional<ScoreOptions>()> getScoreConfig;

  // api for the ui
  LevelUIInterface levels;
  PauseInterface pauseInterface;

  std::function<void()> playSound;

  ConsoleInterface consoleInterface;
};


struct UiManagerContext {
	UiContext* uiContext;
};

#endif
