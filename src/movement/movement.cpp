#include "./movement.h"

extern CustomApiBindings* gameapi;

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

void setActiveMovementEntity(Movement& movement, MovementEntityData& movementEntityData, objid id, std::optional<objid> managedCamera){
  movementEntityData.movementEntities.at(id).managedCamera = !managedCamera.has_value() ? std::optional<ThirdPersonCameraInfo>(std::nullopt) : ThirdPersonCameraInfo {
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
  };

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

void setEntityTargetLocation(MovementEntityData& movementEntityData, objid id, std::optional<MovementRequest> movementRequest){
  movementEntityData.movementEntities.at(id).targetLocation = movementRequest;
}

void raycastFromCameraAndMoveTo(MovementEntityData& movementEntityData, objid entityId){
  auto currentTransform = gameapi -> getCameraTransform();
  auto hitpoints = gameapi -> raycast(currentTransform.position, currentTransform.rotation, 100.f);

  if (hitpoints.size() > 0){
    glm::vec3 location = hitpoints.at(0).point;
    setEntityTargetLocation(movementEntityData, entityId, MovementRequest {
      .position = location,
      .speed = 0.5f,
    });
    drawDebugHitmark(hitpoints.at(0), -1);
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

bool maybeAddMovementEntity(MovementEntityData& movementEntityData, objid id){
  if (movementEntityData.movementEntities.find(id) != movementEntityData.movementEntities.end()){
    return false;
  }
  auto player = getSingleAttr(id, "player");
  if (player.has_value()){
    auto playerProfile = getSingleAttr(id, "player-profile");
    movementEntityData.movementEntities[id] = createMovementEntity(id, playerProfile.has_value() ? playerProfile.value() : "default");
    return true;
  }
  return false;
}
void maybeRemoveMovementEntity(MovementEntityData& movementEntityData, objid id){
  movementEntityData.movementEntities.erase(id);
}

Movement createMovement(){
  Movement movement {};
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
  return movement;
}

void onMovementKeyCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeId, int key, int action){
  if (isPaused() || getGlobalState().disableGameInput){
    return;
  }

  if (isCrouchKey(key)){  // ctrl
    if (action == 0 || action == 1){
      if (action == 0){
        movement.controlParams.crouchType = CROUCH_UP;
      }else if (action == 1){
        movement.controlParams.crouchType = CROUCH_DOWN;
      }
    }
  }
  if (isClimbKey(key)) { 
    if (action == 1){
      movement.controlParams.doAttachToLadder = true;
    }else if (action == 0){
      movement.controlParams.doReleaseFromLadder = true;
    }
    return;
  }
  if (isMoveForwardKey(key)){
    if (action == 0){
      movement.controlParams.goForward = false;
    }else if (action == 1){
      movement.controlParams.goForward = true;
    }
    return;
  }
  if (isMoveBackwardKey(key)){
    if (action == 0){
      movement.controlParams.goBackward = false;
    }else if (action == 1){
      movement.controlParams.goBackward = true;
    }
    return;
  }
  if (isMoveLeftKey(key)){
    if (action == 0){
      movement.controlParams.goLeft = false;
    }else if (action == 1){
      movement.controlParams.goLeft = true;
    }
    return;
  }
  if (isMoveRightKey(key)){
    if (action == 0){
      movement.controlParams.goRight = false;
    }else if (action == 1){
      movement.controlParams.goRight = true;
    }
    return;
  }

  if (isJumpKey(key) /* space */ && action == 1){
    movement.controlParams.doJump = true;
    return;
  }

  if (key == 340 /* shift */){
    if (action == 0){
      movement.controlParams.shiftModifier = false;
    }else if (action == 1){
      movement.controlParams.shiftModifier = true;
      MovementEntity& entity = movementEntityData.movementEntities.at(activeId);
      if (entity.managedCamera.has_value()){
        entity.managedCamera.value().reverseCamera = !entity.managedCamera.value().reverseCamera;
      }
    }
  }
}

float zoomSensitivity = 1.f;
void onMovementMouseMoveCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeId, double xPos, double yPos){
  if (isPaused() || getGlobalState().disableGameInput){
    return;
  }

  float xsensitivity = getGlobalState().xsensitivity;
  float ysensitivity = getGlobalState().ysensitivity * (getGlobalState().invertY ? -1.f : 1.f);
  movement.controlParams.lookVelocity = glm::vec2(zoomSensitivity * xsensitivity * xPos, zoomSensitivity * ysensitivity * yPos);
}

void onMovementScrollCallback(Movement& movement, double amount){
  movement.controlParams.zoom_delta = amount;
}


UiMovementUpdate onMovementFrame(MovementEntityData& movementEntityData, Movement& movement, objid activeId, std::function<bool(objid)> isGunZoomed){
  UiMovementUpdate uiUpdate {
    .velocity = std::nullopt,
    .lookVelocity = std::nullopt,
  };
  if (isPaused()){
    return uiUpdate;
  }

  {
    //MovementEntity& entity = movementEntityData.movementEntities.at(activeId);
    //auto controlData = getMovementControlData(movement.controlParams, entity.movementState, *entity.moveParams);
    //onMovementFrameCore(*entity.moveParams, entity.movementState, entity.playerId, controlData, entity.managedCamera, isGunZoomed(activeId));

    //uiUpdate.velocity = entity.movementState.velocity;
    //uiUpdate.lookVelocity = movement.controlParams.lookVelocity;
  }


  for (auto &[id, movementEntity] : movementEntityData.movementEntities){
    if (id == activeId){
      continue;
    }
    if (movementEntity.targetLocation.has_value()){
      bool atTarget = false;
      auto controlData = getMovementControlDataFromTargetPos(movementEntity.targetLocation.value().position, movementEntity.targetLocation.value().speed, movementEntity.movementState, movementEntity.playerId, &atTarget);
      if (atTarget){
        movementEntity.targetLocation = std::nullopt;
      }
      onMovementFrameCore(*movementEntity.moveParams, movementEntity.movementState, movementEntity.playerId, controlData, movementEntity.managedCamera, isGunZoomed(id));  
    }
  }

  movement.controlParams.lookVelocity = glm::vec2(0.f, 0.f);
  movement.controlParams.zoom_delta = 0.f;
  movement.controlParams.doJump = false;
  movement.controlParams.doAttachToLadder = false;
  movement.controlParams.doReleaseFromLadder = false;
  movement.controlParams.crouchType = CROUCH_NONE;
  return uiUpdate;
}

void setZoomSensitivity(float multiplier){
  zoomSensitivity = multiplier;
}