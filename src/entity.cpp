#include "./entity.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;
extern Movement movement;

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
	modlog("controllable entity added id:", std::to_string(idAdded));
  bool shouldAddWeapon = false;
  shouldAddWeapon = shouldAddWeapon || maybeAddMovementEntity(movementEntities, idAdded);
  auto agent = getSingleAttr(idAdded, "agent");
  if (agent.has_value()){
  	shouldAddWeapon = true;
    addAiAgent(aiData, idAdded, agent.value());
    controllableEntities[idAdded] = ControllableEntity {
    	.isInShootingMode = true,
    };
  }

  if (shouldAddWeapon){
	  addInventory(idAdded);
	  addWeaponId(weapons, idAdded);
  }
}

void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved){
	if (controllableEntities.find(idRemoved) != controllableEntities.end()){
		modlog("controllable entity removed id:", std::to_string(idRemoved));
	}
  maybeRemoveMovementEntity(movement, movementEntities, idRemoved);
  maybeRemoveAiAgent(aiData, idRemoved);
  removeWeaponId(weapons, idRemoved);
  removeInventory(idRemoved);
  controllableEntities.erase(idRemoved);
}

bool controllableEntityExists(objid id){
	return controllableEntities.find(id) != controllableEntities.end();
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

std::optional<bool> activePlayerInThirdPerson(){
	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
	bool thirdPerson = getWeaponEntityData(controlledPlayer.playerId.value()).thirdPersonMode;
	return thirdPerson;
}

std::optional<bool> isInShootingMode(objid id){
	if (controllableEntities.find(id) == controllableEntities.end()){
		return false;
	}
	return controllableEntities.at(id).isInShootingMode;
}

void setInShootingMode(objid id, bool shootingMode){
	modassert(controllableEntities.find(id) != controllableEntities.end(), std::string("controllable entity setInShootingMode for unregistered controllableEntity: ") + std::to_string(id));
	controllableEntities.at(id).isInShootingMode = shootingMode;
}

void maybeReEnableMesh(objid id){
	modlog("disable mesh main", gameapi -> getGameObjNameForId(id).value());
	for (auto childId : gameapi -> getChildrenIdsAndParent(id)){
		 modlog("enable mesh", gameapi -> getGameObjNameForId(childId).value());
		 gameapi -> setSingleGameObjectAttr(childId, "disabled", "false");
	}
}
void maybeDisableMesh(objid id){
	modlog("disable mesh main", gameapi -> getGameObjNameForId(id).value());
	for (auto childId : gameapi -> getChildrenIdsAndParent(id)){
		 modlog("disable mesh", gameapi -> getGameObjNameForId(childId).value());
		 gameapi -> setSingleGameObjectAttr(childId, "disabled", "true");
	}
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
	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> removeByGroupId(controlledPlayer.activePlayerManagedCameraId.value());
		controlledPlayer.activePlayerManagedCameraId = std::nullopt;
	}

	if (controlledPlayer.playerId.has_value()){
		removeFromRace(controlledPlayer.playerId.value());
	}
	controlledPlayer.playerId = id.value();
  addToRace(id.value());

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
  if (controlledPlayer.playerId.has_value()){
    doDamageMessage(controlledPlayer.playerId.value(), 10000.f);   
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

FiringTransform getFireTransform(objid id){
	MovementEntity& movementEntity = getMovementData().movementEntities.at(id);

	if (movementEntity.managedCamera.thirdPersonMode){
		// this 0 out only works if these vectors are parallel, otherwise would have to solve the parametric eqtns 
		// z is set to the player entity so it doesnt shoot from behind the character.
		// for example, if you dont do this, if you zoom the cam out you can hit a target in the crosshair behind the character
		auto thirdPersonInfo = lookThirdPersonCalc(movementEntity.movementState, movementEntity.managedCamera);
		auto posFromThirdPerson = glm::inverse(thirdPersonInfo.rotation) * (thirdPersonInfo.position - gameapi -> getGameObjectPos(id, true));
		auto zOffset = glm::vec3(0.f, 0.f, posFromThirdPerson.z);
		auto zSpaceOffset = thirdPersonInfo.rotation * zOffset;
		return FiringTransform {
			.position = thirdPersonInfo.position - zSpaceOffset,
			.rotation = thirdPersonInfo.rotation,
		};
	}

	auto position = gameapi -> getGameObjectPos(id, true);
	auto rotation = getLookDirection(movementEntity);
	return FiringTransform {
		.position = position,
		.rotation = rotation,
	};
}

WeaponEntityData getWeaponEntityData(objid id){
	MovementEntity& movementEntity = getMovementData().movementEntities.at(id);
  return WeaponEntityData {
    .inventory = id,
    .lookVelocity = controlledPlayer.lookVelocity, // this should be in the movementState instead....not be based off the players...
    .velocity = movementEntity.movementState.velocity,
    .thirdPersonMode = movementEntity.managedCamera.thirdPersonMode,
    .fireTransform = getFireTransform(id),
  };
}

DebugConfig debugPrintActivePlayer(){
  DebugConfig debugConfig { .data = {} };

  {
  	std::string activeCameraName = "";
  	auto activeCameraId = gameapi -> getActiveCamera();
  	if (activeCameraId.has_value()){
  		activeCameraName = gameapi -> getGameObjNameForId(activeCameraId.value()).value();
  	}
	  debugConfig.data.push_back({"activeCameraName", print(activeCameraId) + " [" + activeCameraName + "]" });
	}

	{
	  std::string playerName = "";
	  if (controlledPlayer.playerId.has_value()){
	  	playerName = gameapi -> getGameObjNameForId(controlledPlayer.playerId.value()).value();
	  }
	  debugConfig.data.push_back({"activeplayer id", print(controlledPlayer.playerId) + + " [" + playerName + "]" });
	}

	{
  	std::string cameraName = "";
  	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
  		cameraName = gameapi -> getGameObjNameForId(controlledPlayer.activePlayerManagedCameraId.value()).value();
  	}
  	debugConfig.data.push_back({"activePlayerManagedCameraId id", print(controlledPlayer.activePlayerManagedCameraId) + + " [" +  cameraName + "]" });
	}

	auto inThirdPerson = activePlayerInThirdPerson();
	debugConfig.data.push_back({ "mode", (inThirdPerson.has_value() && inThirdPerson.value()) ? "third person" : "first person" });			

  return debugConfig;
}