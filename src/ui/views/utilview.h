#ifndef MOD_AFTERWORLD_COMPONENTS_UTILVIEW
#define MOD_AFTERWORLD_COMPONENTS_UTILVIEW

#include "../components/common.h"
#include "./alert.h"
#include "../components/game/keyboard.h"
#include "../components/console.h"

struct UtilViewOptions {
	bool showKeyboard;
	bool showConsole;
	std::string consoleKeyName;
	ConsoleInterface* consoleInterface;
};

extern Component utilViewComponent;

#endif

