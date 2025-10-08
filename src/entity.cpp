#include "./entity.h"

extern CustomApiBindings* gameapi;
extern Weapons weapons;
extern Movement movement;

MovementEntityData& getMovementData();
void objectRemoved(objid idRemoved);
std::optional<objid> findChildObjBySuffix(objid id, const char* objName);

extern std::unordered_map<objid, ControllableEntity> controllableEntities;  // static-state extern
extern std::unordered_map<objid, Inventory> scopenameToInventory;     // static-state extern

int numberOfPlayers = 1;
int mainPlayerControl = 0;

std::vector<ControlledPlayer> players; // TODO static state

void addPlayerPort(int playerIndex){
	ControlledPlayer player {
		.viewport = playerIndex,
		.lookVelocity = glm::vec2(0.f, 0.f),  // should come from movement state
		.playerId = std::nullopt,
		.activePlayerManagedCameraId = std::nullopt, // this is fixed camera for fps mode
		.tempCamera = std::nullopt,
		.editorMode = false,
		.disablePlayerControl = false,	
	};
	players.push_back(player);
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
}

ControlledPlayer& getControlledPlayer(int playerIndex){
	for (auto& player : players){
		if (player.viewport == playerIndex){
			return player;
		}
	}
	modassert(false, "getControlledPlayer playerIndex invalid");
	return players.at(0);
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

std::vector<objid> findBodyPartAndChilren(objid entityId, const char* part){
  auto bodyPart = findBodyPart(entityId, part);
  if (!bodyPart.has_value()){
  	return {};
  }
  auto childIds = gameapi -> getChildrenIdsAndParent(bodyPart.value());
  std::vector<objid> ids;
  for (auto id : childIds){
  	ids.push_back(id);
  }
  return ids;
}

std::set<objid> entityIdsToDisable(objid entityId){
	std::set<objid> ids;
  auto leftHand = findBodyPart(entityId, "LeftHand");
  auto rightHand = findBodyPart(entityId, "RightHand");
  auto neck = findBodyPart(entityId, "Neck");
  auto head = findBodyPart(entityId, "Head");
  if (leftHand.has_value()){
	  ids.insert(leftHand.value());
  }
  if (rightHand.has_value()){
	  ids.insert(rightHand.value());
  }
  if (neck.has_value()){
	  ids.insert(neck.value());
  }
  if (head.has_value()){
	  ids.insert(head.value());
  }
	return ids;
}

std::set<objid> entityIdsToEnableForShooting(objid entityId){
	std::set<objid> ids;
	//{
  //	auto newIds = findBodyPartAndChilren(entityId, "RightShoulder");
  //	for (auto id : newIds){
  //		ids.insert(id);
  //	}
	//}
	//{
  //	auto newIds = findBodyPartAndChilren(entityId, "LeftShoulder");
  //	for (auto id : newIds){
  //		ids.insert(id);
  //	}
	//}
	{
  	auto newIds = findBodyPartAndChilren(entityId, "Spine");
  	for (auto id : newIds){
  		ids.insert(id);
  	}
	}
	return ids;
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

bool controllableEntityExists(objid id){
	return controllableEntities.find(id) != controllableEntities.end();
}

void updateCamera(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	// ensure viewports
	if (numberOfPlayers == 1){
  	gameapi -> createViewport(0, 0.f, 0.f, 1.f, 1.f, DefaultBindingOption{});
	}else if (numberOfPlayers == 2){
  	gameapi -> createViewport(0, 0.f, 0.5f, 1.f, 0.5f, DefaultBindingOption{});
	  gameapi -> createViewport(1, 0.f, 0.0f, 1.f, 0.5f, DefaultBindingOption{});
	}else {
		modassert(false, "invalid number of players");
	}

  ///////////////


	bool tempCameraDoesNotExistButShould = controlledPlayer.tempCamera.has_value() && !gameapi -> gameobjExists(controlledPlayer.tempCamera.value());
	bool activeCameraDoesNotExistButShould = controlledPlayer.activePlayerManagedCameraId.has_value() && !gameapi -> gameobjExists(controlledPlayer.activePlayerManagedCameraId.value());
	modassert(!tempCameraDoesNotExistButShould, "temp camera has value but obj does not exist");
	modassert(!activeCameraDoesNotExistButShould, "active camera has value but obj does not exist");	

	if (controlledPlayer.editorMode){
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

void setInShootingMode(objid id, bool shootingMode){
	modassert(controllableEntities.find(id) != controllableEntities.end(), std::string("controllable entity setInShootingMode for unregistered controllableEntity: ") + std::to_string(id));
	controllableEntities.at(id).isInShootingMode = shootingMode;
}

void setIsAlive(objid id, bool alive){
	modassert(controllableEntities.find(id) != controllableEntities.end(), std::string("controllable entity setIsAlive for unregistered controllableEntity: ") + std::to_string(id));
	controllableEntities.at(id).isAlive = alive;
	//maybeDisableAi(aiData, id.value());
	MovementEntity& movementEntity = getMovementData().movementEntities.at(id);
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
		 gameapi -> setMeshEnabled(childId, true);
	}
}

void maybeDisableMesh(objid id){
	modlog("disable mesh main", gameapi -> getGameObjNameForId(id).value());
	for (auto childId : gameapi -> getChildrenIdsAndParent(id)){
		 modlog("disable mesh", gameapi -> getGameObjNameForId(childId).value());
		 if (!hasCameraAncestor(childId)){ // guns are children of the camera 
 			 gameapi -> setMeshEnabled(childId, false);
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
	setActiveMovementEntity(movement, false);
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
void setActivePlayerNext(Movement& movement, Weapons& weapons, AiData& aiData, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
  setActivePlayer(movement, weapons, aiData, getNextEntity(getMovementData(), controlledPlayer.playerId), playerIndex);
}

void observePlayer(Movement& movement, Weapons& weapons, AiData& aiData, std::optional<objid> id, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	if (!id.has_value()){
		return;
	}

	setActiveMovementEntity(movement, true);

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
	observePlayer(movement, weapons, aiData, getNextEntity(getMovementData(), controlledPlayer.playerId), playerIndex);
}

int getDefaultPlayerIndex(){
	return mainPlayerControl;
}
std::optional<objid> getActivePlayerId(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	return controlledPlayer.playerId;
}

std::vector<ControlledPlayer>& getPlayers(){
	return players;
}


bool onActivePlayerRemoved(objid id){
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
		return true;
	}
	return false;
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
  auto alive = getMovementData().movementEntities.at(controlledPlayer.playerId.value()).movementState.alive;
  return alive;
}
std::optional<bool> activePlayerFalling(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
  auto falling = getMovementData().movementEntities.at(controlledPlayer.playerId.value()).movementState.falling;
  return falling;
}
void setIsFalling(objid id, bool falling){
	MovementEntity& movementEntity = getMovementData().movementEntities.at(id);
	movementEntity.movementState.falling = falling;
}

std::optional<bool> activePlayerReloading(int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

	if (!controlledPlayer.playerId.has_value()){
		return std::nullopt;
	}
	return isReloading(getMovementData().movementEntities.at(controlledPlayer.playerId.value()).movementState);
}
void setIsReloading(objid id, bool reloading){
	MovementEntity& movementEntity = getMovementData().movementEntities.at(id);
	if (!reloading){
		movementEntity.movementState.reloading = std::nullopt;
	}else{
		movementEntity.movementState.reloading = gameapi -> timeSeconds(false);
	}
}

void setActivePlayerEditorMode(bool editorMode, int playerIndex){
	ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
	controlledPlayer.editorMode = editorMode;
	updateCamera(playerIndex);
}
bool isInGameMode(){
	ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());
	return !controlledPlayer.editorMode;
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
  return getMovementData().movementEntities.at(controlledPlayer.playerId.value()).movementState.velocity;
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

	MovementEntity& movementEntity = getMovementData().movementEntities.at(id);

	if (movementEntity.managedCamera.thirdPersonMode && (controlledPlayer.playerId.has_value() && controlledPlayer.playerId.value() == id)){ // only use the third person code for the active player
		// this 0 out only works if these vectors are parallel, otherwise would have to solve the parametric eqtns 
		// z is set to the player entity so it doesnt shoot from behind the character.
		// for example, if you dont do this, if you zoom the cam out you can hit a target in the crosshair behind the character
		auto thirdPersonInfo = lookThirdPersonCalc(movementEntity.managedCamera, id);
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

	MovementEntity& movementEntity = getMovementData().movementEntities.at(id);
  return WeaponEntityData {
    .inventory = id,
    .lookVelocity = controlledPlayer.lookVelocity, // this should be in the movementState instead....not be based off the players...
    .velocity = movementEntity.movementState.velocity,
    .thirdPersonMode = movementEntity.managedCamera.thirdPersonMode,
    .fireTransform = getFireTransform(id, playerIndex),
  };
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

std::optional<ControllableEntity*> getActiveControllable(int playerIndex){
	auto activePlayer = getActivePlayerId(playerIndex);
	if (!activePlayer.has_value()){
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
		setMovementEntityRotation(getMovementData(), id, rot.value());
	}
}


/////////////////////

struct BoneShape {
	std::string bone;
	ShapeCreateType shape;
};
std::vector<BoneShape> boneValues = {
	BoneShape {
		.bone = "Hand",
		//.shape = PhysicsCreateSphere {
 		//	.radius = 0.5f,
		//},
		.shape = PhysicsCreateRect {
 			.width = 0.2f,
 			.height = 0.2f,
 			.depth = 0.2f,
		},
	},
	BoneShape {
		.bone = "Hips",
		//.shape = PhysicsCreateSphere {
 		//	.radius = 0.5f,
		//},
		.shape = PhysicsCreateRect {
 			.width = 0.2f,
 			.height = 0.2f,
 			.depth = 0.2f,
		},
	},
	BoneShape {
		.bone = "LeftUpLeg",
		//.shape = PhysicsCreateSphere {
 		//	.radius = 0.5f,
		//},
		.shape = PhysicsCreateRect {
 			.width = 0.2f,
 			.height = 0.2f,
 			.depth = 0.2f,
		},
	},
	BoneShape {
		.bone = "RightUpLeg",
		//.shape = PhysicsCreateSphere {
 		//	.radius = 0.5f,
		//},
		.shape = PhysicsCreateRect {
 			.width = 0.2f,
 			.height = 0.2f,
 			.depth = 0.2f,
		},
	},
	BoneShape {
		.bone = "Head",
		.shape = PhysicsCreateSphere {
 			.radius = 0.5f,
		},
	},
};

void createHitbox(objid playerModel){
	for (auto &value : boneValues){
		auto headValue = findChildObjBySuffix(playerModel, value.bone.c_str());
		if (headValue.has_value()){
			auto gameobj = gameapi -> getGameObjNameForId(headValue.value());
			std::cout << "debug createPhysicsBody: " << gameobj.value() << std::endl;

			gameapi -> createPhysicsBody(headValue.value(), value.shape);
			rigidBodyOpts physicsOptions {
	  	  .linear = glm::vec3(1.f, 1.f, 1.f),
	  	  .angular = glm::vec3(0.f, 0.f, 0.f),
	  	  .gravity = glm::vec3(0.f, -9.f, 0.f),
	  	  .friction = 1.f,
	  	  .restitution = 1.f,
	  	  .mass = 1.f,
	  	  .layer = physicsLayers.bones,
	  	  .linearDamping = 0.f,
	  	  .isStatic = true,
	  	  .hasCollisions = false,
			};
			gameapi -> setPhysicsOptions(headValue.value(), physicsOptions);	
		}	
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

std::string print(RigHit& righit){
	std::string value = "";
	value += std::string("headshot: ") + (righit.isHeadShot ? "true" : "false");
	return value;
}
std::optional<RigHit> handleRigHit(objid id) {
	auto groupId = gameapi -> groupId(id);
	auto isControllable = controllableEntityExists(groupId);
	if (!isControllable){
		return std::nullopt;
	}
	auto objectName = gameapi -> getGameObjNameForId(id).value();
	auto isHead = objectName.find("Head") != std::string::npos;
	std::cout << "doDamage: is head: " << (isHead ? "true" : "false") << std::endl;

	return RigHit {
		.isHeadShot = isHead,
		.mainId = groupId,
	};
}