#include "./in-game-ui.h"

extern CustomApiBindings* gameapi;

std::string ingameUiTextureName(objid id){
	return std::string("gentexture-ingame-ui-texture-test");
}
void createInGamesUiInstance(InGameUi& inGameUi, objid id){
	modassert(inGameUi.textDisplays.find(id) == inGameUi.textDisplays.end(), "id already exists");

	std::string texture = ingameUiTextureName(id);
	auto uiTexture = gameapi -> createTexture(texture, 1920, 1080, id);
 	setGameObjectTexture(id, texture);

 	inGameUi.textDisplays[id] = TextDisplay{
 		.texture = "../gameresources/textures/controls/up-down.png",
 		.channel = "ui-debug-text",
 		.text = "hello world",
 		.textPosition = glm::vec2(0.f, 0.f),
 		.textureId = uiTexture,
 		.needsRefresh = true,
 	};
};

void freeInGameUiInstance(InGameUi& inGameUi, objid id){
	gameapi -> freeTexture(ingameUiTextureName(id), id);
	inGameUi.textDisplays.erase(id);
}

void onInGameUiFrame(InGameUi& inGameUi, UiContext& uiContext){
	for (auto &[id, textDisplay] : inGameUi.textDisplays){
		gameapi -> clearTexture(textDisplay.textureId, std::nullopt, std::nullopt, std::nullopt);
		handleDrawMainUi(uiContext, getGlobalState().selectedId, textDisplay.textureId);
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


void onInGameUiMessage(InGameUi& inGameUi, std::string& key, std::any& value){
	for (auto &[_, textDisplay] : inGameUi.textDisplays){
		if (key == textDisplay.channel){
      std::string* channelValue = anycast<std::string>(value);
      modassert(channelValue, "invalid type for channelValue");
      textDisplay.text = *channelValue;
      textDisplay.needsRefresh = true;
		}
	}
}

void testInGameUiSetText(std::string value){
	gameapi -> sendNotifyMessage("ui-debug-text", std::string("textvalue"));
}