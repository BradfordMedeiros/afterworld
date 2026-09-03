#ifndef MOD_AFTERWORLD_COMPONENTS_UICONTEXT
#define MOD_AFTERWORLD_COMPONENTS_UICONTEXT

struct Level {
  std::string scene;
  std::string name;
};


struct TerminalConfig;
struct DebugConfig;
struct RouterHistory;

struct UiContext {
  // UI Options
  std::function<bool()> isDebugMode;
  std::function<bool()> showScreenspaceGrid;
  std::function<bool()> showKeyboard;
  std::function<std::optional<DebugConfig>()> debugConfig;
  std::function<void()> playSound;
};


struct UiManagerContext {
	UiContext* uiContext;
};

#endif
