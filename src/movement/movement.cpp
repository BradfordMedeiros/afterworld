#include "./movement.h"

extern CustomApiBindings* gameapi;

struct Movement {
  ControlParams controlParams;
};


std::optional<ActiveEntity> activeEntity;
std::unordered_map<objid, MovementEntity> movementEntities;

void setActiveEntity(objid id, std::optional<objid> managedCamera){
  activeEntity = ActiveEntity {
    .playerId = id,
    .managedCamera = !managedCamera.has_value() ? std::optional<ThirdPersonCameraInfo>(std::nullopt) : ThirdPersonCameraInfo {
      .id = managedCamera.value(),
      .distanceFromTarget = -5.f,
      .angleX = 0.f,
      .angleY = 0.f,
      .actualDistanceFromTarget = -5.f,
      .actualAngleX = 0.f,
      .actualAngleY = 0.f,
      .additionalCameraOffset = glm::vec3(-0.2f, 0.5f, 0.f),
      .zoomOffset = glm::vec3(-0.6f, -0.2f, -1.f),
      .actualZoomOffset = glm::vec3(0.f, 0.f, 0.f),
      .reverseCamera = false,
    },
  }; 
}
std::optional<objid> getNextEntity(){
  if (movementEntities.size() == 0){
    return std::nullopt;
  }
  std::vector<objid> allEntities;
  for (auto &[id, _] : movementEntities){
    allEntities.push_back(id);
  }
  if (!activeEntity.has_value()){
    return allEntities.at(0);
  }
  auto currId = activeEntity.value().playerId;
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

void setEntityTargetLocation(objid id, std::optional<MovementRequest> movementRequest){
  movementEntities.at(id).targetLocation = movementRequest;
}

void raycastFromCameraAndMoveTo(objid entityId){
  auto currentTransform = gameapi -> getCameraTransform();
  auto hitpoints = gameapi -> raycast(currentTransform.position, currentTransform.rotation, 100.f);

  if (hitpoints.size() > 0){
    glm::vec3 location = hitpoints.at(0).point;
    setEntityTargetLocation(entityId, MovementRequest {
      .position = location,
      .speed = 0.5f,
    });
    showDebugHitmark(hitpoints.at(0), -1);
  }
}

MovementEntity createMovementEntity(objid id, std::string&& name){
  MovementEntity movementEntity {
    .playerId = id,
  };
  movementEntity.movementState = getInitialMovementState(movementEntity.playerId);

  loadMovementCore(name);
  movementEntity.moveParams = findMovementCore(name);
  modassert(movementEntity.moveParams, "could not find movement core");
  movementEntity.movementState.lastMoveSoundPlayTime = 0.f;
  movementEntity.movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);
  setGameObjectPhysics(id, movementEntity.moveParams -> physicsMass, movementEntity.moveParams -> physicsRestitution, movementEntity.moveParams -> friction, movementEntity.moveParams -> gravity);
  return movementEntity;
}

void maybeAddMovementEntity(objid id){
  if (movementEntities.find(id) != movementEntities.end()){
    return;
  }
  auto player = getSingleAttr(id, "player");
  if (player.has_value()){
    auto playerProfile = getSingleAttr(id, "player-profile");
    movementEntities[id] = createMovementEntity(id, playerProfile.has_value() ? playerProfile.value() : "default");
  }
}
void maybeRemoveMovementEntity(objid id){
  if (activeEntity.has_value() && activeEntity.value().playerId == id){
    activeEntity = std::nullopt;
  }
  movementEntities.erase(id);
}

void reloadSettingsConfig(Movement& movement, std::string name){
  auto settingQuery = gameapi -> compileSqlQuery(
    "select xsensitivity, ysensitivity from settings where profile = " + name,
    {}
  );
  bool validSettingsSql = false;
  auto settingsResult = gameapi -> executeSqlQuery(settingQuery, &validSettingsSql);
  modassert(validSettingsSql, "error executing sql query");
  movement.controlParams.xsensitivity = floatFromFirstSqlResult(settingsResult, 0);
  movement.controlParams.ysensitivity = floatFromFirstSqlResult(settingsResult, 1);
}

void changeTargetId(Movement& movement, objid id){
  movement.controlParams.goForward = false;
  movement.controlParams.goBackward = false;
  movement.controlParams.goLeft = false;
  movement.controlParams.goRight = false;
  movement.controlParams.shiftModifier = false;
  movement.controlParams.doJump = false;
  movement.controlParams.lookVelocity = glm::vec2(0.f, 0.f);
  movement.controlParams.zoom_delta = 0.f;
  movement.controlParams.doAttachToLadder = false;
  movement.controlParams.doReleaseFromLadder = false;
  movement.controlParams.crouchType = CROUCH_NONE;
  reloadSettingsConfig(movement, "default");
}

CScriptBinding movementBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid _, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Movement* movement = new Movement;

    movement -> controlParams.goForward = false;
    movement -> controlParams.goBackward = false;
    movement -> controlParams.goLeft = false;
    movement -> controlParams.goRight = false;
    movement -> controlParams.shiftModifier = false;
    movement -> controlParams.doJump = false;
    movement -> controlParams.lookVelocity = glm::vec2(0.f, 0.f);
    movement -> controlParams.zoom_delta = 0.f;

    movement -> controlParams.doAttachToLadder = false;
    movement -> controlParams.doReleaseFromLadder = false;
    movement -> controlParams.crouchType = CROUCH_NONE;

    for (auto id : gameapi -> getObjectsByAttr("player", std::nullopt, std::nullopt)){
      maybeAddMovementEntity(id);
    }
    return movement;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Movement* value = (Movement*)data;
    removeAllMovementCores();
    delete value;
  };
  binding.onKeyCallback = [](int32_t _, void* data, int key, int scancode, int action, int mods) -> void {
    if (isPaused() || getGlobalState().disableGameInput){
      return;
    }
    Movement* movement = static_cast<Movement*>(data);
    if (!activeEntity.has_value()){
      return;
    }

    std::cout << "key is: " << key << std::endl;

    if (isCrouchKey(key)){  // ctrl
      if (action == 0 || action == 1){
        if (action == 0){
          movement -> controlParams.crouchType = CROUCH_UP;
        }else if (action == 1){
          movement -> controlParams.crouchType = CROUCH_DOWN;
        }
      }
    }

    if (key == 'R') { 
      if (action == 1){
        movement -> controlParams.doAttachToLadder = true;
      }else if (action == 0){
        movement -> controlParams.doReleaseFromLadder = true;
      }
      return;
    }

    if (isMoveForwardKey(key)){
      if (action == 0){
        movement -> controlParams.goForward = false;
      }else if (action == 1){
        movement -> controlParams.goForward = true;
      }
      return;
    }
    if (isMoveBackwardKey(key)){
      if (action == 0){
        movement -> controlParams.goBackward = false;
      }else if (action == 1){
        movement -> controlParams.goBackward = true;
      }
      return;
    }
    if (isMoveLeftKey(key)){
      if (action == 0){
        movement -> controlParams.goLeft = false;
      }else if (action == 1){
        movement -> controlParams.goLeft = true;
      }
      return;
    }
    if (isMoveRightKey(key)){
      if (action == 0){
        movement -> controlParams.goRight = false;
      }else if (action == 1){
        movement -> controlParams.goRight = true;
      }
      return;
    }

    if (isJumpKey(key) /* space */ && action == 1){
      Movement* movement = static_cast<Movement*>(data);
      movement -> controlParams.doJump = true;
      return;
    }

    if (key == 340 /* shift */){
      if (action == 0){
        movement -> controlParams.shiftModifier = false;
      }else if (action == 1){
        movement -> controlParams.shiftModifier = true;
        if (activeEntity.has_value() && activeEntity.value().managedCamera.has_value()){
          activeEntity.value().managedCamera.value().reverseCamera = !activeEntity.value().managedCamera.value().reverseCamera;
        }
      }
    }
  };
  binding.onMouseMoveCallback = [](objid _, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void {
    if (isPaused() || getGlobalState().disableGameInput){
      return;
    }
    Movement* movement = static_cast<Movement*>(data);
    if (!activeEntity.has_value()){
      return;
    }

    float xsensitivity = getGlobalState().xsensitivity;
    float ysensitivity = getGlobalState().ysensitivity * (getGlobalState().invertY ? -1.f : 1.f);
    movement -> controlParams.lookVelocity = glm::vec2(xsensitivity * xPos, ysensitivity * yPos);
  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    Movement* movement = static_cast<Movement*>(data);
    movement -> controlParams.zoom_delta = amount;
  };

  binding.onFrame = [](int32_t _, void* data) -> void {
    if (isPaused()){
      return;
    }
    Movement* movement = static_cast<Movement*>(data);
    //checkMovementCollisions(*movement);
    if (!activeEntity.has_value()){
      return;
    }
    MovementEntity& entity = movementEntities.at(activeEntity.value().playerId);

    auto controlData = getMovementControlData(movement -> controlParams, entity.movementState, *entity.moveParams);
    onMovementFrame(*entity.moveParams, entity.movementState, entity.playerId, controlData, activeEntity.value().managedCamera, getIsGunZoomed());
    
    //for (MovementEntity& movementEntity : movementEntities){
    //  if (movementEntity.targetLocation.has_value()){
    //    bool atTarget = false;
    //    auto controlData = getMovementControlDataFromTargetPos(movementEntity.targetLocation.value().position, movementEntity.targetLocation.value().speed, entity.movementState, entity.playerId, &atTarget);
    //    if (atTarget){
    //      movementEntity.targetLocation = std::nullopt;
    //    }
    //    onMovementFrame(*movementEntity.moveParams, movementEntity.movementState, movementEntity.playerId, controlData);  
    //  }
    //}


    movement -> controlParams.lookVelocity = glm::vec2(0.f, 0.f);
    movement -> controlParams.zoom_delta = 0.f;
    movement -> controlParams.doJump = false;
    movement -> controlParams.doAttachToLadder = false;
    movement -> controlParams.doReleaseFromLadder = false;
    movement -> controlParams.crouchType = CROUCH_NONE;

    //modlog("movement num entitiies: ", std::to_string(movementEntities.size()));
  };

  binding.onMessage = [](int32_t _, void* data, std::string& key, std::any& value){
    Movement* movement = static_cast<Movement*>(data);
    if (key == "active-player-change"){
      Movement* movement = static_cast<Movement*>(data);
      auto objIdValue = anycast<objid>(value); 
      modassert(objIdValue != NULL, "movement - request change control value invalid");
      changeTargetId(*movement, *objIdValue);
    }
  };

  binding.onObjectAdded = [](int32_t _, void* data, int32_t idAdded) -> void {
    maybeAddMovementEntity(idAdded);
  };
  binding.onObjectRemoved = [](int32_t _, void* data, int32_t idRemoved) -> void {
    maybeRemoveMovementEntity(idRemoved);
  };

  return binding;
}


