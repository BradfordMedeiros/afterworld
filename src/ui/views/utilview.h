#ifndef MOD_AFTERWORLD_COMPONENTS_UTILVIEW
#define MOD_AFTERWORLD_COMPONENTS_UTILVIEW

#include "../components/common.h"
#include "../components/game/keyboard.h"

struct UtilViewOptions {
	bool showKeyboard;
	bool showScreenspaceGrid;
	std::string consoleKeyName;
	std::optional<glm::vec2> ndiCursor;
};

extern Component utilViewComponent;

#endif

