#include "./movementcore.h"

extern CustomApiBindings* gameapi;

struct MovementCore {
  std::string name;
  MovementParams moveParams;
};

std::unordered_map<std::string, MovementCore> movementCores = {};

MovementParams* findMovementCore(std::string& name){
  for (auto &[_, movementCore] : movementCores){
    if (movementCore.name == name){
      //modlog("movement, comparing", name + ", to " + movementCore.name);
      return &movementCore.moveParams;
    }
  }
  return NULL;
}

MovementParams getMovementParams(std::string name){
  auto traitsQuery = gameapi -> compileSqlQuery(
    "select speed, speed-air, jump-height, gravity, restitution, friction, max-angleup, max-angledown, mass, jump-sound, land-sound, dash, dash-sound, move-sound, move-sound-distance, move-sound-mintime, ground-angle, gravity-water, crouch, crouch-speed, crouch-scale, crouch-delay, crouch-friction, speed-water, move-vertical from traits where profile = " + name,
    {}
  );
  bool validTraitSql = false;
  auto result = gameapi -> executeSqlQuery(traitsQuery, &validTraitSql);
  modassert(validTraitSql, "error executing sql query");

  MovementParams moveParams {};
  moveParams.moveSpeed = floatFromFirstSqlResult(result, 0);
  moveParams.moveSpeedAir = floatFromFirstSqlResult(result, 1);
  moveParams.moveSpeedWater = floatFromFirstSqlResult(result, 23);
  moveParams.jumpHeight = floatFromFirstSqlResult(result, 2);
  moveParams.maxAngleUp = floatFromFirstSqlResult(result, 6);
  moveParams.maxAngleDown = floatFromFirstSqlResult(result, 7);
  moveParams.moveSoundDistance = floatFromFirstSqlResult(result, 14);
  moveParams.moveSoundMintime = floatFromFirstSqlResult(result, 15);
  moveParams.groundAngle = glm::cos(glm::radians(floatFromFirstSqlResult(result, 16)));
  moveParams.gravity = glm::vec3(0.f, floatFromFirstSqlResult(result, 3), 0.f);
  moveParams.canCrouch = boolFromFirstSqlResult(result, 18);
  moveParams.moveVertical = boolFromFirstSqlResult(result, 24);
  moveParams.crouchSpeed = floatFromFirstSqlResult(result, 19);
  moveParams.crouchScale = floatFromFirstSqlResult(result, 20);
  moveParams.crouchDelay = floatFromFirstSqlResult(result, 21);
  moveParams.friction = floatFromFirstSqlResult(result, 5);
  moveParams.crouchFriction = floatFromFirstSqlResult(result, 22);
  moveParams.physicsMass = floatFromFirstSqlResult(result, 8);
  moveParams.physicsRestitution = floatFromFirstSqlResult(result, 4);

  moveParams.jumpSound = result.at(0).at(9);
  moveParams.landSound = result.at(0).at(10);
  moveParams.moveSound = result.at(0).at(13);

  modlog("movement - load params - jump height", std::string("profile - ") + name + ", jump height = " + std::to_string(moveParams.jumpHeight));

  return moveParams;
}

void loadMovementCore(std::string& coreName){
  modlog("movement", std::string("load movement core: ") + coreName);
  if (findMovementCore(coreName)){
    modlog("movement", "core already loaded");
    return;
  }

  MovementCore movementCore { .name = coreName };
  movementCore.moveParams = getMovementParams(coreName);
  ensureSoundsLoaded(gameapi -> rootSceneId(), movementCore.moveParams.jumpSound, movementCore.moveParams.landSound, movementCore.moveParams.moveSound);
  movementCores[coreName] = movementCore;
}

void removeAllMovementCores(){
  movementCores = {};
  ensureSoundsUnloaded(gameapi -> rootSceneId());
}

void jump(MovementParams& moveParams, MovementState& movementState, objid id){
  glm::vec3 impulse(0, moveParams.jumpHeight, 0);
  modlog("movement - jump - height: ", std::to_string(moveParams.jumpHeight));
  if (movementState.isGrounded){
    gameapi -> applyImpulse(id, impulse);
    if (getManagedSounds().jumpSoundObjId.has_value()){
      playGameplayClipById(getManagedSounds().jumpSoundObjId.value(), std::nullopt, std::nullopt);
    }
  }
  if (movementState.inWater){
    gameapi -> applyImpulse(id, impulse);
  }
  gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
    .entityId = id,
    .transition = "jump",
  });
}

void land(objid id){
  if (getManagedSounds().landSoundObjId.has_value()){
    playGameplayClipById(getManagedSounds().landSoundObjId.value(), std::nullopt, std::nullopt);
  }
}

void moveUp(objid id, glm::vec2 direction){
  float time = gameapi -> timeElapsed();
  gameapi -> applyImpulse(id, time * glm::vec3(0.f, -direction.y, 0.f));
}

void moveAbsolute(objid id, glm::vec3 direction){ // i wonder if i should make this actually parellel to the surface the player is moving along
  //modlog("editor: move xz: ", print(direction));
  float time = gameapi -> timeElapsed();
  auto directionVec = direction;

  auto magnitude = glm::length(directionVec);
  if (aboutEqual(magnitude, 0.f)){
    //modassert(false, "magnitude was about zero");
    modlog("movement", "warning magnitude was about zero");
    return;
  }
  gameapi -> applyImpulse(id, time * glm::normalize(directionVec) * magnitude); // change to apply force since every frame
}


float ironsightSpeedMultiplier = 0.4f;
float getMoveSpeed(MovementParams& moveParams, MovementState& movementState, bool ironsight, bool isGrounded){
  auto baseMoveSpeed = movementState.isCrouching ? moveParams.crouchSpeed : moveParams.moveSpeed;
  auto speed = isGrounded ? baseMoveSpeed : moveParams.moveSpeedAir;
  speed = movementState.inWater ? moveParams.moveSpeedWater : speed;
  if (ironsight){
    speed = speed * ironsightSpeedMultiplier;
  }
  return speed;
}

void updateVelocity(MovementState& movementState, objid id, float elapsedTime, glm::vec3 currPos, bool* _movingDown){ // returns if moveing down
  glm::vec3 displacement = (currPos - movementState.lastPosition) / elapsedTime;
  //auto speed = glm::length(displacement);
  movementState.lastPosition = currPos;
  //std::cout << "velocity = " << print(displacement) << ", speed = " << speed << std::endl;
  setPlayerVelocity(displacement);

  *_movingDown = displacement.y < 0.f;
}

void updateFacingWall(MovementState& movementState, objid id){
  auto mainobjPos = gameapi -> getGameObjectPos(id, true);
  auto rot = gameapi -> getGameObjectRotation(id, true);
  //  (define shotangle (if (should-zoom) rot (with-bloom rot)))
  auto hitpoints =  gameapi -> raycast(mainobjPos, rot, 2.f);

  if (hitpoints.size() > 0){
    movementState.facingWall = true;
    auto hitpoint = hitpoints.at(closestHitpoint(hitpoints, mainobjPos));
    auto attr = getAttrHandle(hitpoint.id);
    movementState.facingLadder = getStrAttr(attr, "ladder").has_value();
  }else{
    movementState.facingWall = false;
    movementState.facingLadder = false;
  }
}

void restrictLadderMovement(MovementState& movementState, objid id, bool movingDown){
  if (movementState.attachedToLadder){
    auto attr = getAttrHandle(id);
    auto velocity = getVec3Attr(attr, "physics_velocity").value();
    if (velocity.y > 0.f){
      return;
    }
    velocity.y = 0.f;
    setGameObjectVelocity(id, velocity);
  }
}

void look(MovementParams& moveParams, MovementState& movementState, objid id, float elapsedTime, bool ironsight, float ironsight_turn, float turnX, float turnY){
  auto forwardVec = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));

  float raw_deltax = turnX * elapsedTime;
  float raw_deltay = turnY * elapsedTime; 

  float deltax = ironsight ? (raw_deltax * ironsight_turn) : raw_deltax;
  float deltay = ironsight ? (raw_deltay * ironsight_turn) : raw_deltay;

  movementState.xRot = limitAngle(movementState.xRot + deltax, std::nullopt, std::nullopt);
  movementState.yRot = limitAngle(movementState.yRot + deltay, moveParams.maxAngleUp, moveParams.maxAngleDown); 
  auto rotation = gameapi -> setFrontDelta(forwardVec, movementState.xRot, movementState.yRot, 0, 1.f);
  gameapi -> setGameObjectRot(id, rotation, false);
}


// this code should use the main look logic, and then manage the camera for now separate codepaths but shouldn't be, probably 
void lookThirdPerson(MovementParams& moveParams, MovementState& movementState, float turnX, float turnY, float zoom_delta, objid id, std::optional<ThirdPersonCameraInfo>& managedCamera, float elapsedTime, bool isGunZoomed){
  ThirdPersonCameraInfo& thirdPersonInfo = managedCamera.value();
  thirdPersonInfo.angleX += turnX * 0.05 /* 0.05f arbitary turn speed */;
  thirdPersonInfo.angleY += turnY * 0.05 /* 0.05f arbitary turn speed */;
  thirdPersonInfo.distanceFromTarget += zoom_delta;

  float reverseMultiplier = thirdPersonInfo.reverseCamera ? -1.f : 1.f;

  auto finalAdjustedDistance = isGunZoomed ? thirdPersonInfo.zoomOffset.z :  thirdPersonInfo.distanceFromTarget;
  float zoomSpeed = isGunZoomed ? 10.f : 2.f;

  thirdPersonInfo.actualAngleX = glm::lerp(thirdPersonInfo.actualAngleX, thirdPersonInfo.angleX, glm::clamp(elapsedTime * 2.f, 0.f, 1.f));  // 5.f arbitrary lerp amount
  thirdPersonInfo.actualAngleY = glm::lerp(thirdPersonInfo.actualAngleY, thirdPersonInfo.angleY, glm::clamp(elapsedTime * 2.f, 0.f, 1.f));
  thirdPersonInfo.actualDistanceFromTarget = glm::lerp(thirdPersonInfo.actualDistanceFromTarget, finalAdjustedDistance, glm::clamp(elapsedTime * zoomSpeed, 0.f, 1.f));
  thirdPersonInfo.actualZoomOffset.x = glm::lerp(thirdPersonInfo.actualZoomOffset.x, isGunZoomed ? (thirdPersonInfo.zoomOffset.x * reverseMultiplier) : 0.f, glm::clamp(elapsedTime * zoomSpeed, 0.f, 1.f));
  thirdPersonInfo.actualZoomOffset.y = glm::lerp(thirdPersonInfo.actualZoomOffset.y, isGunZoomed ? thirdPersonInfo.zoomOffset.y : 0.f, glm::clamp(elapsedTime * zoomSpeed, 0.f, 1.f));


  float x = glm::cos(thirdPersonInfo.actualAngleX) * thirdPersonInfo.actualDistanceFromTarget;
  float z = glm::sin(thirdPersonInfo.actualAngleX) * thirdPersonInfo.actualDistanceFromTarget;
  float y = glm::cos(thirdPersonInfo.actualAngleY) * thirdPersonInfo.actualDistanceFromTarget;

  auto targetLocation = movementState.lastPosition;
  auto fromLocation = targetLocation + glm::vec3(x, -y, z);
  auto newOrientation = orientationFromPos(fromLocation, targetLocation);

  auto finalCameraLocation = fromLocation + (newOrientation * glm::vec3(reverseMultiplier * thirdPersonInfo.additionalCameraOffset.x, thirdPersonInfo.additionalCameraOffset.y, thirdPersonInfo.additionalCameraOffset.z)) + (newOrientation * glm::vec3(thirdPersonInfo.actualZoomOffset.x, thirdPersonInfo.actualZoomOffset.y, 0.f));
    
  gameapi -> setGameObjectRot(managedCamera.value().id, newOrientation, true);
  gameapi -> setGameObjectPosition(managedCamera.value().id, finalCameraLocation, true);

  // should encode the same logic as look probably, and this also shouldn't be exactly the same as the camera.  Don't set upward rotation
  gameapi -> setGameObjectRot(id, newOrientation, true);  

}


void attachToLadder(MovementState& movementState){
  if (movementState.facingLadder){
     movementState.attachedToLadder = true;
  }
}
void releaseFromLadder(MovementState& movementState){
  movementState.attachedToLadder = false;
}

void toggleCrouch(MovementParams& moveParams, MovementState& movementState, objid id, bool shouldCrouch){
  modlog("movement", "toggle crouch: " + print(shouldCrouch));
  auto crouchScale = moveParams.crouchScale;
  auto scale = shouldCrouch ? (crouchScale * movementState.initialScale) : movementState.initialScale;
  float friction = shouldCrouch ? moveParams.crouchFriction : moveParams.friction;
  setGameObjectFriction(id, friction);

  gameapi -> setGameObjectScale(id, scale, true);
  if (shouldCrouch){
    // two reasons: 
    // bug where scaling and object makes the object float in the air 
    // old school crouch jump extra height (intentional)
    gameapi -> applyImpulse(id, glm::vec3(0.f, 1.f, 0.f)); 
  }
}

float getPlayerTop(glm::vec3 playerPos){
  auto playerHitboxHeight = 1.f;
  return playerPos.y + (playerHitboxHeight * 0.5f);
}
bool somethingAbovePlayer(glm::vec3 playerPos){
  auto abovePlayer = getPlayerTop(playerPos);
  playerPos.y = abovePlayer;
  auto hitpoints =  gameapi -> raycast(playerPos, gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f)), 0.6f);
  return hitpoints.size() > 0;
}
void updateCrouch(MovementParams& moveParams, MovementState& movementState, objid id){
  if (movementState.shouldBeCrouching && !movementState.isCrouching){
    movementState.isCrouching = true;
    movementState.lastCrouchTime = gameapi -> timeSeconds(false);
    toggleCrouch(moveParams, movementState, id, true);
  }else if (!movementState.shouldBeCrouching && movementState.isCrouching){
    auto playerPos = gameapi -> getGameObjectPos(id, true);
    auto canUncrouch = !somethingAbovePlayer(playerPos);
    if (canUncrouch){
      movementState.isCrouching = false;
      toggleCrouch(moveParams, movementState, id, false);
    }
  }
}

bool isCollideable(objid id){
  auto attrHandle = getAttrHandle(id);
  return getBoolAttr(attrHandle, "physics_collision").value();
}

bool shouldStepUp(objid id){ // check this logic 
  auto playerPos = gameapi -> getGameObjectPos(id, true);
  auto inFrontOfPlayer = gameapi -> getGameObjectRotation(id, true) * glm::vec3(0.f, 0.f, -1.f);
  auto inFrontOfPlayerSameHeight = playerPos + glm::vec3(inFrontOfPlayer.x, 0.f, inFrontOfPlayer.z);
  
  auto belowPos = playerPos - glm::vec3(0.f, 0.95f, 0.f);
  auto belowPosSameHeight = inFrontOfPlayerSameHeight - glm::vec3(0.f, 0.95f, 0.f);
  
  auto abovePos = playerPos - glm::vec3(0.f, 1.f, 0.f);
  auto abovePosSameHeight = inFrontOfPlayerSameHeight - glm::vec3(0.f, 0.2f, 0.f);

  auto belowDir = gameapi -> orientationFromPos(belowPos, belowPosSameHeight);
  auto aboveDir = gameapi -> orientationFromPos(abovePos, abovePosSameHeight);

  auto belowHitpoints = gameapi -> raycast(belowPos, belowDir, 2.f);
  auto aboveHitpoints = gameapi -> raycast(abovePos, aboveDir, 2.f);

  bool anyCollideableBelow = false;
  for (auto &hitpoint : belowHitpoints){
    if (isCollideable(hitpoint.id)){
      anyCollideableBelow = true;
      break;
    }
  }

  //gameapi -> drawLine(belowPos, gameapi -> moveRelative(belowPos, belowDir, 2.f), true, id, glm::vec4(1.f, 0.f, 0.f, 1.f),  std::nullopt, std::nullopt);
  //gameapi -> drawLine(abovePos, gameapi -> moveRelative(abovePos, aboveDir, 2.f), true, id, glm::vec4(0.f, 0.f, 1.f, 1.f),  std::nullopt, std::nullopt);

  //std::cout << "hitpoints:  low = " << belowHitpoints.size() << ", high = " << aboveHitpoints.size() << std::endl;
  return anyCollideableBelow && aboveHitpoints.size() == 0;
}

enum COLLISION_SPACE_INDEX { COLLISION_SPACE_LEFT = 0, COLLISION_SPACE_RIGHT = 1, COLLISION_SPACE_DOWN = 3 };
std::vector<bool> getCollisionSpaces(std::vector<HitObject>& hitpoints, glm::quat rotationWithoutY);

struct MovementCollisions {
  std::vector<bool> movementCollisions;
  std::vector<objid> allCollisions;
};

struct CollisionSpace {
  glm::vec3 direction;
  float comparison;
};
std::vector<CollisionSpace> collisionSpaces = {
  CollisionSpace {   // left
    .direction = glm::vec3(1.f, 0.f, 0.f),
    .comparison = 0.9f,
  },
  CollisionSpace {  // right
    .direction = glm::vec3(-1.f, 0.f, 0.f),
    .comparison = 0.9f,
  },
  CollisionSpace {  // up
    .direction = glm::vec3(0.f, -1.f, 0.f),
    .comparison = 0.9f,
  },
  CollisionSpace {  // down
    .direction = glm::vec3(0.f, 1.f, 0.f),
    .comparison = 0.9f,
  },
  CollisionSpace {  // backward
    .direction = glm::vec3(0.f, 0.f, 1.f),
    .comparison = 0.9f,
  },
  CollisionSpace {  // forward
    .direction = glm::vec3(0.f, 0.f, -1.f),
    .comparison = 0.9f,
  },
};

bool checkCollision(HitObject& hitpoint, CollisionSpace& collisionSpace, glm::quat rotationWithoutY){
  //     gameapi -> drawLine(hitpoint.point,  hitpoint.point + normal, true, movement.playerId, std::nullopt, std::nullopt, std::nullopt);
  auto direction = glm::normalize(hitpoint.normal * glm::vec3(0.f, 0.f, -1.f));
  auto checkAgainstDirection = rotationWithoutY * collisionSpace.direction;
  float value = glm::dot(direction, checkAgainstDirection);
  //std::cout << "value is: " << value << std::endl;
  return value >= collisionSpace.comparison;
}

std::vector<bool> getCollisionSpaces(std::vector<HitObject>& hitpoints, glm::quat rotationWithoutY){
  std::vector<bool> values;
  for (auto &collisionSpace : collisionSpaces){
    bool inCollisionSpace = false;
    for (auto &hitpoint : hitpoints){
      if (checkCollision(hitpoint, collisionSpace, rotationWithoutY)){
        inCollisionSpace = true;
        break;
      }
    }
    values.push_back(inCollisionSpace);
  }
  return values;
}


MovementCollisions checkMovementCollisions(objid playerId, std::vector<glm::quat>& _hitDirections, glm::quat rotationWithoutY){
  auto hitpoints = gameapi -> contactTest(playerId);
  //std::cout << "hitpoints: [ ";

  //for (auto &hitpoint : hitpoints){
  //  std::cout << "\n[ id = " << hitpoint.id << ", pos = " << print(hitpoint.point) << ", normal = " << serializeQuat(hitpoint.normal) << " ] " << std::endl;
  //  auto normal = 10.f * glm::normalize(hitpoint.normal * glm::vec3(0.f, 0.f, -1.f));
  //  gameapi -> drawLine(hitpoint.point,  hitpoint.point + normal, true, movement.playerId, std::nullopt, std::nullopt, std::nullopt);
  //}

  std::vector<objid> allCollisions;
  for (auto &hitpoint : hitpoints){
    if (isCollideable(hitpoint.id)){
      _hitDirections.push_back(hitpoint.normal);
      allCollisions.push_back(hitpoint.id);
    }
  }

  auto collisions = getCollisionSpaces(hitpoints, rotationWithoutY);
  //std::cout << "collisions: " << print(collisions) << std::endl;

  //std::cout << " ]" << std::endl;
  return MovementCollisions {
    .movementCollisions = collisions,
    .allCollisions = allCollisions,
  };
}

glm::vec3 limitMoveDirectionFromCollisions(glm::vec3 moveVec, std::vector<glm::quat>& hitDirections, glm::quat playerDirection){
  auto directionVec = playerDirection * moveVec; 

  for (auto &hitDirection : hitDirections){
    auto playerDirectionFromWall = glm::inverse(hitDirection) * directionVec;
    auto playerDirectionFromWallNoZ = playerDirectionFromWall;
    if (playerDirectionFromWallNoZ.z > 0){ // cannot move into the wall
      playerDirectionFromWallNoZ.z = 0.f;
    }
    auto playerDirectionWorld = hitDirection * playerDirectionFromWallNoZ;
    //std::cout << "player abs direction: " << print(directionVec) << std::endl;
    //std::cout << "player direction relative to wall: " << print(playerDirectionFromWall) << std::endl;
    //std::cout << "player direction relative to wall target (no +z): " << print(playerDirectionFromWallNoZ) << std::endl;
    //std::cout << "player direction world (no +z): " << print(playerDirectionWorld) << std::endl;
    directionVec = playerDirectionWorld;
  }  

  auto relativeToPlayer = glm::inverse(playerDirection) * directionVec;
  return relativeToPlayer;
}

void maybeToggleCrouch(MovementParams& moveParams, MovementState& movementState, bool crouchDown){
  auto timeSinceLastCrouch = (gameapi -> timeSeconds(false) - movementState.lastCrouchTime) * 1000;
  if (crouchDown && moveParams.canCrouch && (timeSinceLastCrouch > moveParams.crouchDelay)){
    movementState.shouldBeCrouching = true;
  }else {
    movementState.shouldBeCrouching = false;
  }
}

// This is obviously wrong, but a starting point
MovementControlData getMovementControlDataFromTargetPos(glm::vec3 targetPosition, float speed, MovementState& movementState, objid playerId, bool* atTargetPos){
  MovementControlData controlData {
    .moveVec = glm::vec3(0.f, 0.f, 0.f),
    .isWalking = false,
    .doJump = false,
    .doAttachToLadder = false,
    .doReleaseFromLadder = false,
    .crouchType = CROUCH_NONE,
    .raw_deltax = 0.f,
    .raw_deltay = 0.f,
    .zoom_delta = 0.f,
    .speed = speed,
  };
  if (atTargetPos){
    *atTargetPos = false;
  }

  auto playerDirection = gameapi -> getGameObjectRotation(playerId, true);
  glm::vec3 positionDiff = glm::vec3(targetPosition.x, targetPosition.y, targetPosition.z) - glm::vec3(movementState.lastPosition.x, movementState.lastPosition.y, movementState.lastPosition.z);
  positionDiff = glm::inverse(playerDirection) * positionDiff;

  controlData.moveVec = glm::vec3(positionDiff.x, 0.f, positionDiff.z);
  auto moveLength = glm::length(controlData.moveVec);

  if (moveLength < 0.5){  // already arrived
    if (atTargetPos){
      *atTargetPos = true;
    }
    return controlData;
  }

  if (moveLength){
    controlData.moveVec = glm::normalize(controlData.moveVec);
  }

  controlData.isWalking = true;

  modlog("movement movevec", std::string("last pos: ") + print(movementState.lastPosition) + ", target = " + print(targetPosition) + ", movVec = " +  print(controlData.moveVec));
  return controlData;
}

MovementControlData getMovementControlData(ControlParams& controlParams, MovementState& movementState, MovementParams& moveParams){
  MovementControlData controlData {
    .moveVec = glm::vec3(0.f, 0.f, 0.f),
    .isWalking = false,
    .doJump = controlParams.doJump,
    .doAttachToLadder = controlParams.doAttachToLadder,
    .doReleaseFromLadder = controlParams.doReleaseFromLadder,
    .crouchType = controlParams.crouchType,
    .raw_deltax = controlParams.lookVelocity.x * controlParams.xsensitivity,
    .raw_deltay = -1.f * controlParams.lookVelocity.y * controlParams.ysensitivity,
    .zoom_delta = controlParams.zoom_delta,
    .speed = 1.f,
  };

  static float horzRelVelocity = 0.8f;

  if (controlParams.goForward){
    std::cout << "should move forward" << std::endl;
    if (controlParams.shiftModifier && moveParams.moveVertical){
      controlData.moveVec += glm::vec3(0.f, 1.f, 0.f);
    }else{
      controlData.moveVec += glm::vec3(0.f, 0.f, -1.f);
    }
    if (!(movementState.facingLadder || movementState.attachedToLadder)){
      controlData.isWalking = true;
    }
  }
  if (controlParams.goBackward){
    if (controlParams.shiftModifier && moveParams.moveVertical){
      controlData.moveVec += glm::vec3(0.f, -1.f, 0.f);
    }else{
      controlData.moveVec += glm::vec3(0.f, 0.f, 1.f);
    }
    if (!(movementState.facingLadder || movementState.attachedToLadder)){
      controlData.isWalking = true;
    }
  }
  if (controlParams.goLeft){
    controlData.moveVec += glm::vec3(horzRelVelocity * -1.f, 0.f, 0.f);
    if (!movementState.attachedToLadder){
      controlData.isWalking = true;
    }
    
  }
  if (controlParams.goRight){
    controlData.moveVec += glm::vec3(horzRelVelocity * 1.f, 0.f, 0.f);
    if (!movementState.attachedToLadder){
      controlData.isWalking = true;
    }
  }

  return controlData;
}


void onMovementFrameControl(MovementParams& moveParams, MovementState& movementState, objid playerId, MovementControlData& controlData, std::optional<ThirdPersonCameraInfo>& managedCamera, bool isGunZoomed){
  std::vector<glm::quat> hitDirections;

    //std::vector<glm::quat> hitDirections;
  auto playerDirection = gameapi -> getGameObjectRotation(playerId, true);
  auto directionVec = playerDirection * glm::vec3(0.f, 0.f, -1.f); 
  directionVec.y = 0.f;
  auto rotationWithoutY = quatFromDirection(directionVec);

  movementState.lastFrameIsGrounded = movementState.isGrounded;

  auto collisions = checkMovementCollisions(playerId, hitDirections, rotationWithoutY);
  bool isGrounded = collisions.movementCollisions.at(COLLISION_SPACE_DOWN);
   
  movementState.inWater = false;
  for (auto id : collisions.allCollisions){
    auto isWater = getSingleAttr(id, "water").has_value();
    if (isWater){
      movementState.inWater = true;
      break;
    }
  }
  movementState.isGrounded = isGrounded && !movementState.inWater; 

  if (movementState.isGrounded && !movementState.lastFrameIsGrounded){
    modlog("animation controller", "land");
    gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
      .entityId = playerId,
      .transition = "land",
    });
    land(playerId);
  }

  

  float moveSpeed = getMoveSpeed(moveParams, movementState, false, isGrounded) * controlData.speed;
  //modlog("editor: move speed: ", std::to_string(moveSpeed) + ", is grounded = " + print(isGrounded));
  auto limitedMoveVec = limitMoveDirectionFromCollisions(glm::vec3(controlData.moveVec.x, controlData.moveVec.y, controlData.moveVec.z), hitDirections, rotationWithoutY);
  //auto limitedMoveVec = moveVec;
  if (!moveParams.moveVertical){
    limitedMoveVec.y = 0.f;
  }
  auto direction = glm::vec3(limitedMoveVec.x, limitedMoveVec.y, limitedMoveVec.z);

  if (controlData.isWalking){
    std::cout << "movement, direction = : " << print(direction) << std::endl;
    moveAbsolute(playerId, rotationWithoutY * (moveSpeed * direction));

    bool isSideStepping = glm::abs(controlData.moveVec.x) > glm::abs(controlData.moveVec.z);
    if (isSideStepping){
      gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
        .entityId = playerId,
        .transition = "sidestep",
      });
    }else{
      gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
        .entityId = playerId,
        .transition = "walking",
      });
    }
  }else if (movementState.facingLadder || movementState.attachedToLadder  /* climbing ladder */ ){
    moveUp(playerId, controlData.moveVec);
    gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
      .entityId = playerId,
      .transition = "not-walking",
    });
  }else{
    gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
      .entityId = playerId,
      .transition = "not-walking",
    });
  }


  auto currPos = gameapi -> getGameObjectPos(playerId, true);
  auto currTime = gameapi -> timeSeconds(false);

  if (glm::length(currPos - movementState.lastMoveSoundPlayLocation) > moveParams.moveSoundDistance && isGrounded && getManagedSounds().moveSoundObjId.has_value() && ((currTime - movementState.lastMoveSoundPlayTime) > moveParams.moveSoundMintime)){
    // move-sound-distance:STRING move-sound-mintime:STRING
    std::cout << "should play move clip" << std::endl;
    playGameplayClipById(getManagedSounds().moveSoundObjId.value(), std::nullopt, std::nullopt);
    movementState.lastMoveSoundPlayTime = currTime;
    movementState.lastMoveSoundPlayLocation = currPos;
  }
    
  float elapsedTime = gameapi -> timeElapsed();

  if (managedCamera.has_value()){
    lookThirdPerson(moveParams, movementState, controlData.raw_deltax, controlData.raw_deltay, controlData.zoom_delta, playerId, managedCamera, elapsedTime, isGunZoomed);
  }else{
    look(moveParams, movementState, playerId, elapsedTime, false, 0.5f, controlData.raw_deltax, controlData.raw_deltay);
  }


  bool movingDown = false;
  updateVelocity(movementState, playerId, elapsedTime, currPos, &movingDown);

  updateFacingWall(movementState, playerId);
  restrictLadderMovement(movementState, playerId, movingDown);
  updateCrouch(moveParams, movementState, playerId);

  auto shouldStep = shouldStepUp(playerId) && (direction.z < 0 /* going forward */ );
  //std::cout << "should step up: " << shouldStep << std::endl;
  if (shouldStep){
    gameapi -> applyImpulse(playerId, glm::vec3(0.f, 0.4f, 0.f));
  }
}

void onMovementFrame(MovementParams& moveParams, MovementState& movementState, objid playerId, MovementControlData& controlData, std::optional<ThirdPersonCameraInfo>& managedCamera, bool isGunZoomed){
  onMovementFrameControl(moveParams, movementState, playerId, controlData, managedCamera, isGunZoomed);
  if (controlData.doJump){
    jump(moveParams, movementState, playerId);      
  }
  if (controlData.doAttachToLadder){
    attachToLadder(movementState);
  }
  if (controlData.doReleaseFromLadder){
    releaseFromLadder(movementState);       
  }
  if (controlData.crouchType != CROUCH_NONE){
    if (controlData.crouchType == CROUCH_DOWN){
      maybeToggleCrouch(moveParams, movementState, true);
    }else if (controlData.crouchType == CROUCH_UP){
      maybeToggleCrouch(moveParams, movementState, false);
    }
  }
}

glm::vec2 pitchXAndYawYRadians(glm::quat currRotation){
  glm::vec3 euler_angles = glm::eulerAngles(currRotation);
  auto forwardVec = currRotation * glm::vec3(0.f, 0.f, -1.f);
  auto angleX = glm::atan(forwardVec.x / forwardVec.z);
  angleX *= -1;
  if (forwardVec.x > 0 && forwardVec.z > 0){
    angleX = angleX + 3.1418;
  }else if (forwardVec.x < 0 && forwardVec.z > 0){
    angleX = angleX - 3.1418;
  }
  auto angleY = -1 * euler_angles.x;
  if (forwardVec.z > 0){
    angleY = angleY - MODPI;
  }
  return glm::vec2(angleX, angleY);
}
MovementState getInitialMovementState(objid playerId){
  MovementState movementState {};
  movementState.lastMoveSoundPlayTime = 0.f;
  movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);

  movementState.lastPosition = glm::vec3(0.f, 0.f, 0.f);

  auto oldXYRot = pitchXAndYawYRadians(gameapi -> getGameObjectRotation(playerId, true));
  movementState.xRot = oldXYRot.x;
  movementState.yRot = oldXYRot.y;    
  

  movementState.isGrounded = false;
  movementState.lastFrameIsGrounded = false;
  movementState.facingWall = false;
  movementState.facingLadder = false;
  movementState.attachedToLadder = false;

  movementState.inWater = false;
  movementState.isCrouching = false;
  movementState.shouldBeCrouching = false;
  movementState.lastCrouchTime = -10000.f;  // so can immediately crouch

  auto scale = gameapi -> getGameObjectScale(playerId, false);
  movementState.initialScale = scale;

  return movementState;
}

std::string movementToStr(ControlParams& controlParams){
  std::string str;
  str += std::string("goForward: ") + (controlParams.goForward ? "true" : "false") + "\n";
  str += std::string("goBackward: ") + (controlParams.goBackward ? "true" : "false") + "\n";
  str += std::string("goLeft: ") + (controlParams.goLeft ? "true" : "false") + "\n";
  str += std::string("goRight: ") + (controlParams.goRight ? "true" : "false") + "\n";
  str += std::string("shiftModifier: ") + (controlParams.shiftModifier ? "true" : "false") + "\n";
  return str;
}