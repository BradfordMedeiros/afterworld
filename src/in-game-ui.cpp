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
 	};
};

void freeInGameUiInstance(InGameUi& inGameUi, objid id){
	gameapi -> freeTexture(ingameUiTextureName(id), id);
	inGameUi.textDisplays.erase(id);
}

void onInGameUiFrame(InGameUi& inGameUi, UiContext& uiContext, std::optional<objid> textureId, glm::vec2 ndiCoord){
	// should make sure the texture id is the same
	for (auto &[id, textDisplay] : inGameUi.textDisplays){
		gameapi -> clearTexture(textDisplay.textureId, std::nullopt, std::nullopt, std::nullopt);
		textDisplay.handlerFns = handleDrawMainUi(uiContext, getGlobalState().selectedId, textDisplay.textureId, ndiCoord);

		auto ndiCoords = uvToNdi(getGlobalState().texCoordUvView);
    gameapi -> idAtCoordAsync(ndiCoords.x, ndiCoords.y, false, textDisplay.textureId, [ndiCoords](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
			modlog("on game ui id", std::to_string(selectedId.value()));
			modlog("on game ui uv", print(ndiCoords));
    });

	}
}



void zoomIntoGameUi(objid id){
	auto rotation = gameapi -> getGameObjectRotation(id, true);
	auto objectPosition = gameapi -> getGameObjectPos(id, true);
  auto uiOffset = getSingleVec3Attr(id, "in-game-ui-offset");
  auto offset = uiOffset.has_value() ? uiOffset.value() : glm::vec3(0.f, 0.f, 0.f);
	auto position = objectPosition + rotation * offset;
	auto viewOrientation = gameapi -> orientationFromPos(position, objectPosition);
	setTempViewpoint(position, viewOrientation);
}

std::optional<objid> getAnyUiInstance(InGameUi& inGameUi){
	if (inGameUi.textDisplays.size() == 0){
		return std::nullopt;
	}
	auto firstElement = inGameUi.textDisplays.begin() -> first; // will return the first set<int>
	return firstElement;
}


void onInGameUiSelect(InGameUi& inGameUi, int button, int action, std::optional<objid> selectedId){
	if (!selectedId.has_value()){
		return;
	}

	modlog("on game ui onInGameUiSelect", print(selectedId));

	auto id = selectedId.value();
	if (inGameUi.textDisplays.find(id) == inGameUi.textDisplays.end()){
		return;
	}
	auto& handlerFns = inGameUi.textDisplays.at(id).handlerFns;
  onMainUiMousePress(handlerFns, button, action, selectedId);
}

void testInGameUiSetText(std::string value){
	gameapi -> sendNotifyMessage("ui-debug-text", std::string("textvalue"));
}