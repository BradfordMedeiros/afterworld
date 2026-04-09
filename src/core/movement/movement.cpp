#include "./movement.h"

extern CustomApiBindings* gameapi;

std::optional<bool> isInShootingMode(objid id);
std::optional<objid> findBodyPart(objid entityId, const char* part);
void setEntityThirdPerson(objid id);
void setEntityFirstPerson(objid id);

struct AnimGunPosUpdate {
  objid id;
  glm::vec3 pos;
};
struct AnimGunRotUpdate {
  objid id;
  glm::quat rot;
};

struct AnimationGunUpdate {
  std::optional<AnimGunPosUpdate> leftHand;
  std::optional<AnimGunRotUpdate> rightHand;
  std::optional<AnimGunRotUpdate> neck;
  std::optional<AnimGunRotUpdate> head;
};

AnimationGunUpdate updateEntityGunPosition(objid entityId, glm::quat orientation){
  AnimationGunUpdate update {};
  if (!isInShootingMode(entityId).value()){
    return update;
  }

  auto leftHand = findBodyPart(entityId, "LeftHand");
  auto rightHand = findBodyPart(entityId, "RightHand");
  auto neck = findBodyPart(entityId, "Neck");
  auto head = findBodyPart(entityId, "Head");

  if (rightHand.has_value()){
    update.rightHand = AnimGunRotUpdate {
      .id = rightHand.value(),
      .rot = orientation,
    };
  }

  if (!leftHand.has_value() || !rightHand.has_value() || !neck.has_value() || !head.has_value()){
    return update;
  }

  auto rightHandPosition = gameapi -> getGameObjectPos(rightHand.value(), true, "[gamelogic] updateEntityGunPosition - rightHandPosition");
  auto leftHandDir = orientation * glm::vec3(0.f, 0.f, -0.1f);
  auto newLeftHandPosition = rightHandPosition + leftHandDir;

  update.leftHand = AnimGunPosUpdate {
    .id = leftHand.value(),
    .pos = newLeftHandPosition,
  };

  auto headPosition = gameapi -> getGameObjectPos(neck.value(), true, "[gamelogic] updateEntityGunPosition - neckPosition");
  auto lookAtPosition = headPosition + (orientation * glm::vec3(0.f, 0.f, -10.f));

  auto headOrientation = gameapi -> orientationFromPos(lookAtPosition, headPosition);
  update.neck = AnimGunRotUpdate {
    .id = neck.value(),
    .rot = headOrientation,
  };
  update.head = AnimGunRotUpdate {
    .id = head.value(),
    .rot = headOrientation,
  };
  return update;
}

void setActiveMovementEntity(Movement& movement, bool observeMode, int playerPort){
  ControlParams& controlParams = getControlParamsByPort(movement, playerPort);;

  controlParams.goForward = false;
  controlParams.goBackward = false;
  controlParams.goLeft = false;
  controlParams.goRight = false;
  controlParams.shiftModifier = false;
  controlParams.doJump = false;
  controlParams.lookVelocity = glm::vec2(0.f, 0.f);
  controlParams.zoom_delta = 0.f;
  controlParams.doAttachToLadder = false;
  controlParams.doReleaseFromLadder = false;
  controlParams.doGrind = false;
  controlParams.doReverseGrind = false;
  controlParams.doReload = false;
  controlParams.crouchType = CROUCH_NONE;
  controlParams.observeMode = observeMode;
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
void setEntityTargetRotation(MovementEntityData& movementEntityData, objid id, std::optional<glm::quat> rotation){
  movementEntityData.movementEntities.at(id).targetRotation = rotation;
}

void raycastFromCameraAndMoveTo(MovementEntityData& movementEntityData, objid entityId, int viewportIndex){
  auto currentTransform = gameapi -> getCameraTransform(viewportIndex);
  auto hitpoints = gameapi -> raycast(currentTransform.position, currentTransform.rotation, 100.f, std::nullopt);

  if (hitpoints.size() > 0){
    glm::vec3 location = hitpoints.at(0).point;
    setEntityTargetLocation(movementEntityData, entityId, MovementRequest {
      .position = location,
      .speed = 0.5f,
    });
    drawDebugHitmark(hitpoints.at(0), -1);
  }
}


void setMovementEntityParams(MovementEntity& movementEntity, objid id, std::string& name){
  loadMovementCore(name);
  movementEntity.moveParams = findMovementCore(name);
  modassert(movementEntity.moveParams, "could not find movement core");
  setGameObjectPhysics(id, movementEntity.moveParams -> physicsMass, movementEntity.moveParams -> physicsRestitution, movementEntity.moveParams -> friction, movementEntity.moveParams -> gravity);
}
MovementEntity createMovementEntity(objid id, std::string& name){
  MovementEntity movementEntity {
    .playerId = id,
  };
  movementEntity.movementState = getInitialMovementState(movementEntity.playerId);
  movementEntity.managedCamera = ThirdPersonCameraInfo {
    .thirdPersonMode = false,
    .distanceFromTarget = -5.f,
    .angleX = 0.f,
    .angleY = 0.f,
    .actualDistanceFromTarget = -5.f,
    .additionalCameraOffset = glm::vec3(-1.f, 0.5f, 0.f),
    .zoomOffset = glm::vec3(-0.6f, -0.2f, -1.f),
    .actualZoomOffset = glm::vec3(0.f, 0.f, 0.f),
    .reverseCamera = false,
  };
  movementEntity.movementState.lastMoveSoundPlayTime = 0.f;
  movementEntity.movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);
  setMovementEntityParams(movementEntity, id, name);
  return movementEntity;
}

void changeMovementEntityType(MovementEntityData& movementEntityData, objid id, std::string name){
  MovementEntity& movementEntity = movementEntityData.movementEntities.at(id);
  setMovementEntityParams(movementEntity, id, name);
}

bool maybeAddMovementEntity(MovementEntityData& movementEntityData, objid id){
  if (movementEntityData.movementEntities.find(id) != movementEntityData.movementEntities.end()){
    return false;
  }
  auto player = getSingleAttr(id, "player");
  if (player.has_value()){
    movementEntityData.movementEntities[id] = createMovementEntity(id, player.value());
    return true;
  }
  return false;
}
void maybeRemoveMovementEntity(Movement& movement, MovementEntityData& movementEntityData, objid id){
  movement.disabledMeshes.erase(id);
  movementEntityData.movementEntities.erase(id);
}

ControlParams createControlParams(int playerPort){
  ControlParams controlParams;
  controlParams.playerPort = playerPort;
  controlParams.goForward = false;
  controlParams.goBackward = false;
  controlParams.goLeft = false;
  controlParams.goRight = false;
  controlParams.shiftModifier = false;
  controlParams.doJump = false;
  controlParams.lookVelocity = glm::vec2(0.f, 0.f);
  controlParams.zoom_delta = 0.f;

  controlParams.doAttachToLadder = false;
  controlParams.doReleaseFromLadder = false;
  controlParams.doGrind = false;
  controlParams.doReverseGrind = false;
  controlParams.doReload = false;
  controlParams.crouchType = CROUCH_NONE;
  controlParams.observeMode = false;
  return controlParams;
}
Movement createMovement(){
  Movement movement {};
  movement.disabledMeshes = {};
  movement.controlParams = { createControlParams(0) };
  return movement;
}

ControlParams& getControlParamsByPort(Movement& movement, int playerIndex){
  for (auto& controlParams : movement.controlParams){
    if (controlParams.playerPort == playerIndex){
      return controlParams;
    }
  }
  modassert(false, "invalid player port");
  return movement.controlParams.at(0);
}


void addPlayerPortToMovement(Movement& movement, int port){
  for (auto& controlParams : movement.controlParams){
    if (controlParams.playerPort == port){
      return;
    }
  }
  movement.controlParams.push_back(createControlParams(port));
}
void removePlayerPortFromMovement(Movement& movement, int port){

}

void onMovementKeyCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeId, int key, int action, int playerIndex){
  if (isPaused() || getGlobalState().disableGameInput){
    return;
  }

  ControlParams& controlParams = getControlParamsByPort(movement, playerIndex);
  if (isCrouchKey(key)){  // ctrl
    if (action == 0 || action == 1){
      if (action == 0){
        controlParams.crouchType = CROUCH_UP;
      }else if (action == 1){
        controlParams.crouchType = CROUCH_DOWN;
      }
    }
  }
  if (isClimbKey(key)) { 
    if (action == 1){
      controlParams.doAttachToLadder = true;
    }else if (action == 0){
      controlParams.doReleaseFromLadder = true;
    }
    return;
  }
  if (isMoveForwardKey(key)){
    if (action == 0){
      controlParams.goForward = false;
    }else if (action == 1){
      controlParams.goForward = true;
    }
    return;
  }
  if (isMoveBackwardKey(key)){
    if (action == 0){
      controlParams.goBackward = false;
    }else if (action == 1){
      controlParams.goBackward = true;
    }
    return;
  }
  if (isMoveLeftKey(key)){
    if (action == 0){
      controlParams.goLeft = false;
    }else if (action == 1){
      controlParams.goLeft = true;
    }
    return;
  }
  if (isMoveRightKey(key)){
    if (action == 0){
      controlParams.goRight = false;
    }else if (action == 1){
      controlParams.goRight = true;
    }
    return;
  }

  if (isJumpKey(key) /* space */ && action == 1){
    controlParams.doJump = true;
    return;
  }
  if (isGrindKey(key) && action == 1){
    controlParams.doGrind = true;
    return;
  }
  if (isReverseGrindKey(key) && action == 1){
    controlParams.doReverseGrind = true;
    return;
  }
  if (isReloadKey(key) && action == 1){
    controlParams.doReload = true;
    return;
  }

  if (key == 340 /* shift */){
    if (action == 0){
      controlParams.shiftModifier = false;
    }else if (action == 1){
      controlParams.shiftModifier = true;
      MovementEntity& entity = movementEntityData.movementEntities.at(activeId);
      if (entity.managedCamera.thirdPersonMode){
        entity.managedCamera.reverseCamera = !entity.managedCamera.reverseCamera;
      }
    }
  }

  if (isToggleThirdPersonKey(key) && action == 1){
    MovementEntity& entity = movementEntityData.movementEntities.at(activeId);
    entity.managedCamera.thirdPersonMode = !entity.managedCamera.thirdPersonMode;
  }
}

void onMovementMouseMoveCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeId, double xPos, double yPos, int playerPort){
  if (isPaused() || getGlobalState().disableGameInput){
    return;
  }

  float xsensitivity = getGlobalState().xsensitivity;
  float ysensitivity = getGlobalState().ysensitivity * (getGlobalState().invertY ? -1.f : 1.f);

  MovementEntity& movementEntity = movementEntityData.movementEntities.at(activeId);

  ControlParams& controlParams = getControlParamsByPort(movement, playerPort);
  controlParams.lookVelocity = glm::vec2(movementEntity.zoomSensitivity * xsensitivity * xPos, movementEntity.zoomSensitivity * ysensitivity * yPos);
}

void onMovementScrollCallback(Movement& movement, double amount, int playerPort){
  ControlParams& controlParams = getControlParamsByPort(movement, playerPort);
  controlParams.zoom_delta = amount;
}

glm::quat getLookDirection(MovementEntity& movementEntity){
  return weaponLookDirection(movementEntity.movementState);
}

glm::vec3 getMovementControlData(ControlParams& controlParams, MovementParams& moveParams){
  static float horzRelVelocity = 0.8f;

  glm::vec3 moveVec(0.f, 0.f, 0.f);
  if (controlParams.goForward){
    std::cout << "should move forward" << std::endl;
    if (controlParams.shiftModifier && moveParams.moveVertical){
      moveVec += glm::vec3(0.f, 1.f, 0.f);
    }else{
      moveVec += glm::vec3(0.f, 0.f, -1.f);
    }
  }
  if (controlParams.goBackward){
    if (controlParams.shiftModifier && moveParams.moveVertical){
      moveVec += glm::vec3(0.f, -1.f, 0.f);
    }else{
      moveVec += glm::vec3(0.f, 0.f, 1.f);
    }
  }
  if (controlParams.goLeft){
    moveVec += glm::vec3(horzRelVelocity * -1.f, 0.f, 0.f);
  }
  if (controlParams.goRight){
    moveVec += glm::vec3(horzRelVelocity * 1.f, 0.f, 0.f);
  }

  return moveVec;
}


bool isControlledPlayer(std::vector<MovementActivePlayer>& players, objid id){
  for (auto& player : players){
    if (id == player.activeId){
      return true;
    }
  }
  return false;
}
std::optional<MovementActivePlayer*> getControlledPlayer(std::vector<MovementActivePlayer>& players, objid id){
  for (auto& player : players){
    if (id == player.activeId){
      return &player;;
    }
  }
  return std::nullopt;
}



// TODO third person mode should only be a thing if active id
UiMovementUpdate onMovementFrame(MovementEntityData& movementEntityData, Movement& movement, std::function<bool(objid)> isGunZoomed, bool disableThirdPersonMesh, std::vector<EntityUpdate>& _entityUpdates, std::vector<MovementActivePlayer>& players){
  UiMovementUpdate uiUpdate {
    .velocity = std::nullopt,
    .lookVelocity = std::nullopt,
  };
  if (isPaused()){
    return uiUpdate;
  }

  {
    for (auto& player : players){
      ControlParams& controlParams = getControlParamsByPort(movement, player.playerPort);
      if (!controlParams.observeMode){
        MovementEntity& entity = movementEntityData.movementEntities.at(player.activeId);
        entity.movementState.moveVec = getMovementControlData(controlParams, *entity.moveParams);;
        entity.movementState.speed = 1.f;
        entity.movementState.zoom_delta = controlParams.zoom_delta;
        entity.movementState.doJump = controlParams.doJump;
        entity.movementState.doAttachToLadder = controlParams.doAttachToLadder;
        entity.movementState.doReleaseFromLadder = controlParams.doReleaseFromLadder;
        entity.movementState.doGrind = controlParams.doGrind;
        entity.movementState.doReverseGrind = controlParams.doReverseGrind;
        if (controlParams.doReload && !entity.movementState.reloading.has_value()){
          entity.movementState.reloading = gameapi -> timeSeconds(false);
        }
        entity.movementState.raw_deltax = controlParams.lookVelocity.x;
        entity.movementState.raw_deltay = -1.f * controlParams.lookVelocity.y;
        entity.movementState.crouchType = controlParams.crouchType;

        // should take the rotation and direct and stuff from where the player is looking
        auto cameraUpdate = onMovementFrameCore(*entity.moveParams, entity.movementState, entity.playerId, entity.managedCamera, isGunZoomed(player.activeId),  isControlledPlayer(players, entity.playerId));
        uiUpdate.velocity = entity.movementState.velocity;
        uiUpdate.lookVelocity = controlParams.lookVelocity;

        if (cameraUpdate.thirdPerson.has_value()){
          if (entity.movementState.alive){
            gameapi -> setGameObjectRot(entity.playerId, cameraUpdate.thirdPerson.value().yAxisRotation, true, Hint { .hint = "[gamelogic] onMovementFrame1 rot" });
          }
          auto gunAimingUpdates = updateEntityGunPosition(entity.playerId, cameraUpdate.thirdPerson.value().rotation);
          if (gunAimingUpdates.rightHand.has_value()){
            gameapi -> setGameObjectRot(gunAimingUpdates.rightHand.value().id, gunAimingUpdates.rightHand.value().rot, true, Hint { .hint = "updateEntityGunPosition right hand" });
          }        
          if (gunAimingUpdates.leftHand.has_value()){
            gameapi -> setGameObjectPosition(gunAimingUpdates.leftHand.value().id, gunAimingUpdates.leftHand.value().pos, true, Hint { .hint = "updateEntityGunPosition" });
          }
          if (gunAimingUpdates.neck.has_value()){
            gameapi -> setGameObjectRot(gunAimingUpdates.neck.value().id, gunAimingUpdates.neck.value().rot, true, Hint { .hint = "updateEntityGunPosition neck" });
          }
          if (gunAimingUpdates.head.has_value()){
            gameapi -> setGameObjectRot(gunAimingUpdates.head.value().id, gunAimingUpdates.head.value().rot, true, Hint { .hint = "updateEntityGunPosition head" });
          }
        }else{
          _entityUpdates.push_back(EntityUpdate {
            .id = entity.playerId,
            .rot = cameraUpdate.firstPerson.yAxisRotation,
            .rotHint = "[gamelogic] onMovementFrame rotatePlayerModelOnYAxis - rot",
          });
        }
      }
    }
  }

  for (auto &[id, movementEntity] : movementEntityData.movementEntities){
    auto controlledPlayer = getControlledPlayer(players, id);
    if (controlledPlayer.has_value() && !getControlParamsByPort(movement, controlledPlayer.value() -> playerPort).observeMode){
      continue;
    }
    if (movementEntity.targetLocation.has_value()){
      bool atTarget = false;
      movementEntity.movementState.moveVec = getMovementControlDataFromTargetPos(movementEntity.targetLocation.value().position, movementEntity.movementState, movementEntity.playerId, &atTarget, movementEntity.moveParams -> moveVertical);;
      movementEntity.movementState.speed = movementEntity.targetLocation.value().speed;
      movementEntity.movementState.zoom_delta = 0.f;

      if (atTarget){
        movementEntity.targetLocation = std::nullopt;
        movementEntity.movementState.speed = 1.f;
        continue;
      }
      auto cameraUpdate = onMovementFrameCore(*movementEntity.moveParams, movementEntity.movementState, movementEntity.playerId, movementEntity.managedCamera, isGunZoomed(id), isControlledPlayer(players, id));  
     
      if (!movementEntity.movementState.alive){  // hackey, this needs to be after core so the animations trigger still
        continue;
      }
      auto orientation = gameapi -> orientationFromPos(glm::vec3(movementEntity.movementState.lastPosition.x, 0.f, movementEntity.movementState.lastPosition.z), glm::vec3(movementEntity.targetLocation.value().position.x, 0.f, movementEntity.targetLocation.value().position.z));

      // not sure i should set the always here? 
      gameapi -> setGameObjectRot(movementEntity.playerId, orientation, true, Hint { .hint = "[gamelogic] movementEntity rot - targetLocation" });   // meh this should really come from the movement system (huh?)
      auto oldXYRot = pitchXAndYawYRadians(orientation);  // TODO - at least set this movement core area code.  at least call this "force". 
      movementEntity.movementState.xRot = oldXYRot.x;
      movementEntity.movementState.yRot = oldXYRot.y;    

      //modassert(!cameraUpdate.thirdPerson.has_value(), "tps camera update for a non-active entity");
    }
  }

  // TODO
  // ideally this rotation could just be a control request to the movement
  // that being said, this kind of just bypasses it an sets to the rotation directly
  // that's ... ok, but can run into bypassing constraints later on, hence it's really forcing it
  for (auto &[id, movementEntity] : movementEntityData.movementEntities){
    auto controlledPlayer = getControlledPlayer(players, id);
    if (controlledPlayer.has_value() && !getControlParamsByPort(movement, controlledPlayer.value() -> playerPort).observeMode){
      continue;
    }
    if (movementEntity.targetRotation.has_value()){
        gameapi -> setGameObjectRot(movementEntity.playerId, movementEntity.targetRotation.value(), true, Hint { .hint = "movementEntity rot - targetRotation" }); 
        auto oldXYRot = pitchXAndYawYRadians(movementEntity.targetRotation.value());  // TODO - at least set this movement core area code.  at least call this "force". 
        movementEntity.movementState.xRot = oldXYRot.x;
        movementEntity.movementState.yRot = oldXYRot.y;    
    }
  }



  for (auto &[id, movementEntity] : movementEntityData.movementEntities){
    movementEntity.movementState.moveVec = glm::vec3(0.f, 0.f, 0.f);
    movementEntity.movementState.speed = 1.f;
    movementEntity.movementState.zoom_delta = 0.f;
    movementEntity.movementState.doJump = false;
    movementEntity.movementState.doAttachToLadder = false;
    movementEntity.movementState.doReleaseFromLadder = false;
    movementEntity.movementState.doGrind = false;
    movementEntity.movementState.doReverseGrind = false;
    movementEntity.movementState.raw_deltax = 0.f;
    movementEntity.movementState.raw_deltay = 0.f;
    movementEntity.movementState.crouchType = CROUCH_NONE;

    movementEntity.targetRotation = std::nullopt;
  }

  for (auto &[id, movementEntity] : movementEntityData.movementEntities){
    if ((movementEntity.managedCamera.thirdPersonMode || isControlledPlayer(players, id)) && movement.disabledMeshes.count(id) > 0){
       movement.disabledMeshes.erase(id);
       setEntityThirdPerson(id);
    }
    if (isControlledPlayer(players, id) && (!movementEntity.managedCamera.thirdPersonMode || disableThirdPersonMesh)){
      if (movement.disabledMeshes.count(id) == 0){
        movement.disabledMeshes.insert(id);
        setEntityFirstPerson(id);       
      }
    }
  }


  for (auto& player : players){
    ControlParams& controlParams = getControlParamsByPort(movement, player.playerPort);
    controlParams.lookVelocity = glm::vec2(0.f, 0.f);
    controlParams.zoom_delta = 0.f;
    controlParams.doJump = false;
    controlParams.doAttachToLadder = false;
    controlParams.doReleaseFromLadder = false;
    controlParams.doGrind = false;
    controlParams.doReverseGrind = false;
    controlParams.doReload = false;
    controlParams.crouchType = CROUCH_NONE;    
  }

  return uiUpdate;
}

void setZoomSensitivity(MovementEntityData& movementEntityData, float multiplier, objid id){
  MovementEntity& movementEntity = movementEntityData.movementEntities.at(id);
  movementEntity.zoomSensitivity = multiplier;
}

void setMovementEntityRotation(MovementEntityData& movementEntityData, objid id, glm::quat rotation){
  MovementEntity& movementEntity = movementEntityData.movementEntities.at(id);
  gameapi -> setGameObjectRot(movementEntity.playerId, rotation, true, Hint { .hint = "movementEntity rot - setMovementEntityRotation" }); 
  auto oldXYRot = pitchXAndYawYRadians(rotation);  // TODO - at least set this movement core area code.  at least call this "force". 
  movementEntity.movementState.xRot = oldXYRot.x;
  movementEntity.movementState.yRot = oldXYRot.y;    

  movementEntity.managedCamera.angleX = movementEntity.movementState.xRot;
  movementEntity.managedCamera.angleY = movementEntity.movementState.yRot;  
}