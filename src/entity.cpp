#include "./entity.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;

MovementEntityData& getMovementData();

std::unordered_map<objid, ControllableEntity> controllableEntities;

ControlledPlayer controlledPlayer {
	.lookVelocity = glm::vec2(0.f, 0.f),  // should come from movement state
	.playerId = std::nullopt,
	.activePlayerManagedCameraId = std::nullopt, // this is fixed camera for fps mode
	.editorMode = false,
};

std::optional<objid> getPlayerId(){
	return controlledPlayer.playerId;
}
void onAddControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idAdded){
	modlog("onAddControllableEntity added id:", std::to_string(idAdded));
  bool shouldAddWeapon = false;
  shouldAddWeapon = shouldAddWeapon || maybeAddMovementEntity(movementEntities, idAdded);
  auto agent = getSingleAttr(idAdded, "agent");
  if (agent.has_value()){
  	shouldAddWeapon = true;
    addAiAgent(aiData, idAdded, agent.value());
    controllableEntities[idAdded] = ControllableEntity {};
  }

  if (shouldAddWeapon){
	  addInventory(idAdded);
	  addWeaponId(weapons, idAdded);
  }
}

void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved){
  maybeRemoveMovementEntity(movementEntities, idRemoved);
  maybeRemoveAiAgent(aiData, idRemoved);
  removeWeaponId(weapons, idRemoved);
  removeInventory(idRemoved);
  controllableEntities.erase(idRemoved);
}

void updateCamera(){
	if (controlledPlayer.editorMode){
		gameapi -> setActiveCamera(std::nullopt, -1);
		return;
	}
	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> setActiveCamera(controlledPlayer.activePlayerManagedCameraId.value(), -1);
	}
}

std::optional<objid> getCameraForThirdPerson(){
	return controlledPlayer.activePlayerManagedCameraId;
}

void setActivePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id){
	if (!id.has_value()){
		return;
	}
	if (controlledPlayer.playerId.has_value()){
    maybeReEnableAi(aiData, controlledPlayer.playerId.value());
	}
	setActiveMovementEntity(movement);
	maybeDisableAi(aiData, id.value());

	controlledPlayer.playerId = id.value();

	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> removeByGroupId(controlledPlayer.activePlayerManagedCameraId.value());
		controlledPlayer.activePlayerManagedCameraId = std::nullopt;
	}
  GameobjAttributes attr { .attr = {{ "position", glm::vec3(0.f, 0.f, 0.f) }} };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id.value()), std::string(">gen-player-camera-") + uniqueNameSuffix(), attr, submodelAttributes).value();
  gameapi -> makeParent(cameraId, id.value());
  controlledPlayer.activePlayerManagedCameraId = cameraId;

  updateCamera();
}
void setActivePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData){
  setActivePlayer(movement, weapons, aiData, getNextEntity(getMovementData(), controlledPlayer.playerId));
}

std::optional<objid> getActivePlayerId(){
	return controlledPlayer.playerId;
}

bool onActivePlayerRemoved(objid id){
	if (controlledPlayer.playerId.has_value() && controlledPlayer.playerId.value() == id){
		controlledPlayer.playerId = std::nullopt;
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
	auto playerId = getActivePlayerId();
  if (playerId.has_value()){
    doDamageMessage(playerId.value(), 10000.f);   
  }
}

glm::vec3 getPlayerVelocity(){
  return getMovementData().movementEntities.at(controlledPlayer.playerId.value()).movementState.velocity;
}

std::optional<glm::vec3> getActivePlayerPosition(){
	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
	return gameapi -> getGameObjectPos(controlledPlayer.playerId.value(), true);
}

WeaponEntityData getWeaponEntityData(objid id){
	MovementState& movementState = getMovementData().movementEntities.at(id).movementState;
  return WeaponEntityData {
    .inventory = id,
    .lookVelocity = controlledPlayer.lookVelocity, // this should be in the movementState instead....not be based off the players...
    .velocity = movementState.velocity,
  };
}

DebugConfig debugPrintActivePlayer(){
  DebugConfig debugConfig { .data = {} };
  debugConfig.data.push_back({"activeplayer id", print(controlledPlayer.playerId) });
  debugConfig.data.push_back({"tempCameraId id", print(controlledPlayer.playerId) });
  return debugConfig;
}