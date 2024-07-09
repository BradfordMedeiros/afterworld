#include "./activeplayer.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;
extern Movement movement;
extern AiData aiData;

void displayGameOverMenu();

struct ControlledPlayer {
	std::optional<objid> activePlayerId;
	std::optional<objid> activePlayerManagedCameraId;
	std::optional<objid> tempViewpoint;
	bool editorMode;
};

ControlledPlayer controlledPlayer {
	.activePlayerId = std::nullopt,
	.activePlayerManagedCameraId = std::nullopt,
	.tempViewpoint = std::nullopt,
	.editorMode = false,
};

void updateCamera(){
	if (controlledPlayer.editorMode){
		gameapi -> setActiveCamera(std::nullopt, -1);
		return;
	}
	if (controlledPlayer.tempViewpoint.has_value()){
		gameapi -> setActiveCamera(controlledPlayer.tempViewpoint.value(), -1);
	}else if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> setActiveCamera(controlledPlayer.activePlayerManagedCameraId.value(), -1);
	}else if (controlledPlayer.activePlayerId.has_value()){
		auto name = gameapi -> getGameObjNameForId(controlledPlayer.activePlayerId.value()).value();
		auto isCamera = name.at(0) == '>';
		modassert(isCamera, "not a camera, but not managed cameras");
		gameapi -> setActiveCamera(controlledPlayer.activePlayerId.value(), -1);
	}
}

std::optional<objid> getActivePlayerId(){
	if (controlledPlayer.tempViewpoint.has_value()){
		return std::nullopt;
	}
	return controlledPlayer.activePlayerId;
}

std::optional<objid> setCameraOrMakeTemp(objid id){
	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> removeByGroupId(controlledPlayer.activePlayerManagedCameraId.value());
		controlledPlayer.activePlayerManagedCameraId = std::nullopt;
	}
	auto name = gameapi -> getGameObjNameForId(id).value();
	auto isCamera = name.at(0) == '>';
	if (isCamera){
		updateCamera();
	}else{
    GameobjAttributes attr {
      .attr = { 
				{ "position", glm::vec3(0.f, 0.f, 20.f) },
      },
    };
    std::string cameraName = std::string(">player-camera-") + uniqueNameSuffix();
    std::map<std::string, GameobjAttributes> submodelAttributes;
    auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id), cameraName, attr, submodelAttributes).value();
    controlledPlayer.activePlayerManagedCameraId = cameraId;
    gameapi -> makeParent(cameraId, id);
    updateCamera();
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
	controlledPlayer.activePlayerId = id.value();
	auto newCameraId = setCameraOrMakeTemp(id.value());
	setActiveMovementEntity(movement, getMovementData(), id.value(), newCameraId);
	changeWeaponTargetId(weapons, id.value(), "another");
	maybeDisableAi(aiData, id.value());
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
		controlledPlayer.activePlayerManagedCameraId = std::nullopt; // probably should delete this too
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
	auto id = controlledPlayer.activePlayerId.value();
  std::string cameraName = std::string(">tempviewpoint-camera-") + uniqueNameSuffix();
  GameobjAttributes attr {
    .attr = { { "position", position } },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id), cameraName, attr, submodelAttributes).value();
  gameapi -> setGameObjectRot(cameraId, rotation, true);
  controlledPlayer.tempViewpoint = cameraId;
  updateCamera();
}
bool hasTempViewpoint(){
	return controlledPlayer.tempViewpoint.has_value();
}
void popTempViewpoint(){
	gameapi -> removeByGroupId(controlledPlayer.tempViewpoint.value());
	controlledPlayer.tempViewpoint = std::nullopt;
	updateCamera();
}

void setActivePlayerEditorMode(bool editorMode){
	controlledPlayer.editorMode = editorMode;
	updateCamera();
}

std::vector<std::vector<std::string>> debugPrintActivePlayer(){
	//ControlledPlayer controlledPlayer {
	//	.activePlayerId = std::nullopt,
	//	.activePlayerTempDisabled = false,
	//	.tempCameraId = std::nullopt,
	//	.tempViewpoint = std::nullopt,
	//};

	return { 
		{"activeplayer id", print(controlledPlayer.activePlayerId) },
		{"tempCameraId id", print(controlledPlayer.activePlayerId) },
		{"tempViewpoint", print(controlledPlayer.tempViewpoint) },

	//	. = std::nullopt,

	};
}