#include "./entity.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;
extern Movement movement;

MovementEntityData& getMovementData();

extern std::unordered_map<objid, ControllableEntity> controllableEntities;  // static-state extern
extern std::unordered_map<objid, Inventory> scopenameToInventory;     // static-state extern

ControlledPlayer controlledPlayer {      // static-state
	.lookVelocity = glm::vec2(0.f, 0.f),  // should come from movement state
	.playerId = std::nullopt,
	.activePlayerManagedCameraId = std::nullopt, // this is fixed camera for fps mode
	.tempCamera = std::nullopt,
	.editorMode = false,
	.disablePlayerControl = false,
	.disableAnimationIds = {},
};

std::optional<objid> getPlayerId(){
	return controlledPlayer.playerId;
}

std::optional<objid> findBodyPart(objid entityId, const char* part){
  auto children = gameapi -> getChildrenIdsAndParent(entityId);
  for (auto childId : children){
    auto name = gameapi -> getGameObjNameForId(childId).value();
    if (stringEndsWith(name, part)){
      return childId;
    }
  } 
  return std::nullopt;
}
void disableEntityAnimations(objid entityId){
  auto leftHand = findBodyPart(entityId, "LeftHand");
  auto rightHand = findBodyPart(entityId, "RightHand");
  auto neck = findBodyPart(entityId, "Neck");
  auto head = findBodyPart(entityId, "Head");
  if (leftHand.has_value()){
	  controlledPlayer.disableAnimationIds.insert(leftHand.value());
  }
  if (rightHand.has_value()){
	  controlledPlayer.disableAnimationIds.insert(rightHand.value());
  }
  if (neck.has_value()){
	  controlledPlayer.disableAnimationIds.insert(neck.value());
  }
  if (head.has_value()){
	  controlledPlayer.disableAnimationIds.insert(head.value());
  }
  gameapi -> disableAnimationIds(controlledPlayer.disableAnimationIds); // maybe should just do add / remove 
}
void reenableEntityAnimations(objid entityId){
	controlledPlayer.disableAnimationIds.erase(entityId);
  gameapi -> disableAnimationIds(controlledPlayer.disableAnimationIds); // maybe should just do add / remove 
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
	  addInventory(scopenameToInventory, idAdded);
	  addWeaponId(weapons, idAdded);
  }

  disableEntityAnimations(idAdded);
}

void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved){
	reenableEntityAnimations(idRemoved);
	if (controllableEntities.find(idRemoved) != controllableEntities.end()){
		modlog("controllable entity removed id:", std::to_string(idRemoved));
	}
  maybeRemoveMovementEntity(movement, movementEntities, idRemoved);
  maybeRemoveAiAgent(aiData, idRemoved);
  removeWeaponId(weapons, idRemoved);
  removeInventory(scopenameToInventory, idRemoved);
  controllableEntities.erase(idRemoved);
}

bool controllableEntityExists(objid id){
	return controllableEntities.find(id) != controllableEntities.end();
}

void updateCamera(){
	bool tempCameraDoesNotExistButShould = controlledPlayer.tempCamera.has_value() && !gameapi -> gameobjExists(controlledPlayer.tempCamera.value());
	bool activeCameraDoesNotExistButShould = controlledPlayer.activePlayerManagedCameraId.has_value() && !gameapi -> gameobjExists(controlledPlayer.activePlayerManagedCameraId.value());
	modassert(!tempCameraDoesNotExistButShould, "temp camera has value but obj does not exist");
	modassert(!activeCameraDoesNotExistButShould, "active camera has value but obj does not exist");

	if (controlledPlayer.editorMode){
		gameapi -> setActiveCamera(std::nullopt);
		return;
	}
	if (controlledPlayer.tempCamera.has_value()){
		gameapi -> setActiveCamera(controlledPlayer.tempCamera.value());
		return;
	}
	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> setActiveCamera(controlledPlayer.activePlayerManagedCameraId.value());
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


bool isCamera(objid id){
	auto name = gameapi -> getGameObjNameForId(id).value();
	return name.at(0) == '>';
}
bool hasCameraAncestor(objid id){
	auto parent = gameapi -> getParent(id);
	if (!parent.has_value()){
		return false;
	}
	if (isCamera(parent.value())){
		return true;
	}
	return hasCameraAncestor(parent.value());
}

void maybeDisableMesh(objid id){
	modlog("disable mesh main", gameapi -> getGameObjNameForId(id).value());
	for (auto childId : gameapi -> getChildrenIdsAndParent(id)){
		 modlog("disable mesh", gameapi -> getGameObjNameForId(childId).value());
		 if (!hasCameraAncestor(childId)){ // guns are children of the camera 
 			 gameapi -> setSingleGameObjectAttr(childId, "disabled", "true");
		 }
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
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
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
	if (controlledPlayer.tempCamera.has_value() && controlledPlayer.tempCamera.value() == id){
		controlledPlayer.tempCamera = std::nullopt;
		updateCamera();
	}
	if (controlledPlayer.playerId.has_value() && controlledPlayer.playerId.value() == id){
		controlledPlayer.playerId = std::nullopt;
		controlledPlayer.activePlayerManagedCameraId = std::nullopt; // probably should delete this too
		controlledPlayer.disablePlayerControl = false;
		updateCamera();
		return true;
	}
	return false;
}

void setDisablePlayerControl(bool isDisabled){
	controlledPlayer.disablePlayerControl = isDisabled;
	updateCamera();
}
bool isPlayerControlDisabled(){
	return controlledPlayer.disablePlayerControl;
}

void setActivePlayerEditorMode(bool editorMode){
	controlledPlayer.editorMode = editorMode;
	updateCamera();
}
bool isInGameMode(){
	return !controlledPlayer.editorMode;
}

void setTempCamera(std::optional<objid> camera){
	controlledPlayer.tempCamera = camera;
	updateCamera();
}
std::optional<objid> getTempCamera(){
	return controlledPlayer.tempCamera;
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
	return gameapi -> getGameObjectPos(controlledPlayer.playerId.value(), true, "[gamelogic] getActivePlayerPosition");
}

std::optional<glm::quat> getActivePlayerRotation(){
	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
	return gameapi -> getGameObjectRotation(controlledPlayer.playerId.value(), true); // tempchecked
}

FiringTransform getFireTransform(objid id){
	MovementEntity& movementEntity = getMovementData().movementEntities.at(id);

	if (movementEntity.managedCamera.thirdPersonMode && (controlledPlayer.playerId.has_value() && controlledPlayer.playerId.value() == id)){ // only use the third person code for the active player
		// this 0 out only works if these vectors are parallel, otherwise would have to solve the parametric eqtns 
		// z is set to the player entity so it doesnt shoot from behind the character.
		// for example, if you dont do this, if you zoom the cam out you can hit a target in the crosshair behind the character
		auto thirdPersonInfo = lookThirdPersonCalc(movementEntity.movementState, movementEntity.managedCamera, id);
		auto posFromThirdPerson = glm::inverse(thirdPersonInfo.rotation) * (thirdPersonInfo.position - gameapi -> getGameObjectPos(id, true, "[gamelogic] getFireTransform 1"));
		auto zOffset = glm::vec3(0.f, 0.f, posFromThirdPerson.z);
		auto zSpaceOffset = thirdPersonInfo.rotation * zOffset;
		return FiringTransform {
			.position = thirdPersonInfo.position - zSpaceOffset,
			.rotation = thirdPersonInfo.rotation,
		};
	}

	auto position = gameapi -> getGameObjectPos(id, true, "[gamelogic] getFireTransform 2");
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