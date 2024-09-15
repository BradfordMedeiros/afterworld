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
	glm::vec2 mouseCoordNdc;
	std::optional<RouterHistory> routerHistory;
};


struct InGameUi {
	std::map<objid, TextDisplay> textDisplays;
};

void setShowSelectionTexture(bool shouldShow);
void createInGamesUiInstance(InGameUi& inGameUi, objid id);
void freeInGameUiInstance(InGameUi& inGameUi, objid id);
void zoomIntoGameUi(objid id);
std::optional<objid>  getAnyUiInstance(InGameUi& inGameUi);
void onInGameUiFrame(UiStateContext& uiState, InGameUi& inGameUi, UiContext& uiContext, std::optional<objid> textureId, glm::vec2 ndiCoord);
void onInGameUiMouseCallback(UiStateContext& uiState, UiContext& uiContext, InGameUi& inGameUi, int button, int action, std::optional<objid> selectedId);
void onInGameUiMouseMoveCallback(InGameUi& inGameUi, double xPos, double yPos, float xNdc, float yNdc);
void onInGameUiScrollCallback(InGameUi& inGameUi, double amount);
void onInGameUiKeyCallback(int key, int scancode, int action, int mods);

void testInGameUiSetText(std::string value);

#endif 