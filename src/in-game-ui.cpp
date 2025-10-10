#include "./in-game-ui.h"

extern CustomApiBindings* gameapi;

bool showSelectionTexture = false;

void setShowSelectionTexture(bool shouldShow){
	showSelectionTexture = shouldShow;
}

std::string ingameUiTextureName(objid id){
	return "gentexture-ingame-ui-texture-test";
}
std::string selectionTextureName(std::string textureName){
	return textureName + "_selection_texture";

}

void createInGamesUiInstance(InGameUi& inGameUi, objid id){
	modassert(inGameUi.textDisplays.find(id) == inGameUi.textDisplays.end(), "id already exists");

	std::string texture = ingameUiTextureName(id);
	auto uiTexture = gameapi -> createTexture(texture, 1920, 1080, id);
 	setGameObjectTexture(id, showSelectionTexture ? selectionTextureName(texture) : texture);

 	inGameUi.textDisplays[id] = TextDisplay{
 		.textureId = uiTexture,
 		.handlerFns = {},
 		.mouseCoordNdc = glm::vec2(0.f, 0.f),
 		.routerHistory = createHistory(),
 		.uiStateContext = UiStateContext {
 			.uiState = createUiState(),
 		},
 	};
 	inGameUi.textDisplays.at(id).uiStateContext.value().routerHistory = &inGameUi.textDisplays.at(id).routerHistory.value();
 	// initial route
  pushHistory(inGameUi.textDisplays.at(id).routerHistory.value(), { "debug", "wheel" }, true);

};

void freeInGameUiInstance(InGameUi& inGameUi, objid id){
	gameapi -> freeTexture(ingameUiTextureName(id), id);
	inGameUi.textDisplays.erase(id);
}

std::optional<objid> getAnyUiInstance(InGameUi& inGameUi){
	if (inGameUi.textDisplays.size() == 0){
		return std::nullopt;
	}
	auto firstElement = inGameUi.textDisplays.begin() -> first; // will return the first set<int>
	return firstElement;
}


void onInGameUiFrame(UiStateContext& uiState, InGameUi& inGameUi, UiContext& uiContext, std::optional<objid> textureId, glm::vec2 ndiCoord){
	// should make sure the texture id is the same
	bool drawCursor = true;
	for (auto &[id, textDisplay] : inGameUi.textDisplays){
		textDisplay.mouseCoordNdc = ndiCoord;

		gameapi -> clearTexture(textDisplay.textureId, std::nullopt, std::nullopt, std::nullopt);
    UiStateContext& actualUiState = textDisplay.uiStateContext.has_value() ? textDisplay.uiStateContext.value() : uiState;
		textDisplay.handlerFns = handleDrawMainUi(
			actualUiState, 
			uiContext, 
			getGlobalState().selectedId, 
			textDisplay.textureId, 
			drawCursor ? textDisplay.mouseCoordNdc : std::optional<glm::vec2>(std::nullopt)
		);

		auto ndiCoords = uvToNdi(getGlobalState().texCoordUvView);
    gameapi -> idAtCoordAsync(ndiCoords.x, ndiCoords.y, false, textDisplay.textureId, [ndiCoords](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
			modlog("ui pick color on game ui id", std::to_string(selectedId.value()));
			if (selectedId.has_value()){
				modlog("ui pick color on game ui id", std::to_string(selectedId.value()));
				modlog("ui pick color on game ui uv", print(ndiCoords));
			}
    });
	}
}

void onInGameUiMouseClick(UiStateContext& uiState, UiContext& uiContext, InGameUi& inGameUi, objid id, int button, int action, glm::vec2 ndiCoords){
  gameapi -> idAtCoordAsync(ndiCoords.x, ndiCoords.y, false, inGameUi.textDisplays.at(id).textureId, [&uiState, id, ndiCoords, &inGameUi, &uiContext, button, action](std::optional<objid> uiId, glm::vec2 texCoordUv) -> void {
		if (inGameUi.textDisplays.find(id) == inGameUi.textDisplays.end()){
			return;
		}
		auto& handlerFns = inGameUi.textDisplays.at(id).handlerFns; // should probably check this still exists
		if (uiId.has_value()){
			TextDisplay& textDisplay = inGameUi.textDisplays.at(id);
	  	UiStateContext& actualUiState = textDisplay.uiStateContext.has_value() ? textDisplay.uiStateContext.value() : uiState;
			modlog("ui pick color on game ui id", std::to_string(uiId.value()));
			onMainUiMousePress(actualUiState, uiContext, handlerFns, button, action, uiId.value());
		}
  });
}

void onInGameUiMouseCallback(UiStateContext& uiState, UiContext& uiContext, InGameUi& inGameUi, int button, int action, std::optional<objid> selectedId){
	if (!selectedId.has_value()){
		return;
	}
	auto id = selectedId.value();
	if (inGameUi.textDisplays.find(id) == inGameUi.textDisplays.end()){
		return;
	}
	auto ndiCoords = uvToNdi(getGlobalState().texCoordUvView);

	TextDisplay& textDisplay = inGameUi.textDisplays.at(id);
  UiStateContext& actualUiState = textDisplay.uiStateContext.has_value() ? textDisplay.uiStateContext.value() : uiState;
	onInGameUiMouseClick(actualUiState, uiContext, inGameUi, id, button, action, ndiCoords);
}

void onInGameUiMouseMoveCallback(InGameUi& inGameUi, double xPos, double yPos, float xNdc, float yNdc){
	for (auto &[id, textDisplay] : inGameUi.textDisplays){
		//textDisplay.mouseCoordNdc.x += (xPos / 100.f);
		//textDisplay.mouseCoordNdc.y += (yPos / 100.f);
		modlog("onInGameUiMouseMoveCallback", print(textDisplay.mouseCoordNdc));
	}
}

void onInGameUiScrollCallback(InGameUi& inGameUi, double amount){

}

void onInGameUiKeyCallback(int key, int scancode, int action, int mods){

}
