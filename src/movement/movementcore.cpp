#include "./movementcore.h"

extern CustomApiBindings* gameapi;
void doAnimationTrigger(objid id, const char* transition);

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
  doAnimationTrigger(id, "jump");
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
  movementState.velocity = displacement;
  modlog("update velocity", print(movementState.velocity));

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

struct LookParams {
  float elapsedTime;
  bool ironsight;
  float ironsightTurn;
  float turnX;
  float turnY;
};


glm::quat weaponLookDirection(MovementState& movementState){
  auto forwardVec = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
  auto rotation = gameapi -> setFrontDelta(forwardVec, movementState.xRot, movementState.yRot, 0, 1.f);
  return rotation;
}
FirstPersonCameraUpdate look(MovementParams& moveParams, MovementState& movementState, LookParams& lookParams){
  auto forwardVec = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));

  float raw_deltax = lookParams.turnX * lookParams.elapsedTime;
  float raw_deltay = lookParams.turnY * lookParams.elapsedTime; 

  float deltax = lookParams.ironsight ? (raw_deltax * lookParams.ironsightTurn) : raw_deltax;
  float deltay = lookParams.ironsight ? (raw_deltay * lookParams.ironsightTurn) : raw_deltay;

  movementState.xRot = limitAngle(movementState.xRot + deltax, std::nullopt, std::nullopt);
  movementState.yRot = limitAngle(movementState.yRot + deltay, moveParams.maxAngleUp, moveParams.maxAngleDown); 
  auto rotation = weaponLookDirection(movementState);
  auto rotation1 = gameapi -> setFrontDelta(forwardVec, movementState.xRot, 0, 0, 1.f);

  return FirstPersonCameraUpdate {
      .rotation = rotation,
      .yAxisRotation = rotation1,
  };
}


// this code should use the main look logic, and then manage the camera for now separate codepaths but shouldn't be, probably 
ThirdPersonCameraUpdate lookThirdPerson(MovementParams& moveParams, MovementState& movementState, LookParams& lookParams, float zoom_delta, ThirdPersonCameraInfo& thirdPersonInfo){
  thirdPersonInfo.angleX += lookParams.turnX * 0.1 /* 0.05f arbitary turn speed */;
  thirdPersonInfo.angleY += lookParams.turnY * 0.1 /* 0.05f arbitary turn speed */;
  thirdPersonInfo.distanceFromTarget += zoom_delta;

  float reverseMultiplier = thirdPersonInfo.reverseCamera ? -1.f : 1.f;

  auto finalAdjustedDistance = lookParams.ironsight ? thirdPersonInfo.zoomOffset.z :  thirdPersonInfo.distanceFromTarget;
  float zoomSpeed = lookParams.ironsight ? 10.f : 2.f;

  thirdPersonInfo.actualDistanceFromTarget = glm::lerp(thirdPersonInfo.actualDistanceFromTarget, finalAdjustedDistance, glm::clamp(lookParams.elapsedTime * zoomSpeed, 0.f, 1.f));
  thirdPersonInfo.actualZoomOffset.x = glm::lerp(thirdPersonInfo.actualZoomOffset.x, lookParams.ironsight ? (thirdPersonInfo.zoomOffset.x * reverseMultiplier) : 0.f, glm::clamp(lookParams.elapsedTime * zoomSpeed, 0.f, 1.f));
  thirdPersonInfo.actualZoomOffset.y = glm::lerp(thirdPersonInfo.actualZoomOffset.y, lookParams.ironsight ? thirdPersonInfo.zoomOffset.y : 0.f, glm::clamp(lookParams.elapsedTime * zoomSpeed, 0.f, 1.f));


  float x = glm::cos(thirdPersonInfo.angleX) * thirdPersonInfo.actualDistanceFromTarget;
  float z = glm::sin(thirdPersonInfo.angleX) * thirdPersonInfo.actualDistanceFromTarget;
  float y = glm::cos(thirdPersonInfo.angleY) * thirdPersonInfo.actualDistanceFromTarget;

  auto targetLocation = movementState.lastPosition;
  auto fromLocation = targetLocation + glm::vec3(x, -y, z);
  auto newOrientation = orientationFromPos(fromLocation, targetLocation);

  auto fromLocationYOnly = targetLocation + glm::vec3(x, 0.f, z);
  auto newOrientationNoX = orientationFromPos(fromLocationYOnly, targetLocation);  // maybe should clear the y

  auto finalCameraLocation = fromLocation + (newOrientation * glm::vec3(reverseMultiplier * thirdPersonInfo.additionalCameraOffset.x, thirdPersonInfo.additionalCameraOffset.y, thirdPersonInfo.additionalCameraOffset.z)) + (newOrientation * glm::vec3(thirdPersonInfo.actualZoomOffset.x, thirdPersonInfo.actualZoomOffset.y, 0.f));
  // should encode the same logic as look probably, and this also shouldn't be exactly the same as the camera.  Don't set upward rotation
    
  ThirdPersonCameraUpdate cameraUpdate {
    .position = finalCameraLocation,
    .rotation = newOrientation,
    .yAxisRotation = newOrientationNoX,
  };
  return cameraUpdate;
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
glm::vec3 getMovementControlDataFromTargetPos(glm::vec3 targetPosition, MovementState& movementState, objid playerId, bool* atTargetPos){
  glm::vec3 moveVec(0.f, 0.f, 0.f);

  auto playerDirection = gameapi -> getGameObjectRotation(playerId, true);
  glm::vec3 positionDiff = glm::vec3(targetPosition.x, targetPosition.y, targetPosition.z) - glm::vec3(movementState.lastPosition.x, movementState.lastPosition.y, movementState.lastPosition.z);
  positionDiff = glm::inverse(playerDirection) * positionDiff;

  moveVec = glm::vec3(positionDiff.x, 0.f, positionDiff.z);
  auto moveLength = glm::length(moveVec);

  if (atTargetPos){
    *atTargetPos = false;
  }
  if (moveLength < 0.5){  // already arrived
    if (atTargetPos){
      *atTargetPos = true;
    }
  }

  if (moveLength){
    moveVec = glm::normalize(moveVec);
  }

  if (*atTargetPos){
    moveVec = glm::vec3(0.f, 0.f, 0.f);
  }

  modlog("movement movevec", std::string("last pos: ") + print(movementState.lastPosition) + ", target = " + print(targetPosition) + ", movVec = " +  print(moveVec));
  return moveVec;
}

bool calcIfWalking(MovementState& movementState){
  if (movementState.facingLadder || movementState.attachedToLadder){
    return false;
  }
  return glm::abs(movementState.moveVec.x) > 0.0001 || glm::abs(movementState.moveVec.z) > 0.0001;
}

CameraUpdate onMovementFrameCore(MovementParams& moveParams, MovementState& movementState, objid playerId, ThirdPersonCameraInfo& managedCamera, bool isGunZoomed){
  auto currTime = gameapi -> timeSeconds(false);
  float elapsedTime = gameapi -> timeElapsed();

  auto currPos = gameapi -> getGameObjectPos(playerId, true);
  auto playerDirection = gameapi -> getGameObjectRotation(playerId, true);

  auto directionVec = playerDirection * glm::vec3(0.f, 0.f, -1.f); 
  directionVec.y = 0.f;
  auto rotationWithoutY = quatFromDirection(directionVec);

  movementState.lastFrameIsGrounded = movementState.isGrounded;

  std::vector<glm::quat> hitDirections;
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
    doAnimationTrigger(playerId, "land");
    land(playerId);
  }

  

  float moveSpeed = getMoveSpeed(moveParams, movementState, false, isGrounded) * movementState.speed;
  //modlog("editor: move speed: ", std::to_string(moveSpeed) + ", is grounded = " + print(isGrounded));
  auto limitedMoveVec = limitMoveDirectionFromCollisions(glm::vec3(movementState.moveVec.x, movementState.moveVec.y, movementState.moveVec.z), hitDirections, rotationWithoutY);
  //auto limitedMoveVec = moveVec;
  if (!moveParams.moveVertical){
    limitedMoveVec.y = 0.f;
  }
  auto direction = glm::vec3(limitedMoveVec.x, limitedMoveVec.y, limitedMoveVec.z);
  auto isWalking = calcIfWalking(movementState);

  if (isWalking){
    std::cout << "movement, direction = : " << print(direction) << std::endl;
    moveAbsolute(playerId, rotationWithoutY * (moveSpeed * direction));
    bool isSideStepping = glm::abs(movementState.moveVec.x) > glm::abs(movementState.moveVec.z);
    doAnimationTrigger(playerId, isSideStepping ? "sidestep" : "walking");
  }else if (movementState.facingLadder || movementState.attachedToLadder  /* climbing ladder */ ){
    moveUp(playerId, movementState.moveVec);
    doAnimationTrigger(playerId, "not-walking");
  }else{
    doAnimationTrigger(playerId, "not-walking");
  }


  if (glm::length(currPos - movementState.lastMoveSoundPlayLocation) > moveParams.moveSoundDistance && isGrounded && getManagedSounds().moveSoundObjId.has_value() && ((currTime - movementState.lastMoveSoundPlayTime) > moveParams.moveSoundMintime)){
    // move-sound-distance:STRING move-sound-mintime:STRING
    std::cout << "should play move clip" << std::endl;
    playGameplayClipById(getManagedSounds().moveSoundObjId.value(), std::nullopt, std::nullopt);
    movementState.lastMoveSoundPlayTime = currTime;
    movementState.lastMoveSoundPlayLocation = currPos;
  }
    
  LookParams lookParams {
    .elapsedTime = elapsedTime,
    .ironsight = isGunZoomed,
    .ironsightTurn = 0.5f,
    .turnX = movementState.raw_deltax,
    .turnY = movementState.raw_deltay,
  };
  
  CameraUpdate cameraUpdate { .thirdPerson = std::nullopt };
  cameraUpdate.firstPerson = look(moveParams, movementState, lookParams);
  if (managedCamera.thirdPersonMode){
    auto thirdPersonCameraUpdate = lookThirdPerson(moveParams, movementState, lookParams, movementState.zoom_delta, managedCamera);
    cameraUpdate.thirdPerson = thirdPersonCameraUpdate;
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

  if (movementState.doJump){
    jump(moveParams, movementState, playerId);      
  }
  if (movementState.doAttachToLadder){
    if (movementState.facingLadder){
      movementState.attachedToLadder = true;
    }
  }
  if (movementState.doReleaseFromLadder){
    movementState.attachedToLadder = false;
  }
  if (movementState.crouchType != CROUCH_NONE){
    if (movementState.crouchType == CROUCH_DOWN){
      maybeToggleCrouch(moveParams, movementState, true);
    }else if (movementState.crouchType == CROUCH_UP){
      maybeToggleCrouch(moveParams, movementState, false);
    }
  }
  return cameraUpdate;
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

  movementState.moveVec = glm::vec3(0.f, 0.f, 0.f);
  movementState.speed = 1.f;
  movementState.zoom_delta = 0.f;
  movementState.doAttachToLadder = false;
  movementState.doReleaseFromLadder = false;
  movementState.raw_deltax = 0.f;
  movementState.raw_deltay = 0.f;
  movementState.crouchType = CROUCH_NONE;

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
  movementState.velocity = glm::vec3(0.f, 0.f, 0.f);

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