#include "./entity.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;
extern Movement movement;
extern Vehicles vehicles;

extern std::unordered_map<objid, ControllableEntity> controllableEntities;
extern std::unordered_map<objid, Inventory> scopenameToInventory;
extern MovementEntityData movementEntities;

extern int numberOfPlayers;
extern int mainPlayerControl;
extern std::vector<ControlledPlayer> players;

void objectRemoved(objid idRemoved);


void addPlayerPort(int playerIndex){
	ControlledPlayer player {
		.viewport = playerIndex,
		.lookVelocity = glm::vec2(0.f, 0.f),  // should come from movement state
		.playerId = std::nullopt,
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
		if (player.playerId.has_value() && player.playerId.value() == playerId){
			return true;
		}
	}
	return false;
}

std::optional<ControllableEntity*> getControllableEntity(objid id){
	if (controllableEntities.find(id) == controllableEntities.end()){
		return std::nullopt;
	}
	return &controllableEntities.at(id);
}

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

void setNumberPlayers(int numPlayers){
  modassert(numPlayers == 1 || numPlayers == 2, "invalid number of players");
  numberOfPlayers = numPlayers;
}

int getNumberOfPlayers(){
	return numberOfPlayers;
}

void setMainPlayerControl(int playerIndex){
	mainPlayerControl = playerIndex;
}

std::optional<objid> getPlayerId(int playerIndex){
	return getControlledPlayer(playerIndex).playerId;
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

void maybeRemoveControllableEntity(AiData& aiData, MovementEntityData& movementEntities, objid idRemoved){
	if (controllableEntities.find(idRemoved) != controllableEntities.end()){
		modlog("controllable entity removed id:", std::to_string(idRemoved));
	}
  maybeRemoveMovementEntity(movement, movementEntities, idRemoved);
  maybeRemoveAiAgent(aiData, idRemoved);
  removeWeaponId(weapons, idRemoved);
  removeInventory(scopenameToInventory, idRemoved);
  controllableEntities.erase(idRemoved);
}

void updateCamera(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	// ensure viewports
	if (numberOfPlayers == 1){
  	gameapi -> createViewport(0, 0.f, 0.f, 1.f, 1.f, DefaultBindingOption{}, {});
	}else if (numberOfPlayers == 2){
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

std::optional<objid> getCameraForThirdPerson(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	return controlledPlayer.activePlayerManagedCameraId;
}

std::optional<bool> activePlayerInThirdPerson(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
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

bool entityInShootingMode(objid id){
  return isInShootingMode(id).value();
}

void setInShootingMode(objid id, bool shootingMode){
	modassert(controllableEntities.find(id) != controllableEntities.end(), std::string("controllable entity setInShootingMode for unregistered controllableEntity: ") + std::to_string(id));
	controllableEntities.at(id).isInShootingMode = shootingMode;
}


void setIsAlive(objid id, bool alive){
	modassert(controllableEntities.find(id) != controllableEntities.end(), std::string("controllable entity setIsAlive for unregistered controllableEntity: ") + std::to_string(id));
	controllableEntities.at(id).isAlive = alive;
	//maybeDisableAi(aiData, id.value());
	MovementEntity& movementEntity = movementEntities.movementEntities.at(id);
	movementEntity.movementState.alive = alive;
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

void maybeReEnableMesh(objid id){
	modlog("disable mesh main", gameapi -> getGameObjNameForId(id).value());
	for (auto childId : gameapi -> getChildrenIdsAndParent(id)){
		 modlog("enable mesh", gameapi -> getGameObjNameForId(childId).value());
		 gameapi -> setMeshEnabled(childId, true, {});
	}
}

void maybeDisableMesh(objid id){
	modlog("disable mesh main", gameapi -> getGameObjNameForId(id).value());
	for (auto childId : gameapi -> getChildrenIdsAndParent(id)){
		 modlog("disable mesh", gameapi -> getGameObjNameForId(childId).value());
		 if (!hasCameraAncestor(childId)){ // guns are children of the camera 
		 	 if (isControlledPlayer(id)){
		 	 	int viewport = getPlayerIndex(id).value();
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

void setActivePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id, int playerIndex){
	modlog("set active player", std::to_string(playerIndex) + " set to: " + print(id));
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	if (!id.has_value()){
		return;
	}
	if (controlledPlayer.playerId.has_value()){
    maybeReEnableAi(aiData, controlledPlayer.playerId.value());
	}
	setActiveMovementEntity(movement, false, playerIndex);
	maybeDisableAi(aiData, id.value());
	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> removeByGroupId(controlledPlayer.activePlayerManagedCameraId.value());
		controlledPlayer.activePlayerManagedCameraId = std::nullopt;
	}

	controlledPlayer.playerId = id.value();

  GameobjAttributes attr { .attr = {{ "position", glm::vec3(0.f, 0.f, 0.f) }} };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id.value()), std::string(">gen-player-camera-") + uniqueNameSuffix(), attr, submodelAttributes).value();
  gameapi -> makeParent(cameraId, id.value());
  controlledPlayer.activePlayerManagedCameraId = cameraId;

  updateCamera(playerIndex);
}

std::optional<objid> getNextEntity(MovementEntityData& movementEntityData, std::optional<objid> activeId){
  if (movementEntityData.movementEntities.size() == 0){
    return std::nullopt;
  }
  std::vector<objid> allEntities;
  for (auto &[id, _] : movementEntityData.movementEntities){
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

void setActivePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
  setActivePlayer(movement, weapons, aiData, getNextEntity(movementEntities, controlledPlayer.playerId), playerIndex);
}

void observePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	if (!id.has_value()){
		return;
	}

	setActiveMovementEntity(movement, true, playerIndex);

	if (controlledPlayer.activePlayerManagedCameraId.has_value()){
		gameapi -> removeByGroupId(controlledPlayer.activePlayerManagedCameraId.value());
		controlledPlayer.activePlayerManagedCameraId = std::nullopt;
	}

	controlledPlayer.playerId = id.value();

  GameobjAttributes attr { .attr = {{ "position", glm::vec3(0.f, 0.f, 0.f) }} };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(id.value()), std::string(">gen-player-camera-") + uniqueNameSuffix(), attr, submodelAttributes).value();
  gameapi -> makeParent(cameraId, id.value());
  controlledPlayer.activePlayerManagedCameraId = cameraId;

  updateCamera(playerIndex);
}
void observePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	observePlayer(movement, weapons, aiData, getNextEntity(movementEntities, controlledPlayer.playerId), playerIndex);
}

int getDefaultPlayerIndex(){
	return mainPlayerControl;
}
std::optional<objid> getActivePlayerId(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	return controlledPlayer.playerId;
}

std::optional<int> getPlayerIndex(objid id){
	for (auto &player : players){
		if (player.playerId.has_value() && player.playerId.value() == id){
			return player.viewport;
		}
	}
	return std::nullopt;
}

std::vector<ControlledPlayer>& getPlayers(){
	modassert(players.size() > 0, "invalid player size");
	return players;
}

void onActivePlayerRemoved(objid id){
	ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());

	if (controlledPlayer.tempCamera.has_value() && controlledPlayer.tempCamera.value() == id){
		controlledPlayer.tempCamera = std::nullopt;
		updateCamera(getDefaultPlayerIndex());
	}
	if (controlledPlayer.playerId.has_value() && controlledPlayer.playerId.value() == id){
		controlledPlayer.playerId = std::nullopt;
		controlledPlayer.activePlayerManagedCameraId = std::nullopt; // probably should delete this too
		controlledPlayer.disablePlayerControl = false;
		updateCamera(getDefaultPlayerIndex());
		return;
	}
}

void setDisablePlayerControl(bool isDisabled, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	controlledPlayer.disablePlayerControl = isDisabled;
	updateCamera(playerIndex);
}
bool isPlayerControlDisabled(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	return controlledPlayer.disablePlayerControl;
}
std::optional<bool> activePlayerAlive(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
  auto alive = movementEntities.movementEntities.at(controlledPlayer.playerId.value()).movementState.alive;
  return alive;
}
std::optional<bool> activePlayerFalling(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
  auto falling = movementEntities.movementEntities.at(controlledPlayer.playerId.value()).movementState.falling;
  return falling;
}
void setIsFalling(objid id, bool falling){
	MovementEntity& movementEntity = movementEntities.movementEntities.at(id);
	movementEntity.movementState.falling = falling;
}

std::optional<bool> activePlayerReloading(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
	return isReloading(movementEntities.movementEntities.at(controlledPlayer.playerId.value()).movementState);
}
void setIsReloading(objid id, bool reloading){
	MovementEntity& movementEntity = movementEntities.movementEntities.at(id);
	if (!reloading){
		movementEntity.movementState.reloading = std::nullopt;
	}else{
		movementEntity.movementState.reloading = gameapi -> timeSeconds(false);
	}
}

bool allPlayersDead(){
	for (auto& player : players){
		std::optional<ControllableEntity*> controllable = getActiveControllable(player.viewport);
		if (controllable.has_value() && controllable.value() -> isAlive){
			return false;
		}
	}
	return true;	
}

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
std::optional<objid> getTempCamera(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	return controlledPlayer.tempCamera;
}

void killActivePlayer(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

  if (controlledPlayer.playerId.has_value()){
    doDamageMessage(controlledPlayer.playerId.value(), 10000.f);   
  }
}

glm::vec3 getPlayerVelocity(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
  return movementEntities.movementEntities.at(controlledPlayer.playerId.value()).movementState.velocity;
}

std::optional<glm::vec3> getActivePlayerPosition(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
	return gameapi -> getGameObjectPos(controlledPlayer.playerId.value(), true, "[gamelogic] getActivePlayerPosition");
}

std::optional<glm::quat> getActivePlayerRotation(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
	return gameapi -> getGameObjectRotation(controlledPlayer.playerId.value(), true, "[gamelogic] getActivePlayerRotation"); // tempchecked
}

FiringTransform getFireTransform(objid id, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	MovementEntity& movementEntity = movementEntities.movementEntities.at(id);

	if (movementEntity.managedCamera.thirdPersonMode && (controlledPlayer.playerId.has_value() && controlledPlayer.playerId.value() == id)){ // only use the third person code for the active player
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

std::optional<ControllableEntity*> getActiveControllable(int playerIndex){
	auto activePlayer = getActivePlayerId(playerIndex);
	if (!activePlayer.has_value()){
		return std::nullopt;
	}
	if (controllableEntities.find(activePlayer.value()) == controllableEntities.end()){
		return std::nullopt;
	}
	return &controllableEntities.at(activePlayer.value());
}

void setEntityThirdPerson(objid id){
	maybeReEnableMesh(id);
}
void setEntityFirstPerson(objid id){
	maybeDisableMesh(id);
}

void disableEntity(objid id){
	maybeDisableMesh(id);
	setGameObjectPhysicsEnable(id, false);
}
void reenableEntity(objid id, std::optional<glm::vec3> pos, std::optional<glm::quat> rot){
	int playerIndex = getDefaultPlayerIndex();
	
	setGameObjectPhysicsEnable(id, true);
	setGameObjectVelocity(id, glm::vec3(0.f, 0.f, 0.f));

	auto isActivePlayer = getActivePlayerId(playerIndex).has_value() && getActivePlayerId(playerIndex).value() == id;
	bool thirdPersonMode = getWeaponEntityData(id).thirdPersonMode;
	if (!isActivePlayer || thirdPersonMode){
		maybeReEnableMesh(id);
	}
	if (pos.has_value()){
	  gameapi -> setGameObjectPosition(id, pos.value(), true, Hint{ .hint = "[gamelogic] - reenableEntity" });
	}
	if (rot.has_value()){
		setMovementEntityRotation(movementEntities, id, rot.value());
	}
}

void enterRagdoll(objid playerModel){
	objectRemoved(playerModel);
	setGameObjectPhysicsEnable(playerModel, false);

	for (auto &value : boneValues){
		auto headValue = findChildObjBySuffix(playerModel, value.bone.c_str());
		auto gameobj = gameapi -> getGameObjNameForId(headValue.value());
		std::cout << "debug createPhysicsBody: " << gameobj.value() << std::endl;

		rigidBodyOpts physicsOptions {
		  .linear = glm::vec3(1.f, 1.f, 1.f),
		  .angular = glm::vec3(0.f, 0.f, 0.f),
		  .gravity = glm::vec3(0.f, -9.81f, 0.f),
		  .friction = 5.f,
		  .restitution = 1.f,
		  .mass = 1.f,
		  .layer = physicsLayers.bones,
		  .linearDamping = 0.f,
		  .isStatic = false,
		  .hasCollisions = true,
		};
		gameapi -> setPhysicsOptions(headValue.value(), physicsOptions);		
	}
}

bool isControllableEntity(objid id){
	auto isControllable = controllableEntities.find(id) != controllableEntities.end();
	return isControllable;
}

std::set<objid>& getDisabledAnimationIds(objid entityId){
  return controllableEntities.at(entityId).disableAnimationIds;
}

void deliverCurrentGunAmmo(objid id, int ammoAmount){
  deliverAmmoToCurrentGun(getWeaponState(weapons, id), ammoAmount, id);
}

bool isGunZoomed(objid id){
  auto gunZoomed = getWeaponState(weapons, id).isGunZoomed;
  return gunZoomed;
}

void enterVehicleEntity(int playerIndex, objid vehicleId, objid id){
  enterVehicle(vehicles, vehicleId, id);
  disableEntity(id);
  std::optional<ControllableEntity*> controllable = getActiveControllable(playerIndex);
  modassert(controllable.has_value() && controllable.value() != NULL, "controllable invalid");
  controllable.value() -> vehicle = vehicleId;
}

void exitVehicleEntity(int playerIndex, objid vehicleId, objid id){
  if (!canExitVehicle()){
    return;
  }
  auto vehicleInfo = exitVehicle(vehicles, getActiveControllable(playerIndex).value() -> vehicle.value(), id);
  reenableEntity(id, vehicleInfo.position, vehicleInfo.rotation);

  getActiveControllable(playerIndex).value() -> vehicle = std::nullopt;
}

bool isControlledVehicle(int vehicleId){
	for (auto& player : players){
		if (!player.playerId.has_value()){
			continue;
		}
		auto controllableEntity = getControllableEntity(player.playerId.value());
		if (controllableEntity.has_value()){
			if (controllableEntity.value() -> vehicle.has_value() && controllableEntity.value() -> vehicle.value() == vehicleId){
				return true;
			}
		}
	}
	return false;
}


void applyScreenshakeByPlayerIndex(int playerIndex, glm::vec3 impulse){
  if (hasOption("no-shake")){
    return;
  }
  getControlledPlayer(playerIndex).shakeImpulse = impulse;
}

void zoomIntoArcade(std::optional<objid> id, int playerIndex){
  bool zoomIn = id.has_value();
  //getGlobalState().zoomIntoArcade = zoomIn;
  setDisablePlayerControl(zoomIn, 0);
  if (!zoomIn){
    setTempCamera(std::nullopt, playerIndex);          
  }else{
    auto arcadeCameraId = findChildObjBySuffix(id.value(), ">camera");
    modassert(arcadeCameraId.has_value(), "arcadeCameraId does not have value");
    auto position = gameapi -> getGameObjectPos(id.value(), true, "[gamelogic] zoomIntoArcade get arcade camera location");
    auto rotation = gameapi -> getGameObjectRotation(id.value(), true, "[gamelogic] zoomIntoArcade get cmaera rotation");  // tempchecked
    setTempCamera(arcadeCameraId.value(), playerIndex);     
  }
}

std::optional<bool> isZoomedIntoArcade(int playerIndex){
	auto controlledPlayer = getActiveControllable(playerIndex);
  if (!controlledPlayer.has_value()){
  	return false;
  }
  return controlledPlayer.value() -> zoomIntoArcade;
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

	auto inThirdPerson = activePlayerInThirdPerson(playerIndex);
	debugConfig.data.push_back({ "mode", (inThirdPerson.has_value() && inThirdPerson.value()) ? "third person" : "first person" });			

  return debugConfig;
}
