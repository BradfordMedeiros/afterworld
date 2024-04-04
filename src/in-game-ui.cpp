#include "./in-game-ui.h"

extern CustomApiBindings* gameapi;

std::string ingameUiTextureName(objid id){
	return std::string("gentexture-ingame-ui-texture-test");
}
void createInGamesUiInstance(InGameUi& inGameUi, objid id){
	std::cout << "create in game texture" << std::endl;
	modassert(inGameUi.ids.find(id) == inGameUi.ids.end(), "id already added");
	inGameUi.ids.insert(id);
	std::string texture = ingameUiTextureName(id);
	auto uiTexture = gameapi -> createTexture(texture, 512, 512, id);
	gameapi -> clearTexture(uiTexture, true, std::nullopt, "../gameresources/textures/controls/up-down.png");

 	GameobjAttributes attr {
 	   .stringAttributes = {
 	     { "texture", texture },
 	   },
 	   .numAttributes = {},
 	   .vecAttr = {
 	     .vec3 = {},
 	     .vec4 = {},
 	   },
 	};
 	gameapi -> setGameObjectAttr(id, attr);  

 	inGameUi.uiInstance =  TestUiInstance {
		.cursorLocation = glm::vec2(0.f, 0.f),
		.textureId = uiTexture,
		.upSelected = true,
	};
};

void freeInGameUiInstance(InGameUi& inGameUi, objid id){
	inGameUi.ids.erase(id);
	gameapi -> freeTexture(ingameUiTextureName(id), id);
}

void onInGameUiFrame(InGameUi& inGameUi){
	if (!inGameUi.uiInstance.has_value()){
		return;
	}
	inGameUi.uiInstance.value().cursorLocation = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
	bool upSelected = inGameUi.uiInstance.value().upSelected;
  gameapi -> drawRect(0.f, 0.f, 1.f, 1.f, false, upSelected ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f), inGameUi.uiInstance.value().textureId, true, std::nullopt, std::nullopt);
	gameapi -> drawText("hello", inGameUi.uiInstance.value().cursorLocation.x, inGameUi.uiInstance.value().cursorLocation.y, 20, false, std::nullopt /*tint */, inGameUi.uiInstance.value().textureId, true, std::nullopt, std::nullopt);
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
	if (inGameUi.ids.size() == 0){
		return std::nullopt;
	}
	auto firstElement = *(inGameUi.ids.begin()); // will return the first set<int>
	return firstElement;
}


void onInGameUiMessage(InGameUi& inGameUi, std::string& message){
	if (inGameUi.uiInstance.has_value() && message == "ui-next"){
		inGameUi.uiInstance.value().upSelected = !inGameUi.uiInstance.value().upSelected;
	}
}