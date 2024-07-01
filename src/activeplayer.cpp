#include "./activeplayer.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;
extern Movement movement;
extern AiData aiData;

void goBackMainMenu();
void displayGameOverMenu();

struct ControlledPlayer {
	std::optional<objid> activePlayerId;
	bool activePlayerTempDisabled;
	std::optional<objid> tempCameraId;
	std::optional<objid> tempViewpoint;

};

ControlledPlayer controlledPlayer {
	.activePlayerId = std::nullopt,
	.activePlayerTempDisabled = false,
	.tempCameraId = std::nullopt,
	.tempViewpoint = std::nullopt,
};


std::optional<objid> getActivePlayerId(){
	if (controlledPlayer.activePlayerTempDisabled){
		return std::nullopt;
	}
	return controlledPlayer.activePlayerId;
}

std::optional<objid> setCameraOrMakeTemp(objid id){
	if (controlledPlayer.tempCameraId.has_value()){
		gameapi -> removeByGroupId(controlledPlayer.tempCameraId.value());
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
    controlledPlayer.tempCameraId = cameraId;
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
	if (controlledPlayer.activePlayerId.has_value()){
    maybeReEnableAi(aiData, controlledPlayer.activePlayerId.value());
	}
	auto newCameraId = setCameraOrMakeTemp(id.value());
	maybeDisableAi(aiData, id.value());
	controlledPlayer.activePlayerId = id.value();
	setActiveMovementEntity(movement, getMovementData(), id.value(), newCameraId);
	changeWeaponTargetId(weapons, id.value(), "another");
}

void setActivePlayerNext(){
	setActivePlayer(getNextEntity(getMovementData()));
}

void onPlayerFrame(){
	//modlog("active player", print(activePlayerId));

}
void onActivePlayerRemoved(objid id){
	if (controlledPlayer.activePlayerId.has_value() && controlledPlayer.activePlayerId.value() == id){
		controlledPlayer.activePlayerId = std::nullopt;
		auto playerScene = gameapi -> listSceneId(id);
		auto playerPos = gameapi -> getGameObjectPos(id, true);
		auto createdObjId = id;
		displayGameOverMenu();
	}
}

void setTempViewpoint(glm::vec3 position, glm::quat rotation){
	if (controlledPlayer.tempViewpoint.has_value()){
		popTempViewpoint();
	}
	controlledPlayer.activePlayerTempDisabled = true;
	auto id = controlledPlayer.activePlayerId.value();
  std::string cameraName = std::string(">tempviewpoint-camera-") + uniqueNameSuffix();
  GameobjAttributes attr {
    .attr = { { "position", position } },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id), cameraName, attr, submodelAttributes).value();
  gameapi -> setActiveCamera(cameraId, -1);
  gameapi -> setGameObjectRot(cameraId, rotation, true);
  controlledPlayer.tempViewpoint = cameraId;
}
bool hasTempViewpoint(){
	return controlledPlayer.tempViewpoint.has_value();
}
void popTempViewpoint(){
	controlledPlayer.activePlayerTempDisabled = false;
	gameapi -> removeByGroupId(controlledPlayer.tempViewpoint.value());
	controlledPlayer.tempViewpoint = std::nullopt;

	if (controlledPlayer.tempCameraId.has_value()){
		gameapi -> setActiveCamera(controlledPlayer.tempCameraId.value(), -1);
	}else{
		gameapi -> setActiveCamera(controlledPlayer.activePlayerId.value(), -1);
	}
}

