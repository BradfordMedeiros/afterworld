#include "./in-game-ui.h"

extern CustomApiBindings* gameapi;

std::string ingameUiTextureName(objid id){
	return std::string("gentexture-ingame-ui-texture-test");
}
void createInGamesUiInstance(InGameUi& inGameUi, objid id){
	modassert(inGameUi.textDisplays.find(id) == inGameUi.textDisplays.end(), "id already exists");

	std::string texture = ingameUiTextureName(id);
	auto uiTexture = gameapi -> createTexture(texture, 512, 512, id);
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

void onInGameUiFrame(InGameUi& inGameUi){
	for (auto &[id, textDisplay] : inGameUi.textDisplays){
		if (true || textDisplay.needsRefresh){
			float width = 0.f;
			float height = 0.f;
			gameapi -> getTextDimensionsNdi(textDisplay.text, 40 / 500.f, true, std::nullopt, &width, &height);
			std::cout << "text: " << width << ", " << height << std::endl;

			gameapi -> clearTexture(textDisplay.textureId, std::nullopt, std::nullopt, "../gameresources/textures/controls/up-down.png");
			gameapi -> drawText(textDisplay.text, textDisplay.textPosition.x - (width * 0.5f), textDisplay.textPosition.y, 40, false, std::nullopt /*tint */, textDisplay.textureId, true, std::nullopt, std::nullopt);
			textDisplay.needsRefresh = false;

			auto uvCoord = getGlobalState().texCoordUvView;
  		gameapi -> drawRect(uvCoord.x * 2 - 1, uvCoord.y * 2 - 1, 0.1f, 0.1f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textDisplay.textureId, true, std::nullopt, "./res/textures/crosshairs/crosshair008.png");
		}
	}



	//inGameUi.uiInstance.value().cursorLocation = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
	//bool upSelected = inGameUi.uiInstance.value().upSelected;
  //gameapi -> drawRect(0.f, 0.f, 1.f, 1.f, false, upSelected ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f), inGameUi.uiInstance.value().textureId, true, std::nullopt, std::nullopt);
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