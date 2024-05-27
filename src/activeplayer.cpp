#include "./activeplayer.h"

extern CustomApiBindings* gameapi;


struct ControlledPlayer {
	
};


std::optional<objid> activePlayerId = std::nullopt;
bool activePlayerTempDisabled = false;

std::optional<objid> tempCameraId = std::nullopt;
std::optional<objid> tempViewpoint = std::nullopt;

std::optional<objid> getActivePlayerId(){
	if (activePlayerTempDisabled){
		return std::nullopt;
	}
	return activePlayerId;
}

std::optional<objid> setCameraOrMakeTemp(objid id){
	if (tempCameraId.has_value()){
		gameapi -> removeByGroupId(tempCameraId.value());
	}
	auto name = gameapi -> getGameObjNameForId(id).value();
	auto isCamera = name.at(0) == '>';
	if (isCamera){
		gameapi -> setActiveCamera(id, -1);
	}else{
    GameobjAttributes attr {
      .attr = { 
				{ "position", glm::vec3(0.f, 0.f, 20.f) },
      },
    };
    std::string cameraName = std::string(">player-camera-") + uniqueNameSuffix();
    std::map<std::string, GameobjAttributes> submodelAttributes;
    auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id), cameraName, attr, submodelAttributes).value();
    tempCameraId = cameraId;
    gameapi -> makeParent(cameraId, id);
    gameapi -> setActiveCamera(cameraId, -1);
    return cameraId;
	}
	return std::nullopt;
}
void setActivePlayer(std::optional<objid> id){
	if (!id.has_value()){
		return;
	}
	if (activePlayerId.has_value()){
	  gameapi -> sendNotifyMessage("ai-activate", activePlayerId.value());
	}
	auto newCameraId = setCameraOrMakeTemp(id.value());
  gameapi -> sendNotifyMessage("ai-deactivate", id.value());
	activePlayerId = id.value();
	setActiveEntity(id.value(), newCameraId);
  gameapi -> sendNotifyMessage("active-player-change", id.value());
}

void setActivePlayerNext(){
	setActivePlayer(getNextEntity());
}

void drawCenteredText(std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId){
  float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi
  float approximateWidth = text.size() * ndiSize;
  gameapi -> drawText(text, ndiOffsetX - (approximateWidth * 0.5f), ndiOffsetY, fontSizeNdiEquivalent, false, tint, std::nullopt, true, std::nullopt, selectionId);
}

bool displayGameOver = false;
void onPlayerFrame(){
	if (displayGameOver){
	  gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.1f, 0.1f, 0.1f, 1.f), std::nullopt, true, std::nullopt, "./res/textures/testgradient.png");
		drawCenteredText("GAME OVER", 0.f, 0.f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
	}
	//modlog("active player", print(activePlayerId));
}
void onActivePlayerRemoved(objid id){
	if (activePlayerId.has_value() && activePlayerId.value() == id){
		activePlayerId = std::nullopt;

		auto playerScene = gameapi -> listSceneId(id);
		auto playerPos = gameapi -> getGameObjectPos(id, true);
	
		// check if scene exists
	
		modlog("active player, create dead camera at: ", print(playerPos));
		auto createdObjId = id;

		displayGameOver = true;
  	gameapi -> schedule(createdObjId, 5000, NULL, [](void*) -> void {
  	 	gameapi -> sendNotifyMessage("game-over", (int)1);
  		displayGameOver = false;
  	});
	}
}

void setTempViewpoint(glm::vec3 position, glm::quat rotation){
	modassert(!tempViewpoint.has_value(), "already have a temp viewpoint");
	activePlayerTempDisabled = true;
	auto id = activePlayerId.value();
  std::string cameraName = std::string(">tempviewpoint-camera-") + uniqueNameSuffix();
  GameobjAttributes attr {
    .attr = { { "position", position } },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id), cameraName, attr, submodelAttributes).value();
  gameapi -> setActiveCamera(cameraId, -1);
  gameapi -> setGameObjectRot(cameraId, rotation, true);
  tempViewpoint = cameraId;
}
bool hasTempViewpoint(){
	return tempViewpoint.has_value();
}
void popTempViewpoint(){
	activePlayerTempDisabled = false;
	gameapi -> removeByGroupId(tempViewpoint.value());
	tempViewpoint = std::nullopt;

	if (tempCameraId.has_value()){
		gameapi -> setActiveCamera(tempCameraId.value(), -1);
	}else{
		gameapi -> setActiveCamera(activePlayerId.value(), -1);
	}
}

