#ifndef MOD_AFTERWORLD_COMPONENTS_UICONTEXT
#define MOD_AFTERWORLD_COMPONENTS_UICONTEXT

struct Level {
  std::string scene;
  std::string name;
};


struct TerminalConfig;
struct RouterHistory;

struct UiContext {
  // UI Options
  std::function<bool()> showKeyboard;
  std::function<void()> playSound;
};


struct UiManagerContext {
	UiContext* uiContext;
};

#endif
