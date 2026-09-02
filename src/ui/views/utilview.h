#ifndef MOD_AFTERWORLD_COMPONENTS_UTILVIEW
#define MOD_AFTERWORLD_COMPONENTS_UTILVIEW

#include "../components/common.h"
#include "./alert.h"
#include "../components/game/keyboard.h"
#include "./debug.h"

struct UtilViewOptions {
	bool showKeyboard;
	bool showScreenspaceGrid;
	std::string consoleKeyName;
	std::optional<glm::vec2> ndiCursor;
	std::optional<DebugConfig> debugConfig;
};

extern Component utilViewComponent;

#endif

