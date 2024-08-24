#ifndef MOD_AFTERWORLD_INGAMEUI
#define MOD_AFTERWORLD_INGAMEUI

#include <iostream>
#include <vector>
#include "./util.h"
#include "./activeplayer.h"
#include "./ui/views/mainui.h"

struct TextDisplay {
	std::string texture;
	std::string channel;
	std::string text;
	glm::vec2 textPosition;
	objid textureId;
	bool needsRefresh;
};


struct InGameUi {
	std::map<objid, TextDisplay> textDisplays;
};


void createInGamesUiInstance(InGameUi& inGameUi, objid id);
void freeInGameUiInstance(InGameUi& inGameUi, objid id);
void onInGameUiFrame(InGameUi& inGameUi, UiContext& uiContext);
void zoomIntoGameUi(objid id);
std::optional<objid>  getAnyUiInstance(InGameUi& inGameUi);
void onInGameUiMessage(InGameUi& inGameUi, std::string& key, std::any& value);
void testInGameUiSetText(std::string value);

#endif 