#include "./entity.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;
extern Movement movement;
extern Vehicles vehicles;
extern AiData aiData;

extern std::unordered_map<objid, ControllableEntity> controllableEntities;
extern std::unordered_map<objid, Inventory> scopenameToInventory;
extern MovementEntityData movementEntities;

extern int mainPlayerControl;
extern std::vector<ControlledPlayer> players;


void updateCamera(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	// ensure viewports
	if (players.size() == 1){
  	gameapi -> createViewport(0, 0.f, 0.f, 1.f, 1.f, DefaultBindingOption{}, {});
	}else if (players.size() == 2){
  	gameapi -> createViewport(0, 0.f, 0.5f, 1.f, 0.5f, DefaultBindingOption{}, {});
	  gameapi -> createViewport(1, 0.f, 0.0f, 1.f, 0.5f, DefaultBindingOption{}, {});
	}else {
		modassert(false, "invalid number of players");
	}

  ///////////////


	bool tempCameraDoesNotExistButShould = controlledPlayer.tempCamera.has_value() && !gameapi -> gameobjExists(controlledPlayer.tempCamera.value());
	bool activeCameraDoesNotExistButShould = controlledPlayer.activePlayerManagedCameraId.has_value() && !gameapi -> gameobjExists(controlledPlayer.activePlayerManagedCameraId.value());
	modassert(!tempCameraDoesNotExistButShould, "temp camera has value but obj does not exist");
	modassert(!activeCameraDoesNotExistButShould, "active camera has value but obj does not exist");	

	if (controlledPlayer.freeCamera){
		gameapi -> setActiveCamera(std::nullopt, controlledPlayer.viewport);
		return;
	}
	if (controlledPlayer.tempCamera.has_value()){
		gameapi -> setActiveCamera(controlledPlayer.tempCamera.value(), controlledPlayer.viewport);
		return;
	}
	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> setActiveCamera(controlledPlayer.activePlayerManagedCameraId.value(), controlledPlayer.viewport);
	}
}

////////////////////// Core Add / Rm  //////////////////////
void onAddEntity(objid idAdded){
	modlog("controllable entity added id:", std::to_string(idAdded));
  bool shouldAddWeapon = false;
  shouldAddWeapon = shouldAddWeapon || maybeAddMovementEntity(movementEntities, idAdded);
  auto agent = getSingleAttr(idAdded, "agent");
  if (agent.has_value()){
  	shouldAddWeapon = true;
    addAiAgent(aiData, idAdded, agent.value());
    controllableEntities[idAdded] = ControllableEntity {
    	.isInShootingMode = true,
    	.zoomIntoArcade = false,
    	.isAlive = true,
    	.lookingAtVehicle = std::nullopt,
    	.disableAnimationIds = entityIdsToDisable(idAdded),
    };

  }

  if (shouldAddWeapon){
	  addInventory(scopenameToInventory, idAdded);
	  addWeaponId(weapons, idAdded);
    createHitbox(idAdded);
  }
}

void onActivePlayerRemoved(objid id){
	ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());

	if (controlledPlayer.tempCamera.has_value() && controlledPlayer.tempCamera.value() == id){
		controlledPlayer.tempCamera = std::nullopt;
		updateCamera(getDefaultPlayerIndex());
	}
	if (controlledPlayer.entityId.has_value() && controlledPlayer.entityId.value() == id){
		controlledPlayer.entityId = std::nullopt;
		controlledPlayer.activePlayerManagedCameraId = std::nullopt; // probably should delete this too
		controlledPlayer.disablePlayerControl = false;
		updateCamera(getDefaultPlayerIndex());
		return;
	}
}

void onRemoveEntity(objid idRemoved){
	if (controllableEntities.find(idRemoved) != controllableEntities.end()){
		modlog("controllable entity removed id:", std::to_string(idRemoved));
	}
  maybeRemoveMovementEntity(movement, movementEntities, idRemoved);
  maybeRemoveAiAgent(aiData, idRemoved);
  removeWeaponId(weapons, idRemoved);
  removeInventory(scopenameToInventory, idRemoved);
  controllableEntities.erase(idRemoved);

  onActivePlayerRemoved(idRemoved);
}


////////////////////// Player port / viewport functions //////////////////////
ControlledPlayer& getControlledPlayer(int playerIndex){
	for (auto& player : players){
		if (player.viewport == playerIndex){
			return player;
		}
	}
	modassert(false, std::string("getControlledPlayer playerIndex invalid: ") + std::to_string(playerIndex));
	return players.at(0);
}

ControlledPlayer& getMainControlledPlayer(){
	return players.at(getDefaultPlayerIndex());
}

int getNumberOfPlayers(){
	return players.size();
}

void setMainPlayerControl(int playerIndex){
	mainPlayerControl = playerIndex;
}

void addPlayerPort(int playerIndex){
	ControlledPlayer player {
		.viewport = playerIndex,
		.lookVelocity = glm::vec2(0.f, 0.f),  // should come from movement state
		.entityId = std::nullopt,
		.activePlayerManagedCameraId = std::nullopt, // this is fixed camera for fps mode
		.tempCamera = std::nullopt,
		.freeCamera = false,
		.disablePlayerControl = false,	
	};
	players.push_back(player);
	addPlayerPortToMovement(movement, playerIndex);
}

void removePlayerPort(int playerIndex){
	std::vector<ControlledPlayer> newPlayers;
	for (auto& player : players){
		if (player.viewport == playerIndex){
			continue;
		}
		newPlayers.push_back(player);
	}
	players = newPlayers;
	removePlayerPortFromMovement(movement, playerIndex);
}

bool isControlledPlayer(int playerId){
	for (auto& player : players){
		if (player.entityId.has_value() && player.entityId.value() == playerId){
			return true;
		}
	}
	return false;
}

int getDefaultPlayerIndex(){
	return mainPlayerControl;
}

void setPlayerControlDisabled(bool isDisabled, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	controlledPlayer.disablePlayerControl = isDisabled;
	updateCamera(playerIndex);
}
bool isPlayerControlDisabled(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	return controlledPlayer.disablePlayerControl;
}

////////////////////// Lookup Utilities //////////////////////
std::optional<int> getPlayerIndexForEntity(objid id){
	for (auto &player : players){
		if (player.entityId.has_value() && player.entityId.value() == id){
			return player.viewport;
		}
	}
	return std::nullopt;
}

std::optional<objid> getPlayerId(int playerIndex){
	return getControlledPlayer(playerIndex).entityId;
}

std::optional<objid> getEntityForPlayerIndex(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	return controlledPlayer.entityId;
}

std::optional<ControllableEntity*> getActiveControllable(int playerIndex){
	auto activePlayer = getEntityForPlayerIndex(playerIndex);
	if (!activePlayer.has_value()){
		return std::nullopt;
	}
	return &controllableEntities.at(activePlayer.value());
}

std::vector<ControlledPlayer>& getPlayers(){
	modassert(players.size() > 0, "invalid player size");
	return players;
}


////////////////////// Core Entity To Viewport Binding //////////////////////
void setEntityForPlayerIndex(std::optional<objid> id, int playerIndex){
	modlog("set active player", std::to_string(playerIndex) + " set to: " + print(id));
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	if (!id.has_value()){
		return;
	}
	if (controlledPlayer.entityId.has_value()){
    maybeReEnableAi(aiData, controlledPlayer.entityId.value());
	}
	setActiveMovementEntity(movement, false, playerIndex);
	maybeDisableAi(aiData, id.value());
	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> removeByGroupId(controlledPlayer.activePlayerManagedCameraId.value());
		controlledPlayer.activePlayerManagedCameraId = std::nullopt;
	}

	controlledPlayer.entityId = id.value();

  GameobjAttributes attr { .attr = {{ "position", glm::vec3(0.f, 0.f, 0.f) }} };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id.value()), std::string(">gen-player-camera-") + uniqueNameSuffix(), attr, submodelAttributes).value();
  gameapi -> makeParent(cameraId, id.value());
  controlledPlayer.activePlayerManagedCameraId = cameraId;

  updateCamera(playerIndex);
}

std::optional<objid> getNextEntity(std::optional<objid> activeId){
  if (movementEntities.movementEntities.size() == 0){
    return std::nullopt;
  }
  std::vector<objid> allEntities;
  for (auto &[id, _] : movementEntities.movementEntities){
    allEntities.push_back(id);
  }
  if (!activeId.has_value()){
    return allEntities.at(0);
  }
  auto currId = activeId.value();
  std::optional<int> currentIndex = 0;
  for (int i = 0; i < allEntities.size(); i++){
    if (currId == allEntities.at(i)){
      currentIndex = i;
      break;
    }
  }
  auto index = currentIndex.value() + 1;
  if (index >= allEntities.size()){
    index = 0;
  }

  return allEntities.at(index);
}

void setEntityNextForPlayerIndex(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
  setEntityForPlayerIndex(getNextEntity(controlledPlayer.entityId), playerIndex);
}

void observeEntity(std::optional<objid> id, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	if (!id.has_value()){
		return;
	}

	setActiveMovementEntity(movement, true, playerIndex);

	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> removeByGroupId(controlledPlayer.activePlayerManagedCameraId.value());
		controlledPlayer.activePlayerManagedCameraId = std::nullopt;
	}

	controlledPlayer.entityId = id.value();

  GameobjAttributes attr { .attr = {{ "position", glm::vec3(0.f, 0.f, 0.f) }} };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id.value()), std::string(">gen-player-camera-") + uniqueNameSuffix(), attr, submodelAttributes).value();
  gameapi -> makeParent(cameraId, id.value());
  controlledPlayer.activePlayerManagedCameraId = cameraId;

  updateCamera(playerIndex);
}
void observeEntityNext(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	observeEntity(getNextEntity(controlledPlayer.entityId), playerIndex);
}


bool isEntity(objid id){
	auto isControllable = controllableEntities.find(id) != controllableEntities.end();
	return isControllable;
}

std::optional<objid> getCameraForThirdPerson(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	return controlledPlayer.activePlayerManagedCameraId;
}


////////////////////// Basic Entity Manipulation //////////////////////
void setEntityInShootingMode(objid id, bool shootingMode){
	modassert(controllableEntities.find(id) != controllableEntities.end(), std::string("controllable entity setInShootingMode for unregistered controllableEntity: ") + std::to_string(id));
	controllableEntities.at(id).isInShootingMode = shootingMode;
}

std::optional<bool> isEntityInShootingMode(objid id){
	if (controllableEntities.find(id) == controllableEntities.end()){
		return false;
	}
	return controllableEntities.at(id).isInShootingMode;
}

void setEntityIsAlive(objid id, bool alive){
	modassert(controllableEntities.find(id) != controllableEntities.end(), std::string("controllable entity setIsAlive for unregistered controllableEntity: ") + std::to_string(id));
	controllableEntities.at(id).isAlive = alive;
	//maybeDisableAi(aiData, id.value());
	MovementEntity& movementEntity = movementEntities.movementEntities.at(id);
	movementEntity.movementState.alive = alive;
}
std::optional<bool> isEntityAlive(objid id){
	if (!isEntity(id)){
		return std::nullopt;
	}
  auto alive = movementEntities.movementEntities.at(id).movementState.alive;
  return alive;
}

void setEntityIsFalling(objid id, bool falling){
	MovementEntity& movementEntity = movementEntities.movementEntities.at(id);
	movementEntity.movementState.falling = falling;
}
std::optional<bool> isEntityFalling(objid id){
	if (!isEntity(id)){
		return std::nullopt;
	}
  auto falling = movementEntities.movementEntities.at(id).movementState.falling;
  return falling;
}

void setEntityIsReloading(objid id, bool reloading){
	MovementEntity& movementEntity = movementEntities.movementEntities.at(id);
	if (!reloading){
		movementEntity.movementState.reloading = std::nullopt;
	}else{
		movementEntity.movementState.reloading = gameapi -> timeSeconds(false);
	}
}
std::optional<bool> isEntityReloading(objid id){
	if (!isEntity(id)){
		return std::nullopt;
	}
	return isReloading(movementEntities.movementEntities.at(id).movementState);
}

void setEntityZoom(objid id, float multiplier){
  auto& player = controllableEntities.at(id);
  bool isZoomedIn = multiplier < 1.f;
  if (isZoomedIn){
    player.zoomAmount = static_cast<int>((1.f / multiplier));
  }else{
    player.zoomAmount = std::nullopt;
  }
  setZoom(multiplier, isZoomedIn);
  setZoomSensitivity(movementEntities, multiplier, id);
  playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt, false);
}
bool isEntityGunZoomed(objid id){
  auto gunZoomed = getWeaponState(weapons, id).isGunZoomed;
  return gunZoomed;
}

std::optional<int> getEntityZoomAmount(objid id){
	if (!isEntityGunZoomed(id)){
		return std::nullopt;
	}
	return controllableEntities.at(id).zoomAmount;
}

void deliverEntityAmmo(objid id, int ammoAmount){
  deliverAmmoToCurrentGun(getWeaponState(weapons, id), ammoAmount, id);
}

glm::vec3 getEntityVelocity(objid id){
	modassert(isEntity(id), "getEntityVelocity not an entity");
  return movementEntities.movementEntities.at(id).movementState.velocity;
}

void killEntity(int id){
	if (!isEntity(id)){
		return;
	}
  doDamageMessage(id, 10000.f);   
}

void maybeReEnableMesh(objid id){
	modlog("disable mesh main", gameapi -> getGameObjNameForId(id).value());
	for (auto childId : gameapi -> getChildrenIdsAndParent(id)){
		 modlog("enable mesh", gameapi -> getGameObjNameForId(childId).value());
		 gameapi -> setMeshEnabled(childId, true, {});
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
		 	 if (isControlledPlayer(id)){
		 	 	int viewport = getPlayerIndexForEntity(id).value();
		 	 	ViewportMeshEnablement meshEnablement {
  					.enable = false,
  					.viewport = viewport,
			 	};
 			 	gameapi -> setMeshEnabled(childId, true, { meshEnablement });
		 	 }else{
 			 	gameapi -> setMeshEnabled(childId, true, {});
		 	 }

		 }
	}
}
void setEntityThirdPerson(objid id){
	modassert(isEntity(id), "setEntityThirdPerson not an entity");
	maybeReEnableMesh(id);
}
void setEntityFirstPerson(objid id){
	modassert(isEntity(id), "setEntityFirstPerson not an entity");
	maybeDisableMesh(id);
}

void disableEntity(objid id){
	maybeDisableMesh(id);
	setGameObjectPhysicsEnable(id, false);
}
void reenableEntity(objid id, std::optional<glm::vec3> pos, std::optional<glm::quat> rot){	
	setGameObjectPhysicsEnable(id, true);
	setGameObjectVelocity(id, glm::vec3(0.f, 0.f, 0.f));

	bool thirdPersonMode = getWeaponEntityData(id).thirdPersonMode;
	if (thirdPersonMode){
		maybeReEnableMesh(id);
	}
	if (pos.has_value()){
	  gameapi -> setGameObjectPosition(id, pos.value(), true, Hint{ .hint = "[gamelogic] - reenableEntity" });
	}
	if (rot.has_value()){
		setMovementEntityRotation(movementEntities, id, rot.value());
	}
}

void setEntityInVehicle(objid id, std::optional<objid> vehicleId){
	if (vehicleId.has_value()){
		auto& controllable = controllableEntities.at(id);
  	enterVehicle(vehicles, vehicleId.value(), id);
  	disableEntity(id);
  	controllable.vehicle = vehicleId.value();
	}else{
  	if (!canExitVehicle()){
  	  return;
  	}
  	auto& controllable = controllableEntities.at(id);
  	auto vehicleInfo = exitVehicle(vehicles, controllable.vehicle.value(), id);
  	reenableEntity(id, vehicleInfo.position, vehicleInfo.rotation);
 		controllable.vehicle = std::nullopt;
	}
}

void entityActionVehicle(objid id){
	auto& controllable = controllableEntities.at(id);
	if (controllable.vehicle.has_value()){
	  setEntityInVehicle(id, std::nullopt); 
	}else if (controllable.lookingAtVehicle.has_value()){
	  setEntityInVehicle(id, controllable.lookingAtVehicle.value());
	}
}

bool setEntityLookAt(objid id, std::optional<objid> lookAt){
	auto& controllable = controllableEntities.at(id);
	bool shouldDrawEnterText = false;
	std::optional<objid> lookingAtVehicle;
	if (lookAt.has_value()){
	  if (isVehicle(vehicles, lookAt.value()) &&  !controllable.vehicle.has_value()){
	    lookingAtVehicle = lookAt.value();
	    shouldDrawEnterText = true;
	  }
	}
	controllable.lookingAtVehicle = lookingAtVehicle;
	return shouldDrawEnterText;
}

void setEntityFocusArcade(std::optional<objid> arcadeId, objid id){
	auto playerIndex = getPlayerIndexForEntity(id).value();
  bool zoomIn = arcadeId.has_value();
  //getGlobalState().zoomIntoArcade = zoomIn;
  setPlayerControlDisabled(zoomIn, 0);
  if (!zoomIn){
    setTempCamera(std::nullopt, playerIndex);          
  }else{
    auto arcadeCameraId = findChildObjBySuffix(arcadeId.value(), ">camera");
    modassert(arcadeCameraId.has_value(), "arcadeCameraId does not have value");
    auto position = gameapi -> getGameObjectPos(arcadeId.value(), true, "[gamelogic] zoomIntoArcade get arcade camera location");
    auto rotation = gameapi -> getGameObjectRotation(arcadeId.value(), true, "[gamelogic] zoomIntoArcade get cmaera rotation");  // tempchecked
    setTempCamera(arcadeCameraId.value(), playerIndex);     
  }
}

std::optional<bool> isEntityFocusArcade(objid id){
	if (!isEntity(id)){
		return std::nullopt;
	}
  return controllableEntities.at(id).zoomIntoArcade;
}



std::set<objid>& getEntityDisabledAnimationIds(objid entityId){
  return controllableEntities.at(entityId).disableAnimationIds;
}


////////////////////// Convenience Utilities for player indexs //////////////////////

bool allPlayersDead(){
	for (auto& player : players){
		std::optional<ControllableEntity*> controllable = getActiveControllable(player.viewport);
		if (controllable.has_value() && controllable.value() -> isAlive){
			return false;
		}
	}
	return true;	
}

std::optional<glm::vec3> getEntityPositionByPlayerIndex(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	if (!controlledPlayer.entityId.has_value()){
		return std::nullopt;
	}
	return gameapi -> getGameObjectPos(controlledPlayer.entityId.value(), true, "[gamelogic] getActivePlayerPosition");
}


////////////////////// Camera Manipulation //////////////////////
void setPlayerFreeCamera(int playerIndex, bool editorMode){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	controlledPlayer.freeCamera = editorMode;
	updateCamera(playerIndex);
}

void setTempCamera(std::optional<objid> camera, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	controlledPlayer.tempCamera = camera;
	updateCamera(playerIndex);
}

void applyScreenshakeByPlayerIndex(int playerIndex, glm::vec3 impulse){
  if (hasOption("no-shake")){
    return;
  }
  getControlledPlayer(playerIndex).shakeImpulse = impulse;
}


////////////////////// Glue //////////////////////
FiringTransform getFireTransform(objid id, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	MovementEntity& movementEntity = movementEntities.movementEntities.at(id);

	if (movementEntity.managedCamera.thirdPersonMode && (controlledPlayer.entityId.has_value() && controlledPlayer.entityId.value() == id)){ // only use the third person code for the active player
		// this 0 out only works if these vectors are parallel, otherwise would have to solve the parametric eqtns 
		// z is set to the player entity so it doesnt shoot from behind the character.
		// for example, if you dont do this, if you zoom the cam out you can hit a target in the crosshair behind the character
		auto thirdPersonInfo = lookThirdPersonCalc(movementEntity.managedCamera, id, false);
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
	int playerIndex = getDefaultPlayerIndex();
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	MovementEntity& movementEntity = movementEntities.movementEntities.at(id);
  return WeaponEntityData {
    .inventory = id,
    .lookVelocity = controlledPlayer.lookVelocity, // this should be in the movementState instead....not be based off the players...
    .velocity = movementEntity.movementState.velocity,
    .thirdPersonMode = movementEntity.managedCamera.thirdPersonMode,
    .fireTransform = getFireTransform(id, playerIndex),
  };
}

////////////////////// Misc //////////////////////
std::optional<bool> entityInThirdPersonByPlayerIndex(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	if (!controlledPlayer.entityId.has_value()){
		return std::nullopt;
	}
	bool thirdPerson = getWeaponEntityData(controlledPlayer.entityId.value()).thirdPersonMode;
	return thirdPerson;
}

DebugConfig debugPrintActivePlayer(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

  DebugConfig debugConfig { .data = {} };

  {
  	std::string activeCameraName = "";
  	auto activeCameraId = gameapi -> getActiveCamera(std::nullopt);
  	if (activeCameraId.has_value()){
  		activeCameraName = gameapi -> getGameObjNameForId(activeCameraId.value()).value();
  	}
	  debugConfig.data.push_back({"activeCameraName", print(activeCameraId) + " [" + activeCameraName + "]" });
	}

	{
	  std::string playerName = "";
	  if (controlledPlayer.entityId.has_value()){
	  	playerName = gameapi -> getGameObjNameForId(controlledPlayer.entityId.value()).value();
	  }
	  debugConfig.data.push_back({"activeplayer id", print(controlledPlayer.entityId) + + " [" + playerName + "]" });
	}

	{
  	std::string cameraName = "";
  	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
  		cameraName = gameapi -> getGameObjNameForId(controlledPlayer.activePlayerManagedCameraId.value()).value();
  	}
  	debugConfig.data.push_back({"activePlayerManagedCameraId id", print(controlledPlayer.activePlayerManagedCameraId) + + " [" +  cameraName + "]" });
	}

	auto inThirdPerson = entityInThirdPersonByPlayerIndex(playerIndex);
	debugConfig.data.push_back({ "mode", (inThirdPerson.has_value() && inThirdPerson.value()) ? "third person" : "first person" });			

  return debugConfig;
}
