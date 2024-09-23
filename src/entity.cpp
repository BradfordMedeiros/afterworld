#include "./entity.h"

extern CustomApiBindings* gameapi;
MovementEntityData& getMovementData();

std::unordered_map<objid, ControllableEntity> controllableEntities;

ControlledPlayer controlledPlayer {
  .playerVelocity = glm::vec3(0.f, 0.f, 0.f),
	.lookVelocity = glm::vec2(0.f, 0.f),
	.playerId = std::nullopt,
	.activeEntity = std::nullopt,
	.activePlayerId = std::nullopt,
	.activePlayerManagedCameraId = std::nullopt,
	.tempViewpoint = std::nullopt,
	.editorMode = false,
};

std::optional<objid> getPlayerId(){
	return controlledPlayer.playerId;
}
void onAddControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idAdded){
  maybeAddMovementEntity(movementEntities, idAdded);

  auto agent = getSingleAttr(idAdded, "agent");
  if (agent.has_value()){
    addAiAgent(aiData, idAdded, agent.value());
    controllableEntities[idAdded] = ControllableEntity {
      .gunCore = createGunCoreInstance("pistol", 5, gameapi -> listSceneId(idAdded)),
    };
  }
}

void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved){
  maybeRemoveMovementEntity(movementEntities, idRemoved);
  maybeRemoveAiAgent(aiData, idRemoved);
  controllableEntities.erase(idRemoved);
}

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

void setActivePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id){
	if (!id.has_value()){
		return;
	}
	if (controlledPlayer.activePlayerId.has_value()){
    maybeReEnableAi(aiData, controlledPlayer.activePlayerId.value());
	}
	controlledPlayer.activePlayerId = id.value();
	auto newCameraId = setCameraOrMakeTemp(id.value());

  controlledPlayer.activeEntity = id;
	setActiveMovementEntity(movement, getMovementData(), id.value(), newCameraId);

	controlledPlayer.playerId = id.value();

	maybeDisableAi(aiData, id.value());
}
void setActivePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData){
  setActivePlayer(movement, weapons, aiData, getNextEntity(getMovementData(), controlledPlayer.activeEntity));
}

std::optional<objid> getActivePlayerId(){
	if (controlledPlayer.tempViewpoint.has_value()){
		return std::nullopt;
	}
	return controlledPlayer.activePlayerId;
}

bool onActivePlayerRemoved(objid id){
	if (controlledPlayer.activePlayerId.has_value() && controlledPlayer.activePlayerId.value() == id){
		controlledPlayer.activePlayerId = std::nullopt;
		controlledPlayer.activePlayerManagedCameraId = std::nullopt; // probably should delete this too
		return true;
	}
	return false;
}


void setActivePlayerEditorMode(bool editorMode){
	controlledPlayer.editorMode = editorMode;
	updateCamera();
}

void killActivePlayer(){
	auto activePlayerId = getActivePlayerId();
  if (activePlayerId.has_value()){
    doDamageMessage(activePlayerId.value(), 10000.f);   
  }
}

void setPlayerVelocity(glm::vec3 velocity){
  controlledPlayer.playerVelocity = velocity;
}

glm::vec3 getPlayerVelocity(){
  return controlledPlayer.playerVelocity;
}

std::string defaultInventory = "default";
std::string& activePlayerInventory(){
	return defaultInventory;
}

DebugConfig debugPrintActivePlayer(){
  DebugConfig debugConfig { .data = {} };
  debugConfig.data.push_back({"activeplayer id", print(controlledPlayer.activePlayerId) });
  debugConfig.data.push_back({"tempCameraId id", print(controlledPlayer.activePlayerId) });
  debugConfig.data.push_back({"tempViewpoint", print(controlledPlayer.tempViewpoint) });
  return debugConfig;
}