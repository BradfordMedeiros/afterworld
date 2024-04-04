#ifndef MOD_AFTERWORLD_INGAMEUI
#define MOD_AFTERWORLD_INGAMEUI

#include <iostream>
#include <vector>
#include "./util.h"
#include "./activeplayer.h"


struct TestUiInstance {
	glm::vec2 cursorLocation;
	objid textureId;
	bool upSelected;
};

struct InGameUi {
	std::set<objid> ids;
	std::optional<TestUiInstance> uiInstance;
};


void createInGamesUiInstance(InGameUi& inGameUi, objid id);
void freeInGameUiInstance(InGameUi& inGameUi, objid id);
void onInGameUiFrame(InGameUi& inGameUi);
void zoomIntoGameUi(objid id);
std::optional<objid>  getAnyUiInstance(InGameUi& inGameUi);
void onInGameUiMessage(InGameUi& inGameUi, std::string& message);

#endif 