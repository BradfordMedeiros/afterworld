#ifndef MOD_AFTERWORLD_INGAMEUI
#define MOD_AFTERWORLD_INGAMEUI

#include <iostream>
#include <vector>
#include "./util.h"
#include "./activeplayer.h"
#include "./ui/views/mainui.h"

struct TextDisplay {
	objid textureId;
	HandlerFns handlerFns;
};


struct InGameUi {
	std::map<objid, TextDisplay> textDisplays;
};

void setShowSelectionTexture(bool shouldShow);
void createInGamesUiInstance(InGameUi& inGameUi, objid id);
void freeInGameUiInstance(InGameUi& inGameUi, objid id);
void onInGameUiFrame(InGameUi& inGameUi, UiContext& uiContext, std::optional<objid> textureId, glm::vec2 ndiCoord);
void zoomIntoGameUi(objid id);
std::optional<objid>  getAnyUiInstance(InGameUi& inGameUi);
void onInGameUiSelect(InGameUi& inGameUi, int button, int action, std::optional<objid> selectedId);
void testInGameUiSetText(std::string value);

#endif 