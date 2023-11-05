#include "./movementcore.h"

extern CustomApiBindings* gameapi;

void jump(MovementParams& moveParams, MovementState& movementState, objid id){
  glm::vec3 impulse(0, moveParams.jumpHeight, 0);
  if (movementState.isGrounded){
    gameapi -> applyImpulse(id, impulse);
    if (getManagedSounds().jumpSoundObjId.has_value()){
      gameapi -> playClipById(getManagedSounds().jumpSoundObjId.value(), std::nullopt, std::nullopt);
    }
  }
  if (movementState.inWater){
    gameapi -> applyImpulse(id, impulse);
  }
}

void land(objid id){
  if (getManagedSounds().landSoundObjId.has_value()){
    gameapi -> playClipById(getManagedSounds().landSoundObjId.value(), std::nullopt, std::nullopt);
  }
}

void moveUp(objid id, glm::vec2 direction){
  float time = gameapi -> timeElapsed();
  gameapi -> applyImpulse(id, time * glm::vec3(0.f, -direction.y, 0.f));
}
void moveDown(objid id, glm::vec2 direction){
  float time = gameapi -> timeElapsed();
  gameapi -> applyImpulse(id, time * glm::vec3(0.f, -direction.y, 0.f));
}

void moveXZ(objid id, glm::vec2 direction){ // i wonder if i should make this actually parellel to the surface the player is moving along
  //modlog("editor: move xz: ", print(direction));
  float time = gameapi -> timeElapsed();
  auto playerRotation = gameapi -> getGameObjectRotation(id, true);
  auto directionVec = playerRotation * glm::vec3(direction.x, 0.f, direction.y);
  auto magnitude = glm::length(directionVec);
  if (aboutEqual(magnitude, 0.f)){
    //modassert(false, "magnitude was about zero");
    modlog("movement", "warning magnitude was about zero");
    return;
  }
  directionVec.y = 0.f;
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
  gameapi -> sendNotifyMessage("velocity", serializeVec(displacement));
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
    auto attr = gameapi -> getGameObjectAttr(hitpoint.id);
    movementState.facingLadder = getStrAttr(attr, "ladder").has_value();
  }else{
    movementState.facingWall = false;
    movementState.facingLadder = false;
  }
}

void restrictLadderMovement(MovementState& movementState, objid id, bool movingDown){
  if (movementState.attachedToLadder){
    auto attr = gameapi -> getGameObjectAttr(id);
    auto velocity = getVec3Attr(attr, "physics_velocity").value();
    if (velocity.y > 0.f){
      return;
    }
    velocity.y = 0.f;

    GameobjAttributes newAttr {
      .stringAttributes = {},
      .numAttributes = {},
      .vecAttr = { 
        .vec3 = { { "physics_velocity", velocity }}, 
        .vec4 = { } 
      },
    };
    gameapi -> setGameObjectAttr(id, newAttr);
  }
}

void look(MovementParams& moveParams, MovementState& movementState, objid id, float elapsedTime, bool ironsight, float ironsight_turn, glm::vec2 lookVelocity, ControlParams& controlParams){
  auto forwardVec = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));

  float raw_deltax = lookVelocity.x * controlParams.xsensitivity * elapsedTime;
  float raw_deltay = -1.f * lookVelocity.y * controlParams.ysensitivity * elapsedTime; 

  float deltax = ironsight ? (raw_deltax * ironsight_turn) : raw_deltax;
  float deltay = ironsight ? (raw_deltay * ironsight_turn) : raw_deltay;

  movementState.xRot = limitAngle(movementState.xRot + deltax, std::nullopt, std::nullopt);
  movementState.yRot = limitAngle(movementState.yRot + deltay, moveParams.maxAngleUp, moveParams.maxAngleDown); 
  auto rotation = gameapi -> setFrontDelta(forwardVec, movementState.xRot, movementState.yRot, 0, 1.f);
  gameapi -> setGameObjectRot(id, rotation, false);
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
  auto scale = shouldCrouch ? glm::vec3(crouchScale, crouchScale, crouchScale) : glm::vec3(1.f, 1.f, 1.f);
  GameobjAttributes newAttr {
    .stringAttributes = {},
    .numAttributes = {
      { "physics_friction", shouldCrouch ? moveParams.crouchFriction : moveParams.friction },
    },
    .vecAttr = { 
      .vec3 = {},
      .vec4 = {} 
    },
  };
  gameapi -> setGameObjectAttr(id, newAttr);
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


bool shouldStepUp(objid id){
  auto playerPos = gameapi -> getGameObjectPos(id, true);
  auto inFrontOfPlayer = gameapi -> getGameObjectRotation(id, true) * glm::vec3(0.f, 0.f, -1.f);
  auto inFrontOfPlayerSameHeight = playerPos + glm::vec3(inFrontOfPlayer.x, 0.f, inFrontOfPlayer.z);
  
  auto belowPos = playerPos - glm::vec3(0.f, 0.95f, 0.f);
  auto belowPosSameHeight = inFrontOfPlayerSameHeight - glm::vec3(0.f, 0.95f, 0.f);
  
  auto abovePos = playerPos - glm::vec3(0.f, 0.2f, 0.f);
  auto abovePosSameHeight = inFrontOfPlayerSameHeight - glm::vec3(0.f, 0.2f, 0.f);

  auto belowDir = gameapi -> orientationFromPos(belowPos, belowPosSameHeight);
  auto aboveDir = gameapi -> orientationFromPos(abovePos, abovePosSameHeight);

  auto belowHitpoints = gameapi -> raycast(belowPos, belowDir, 2.f);
  auto aboveHitpoints = gameapi -> raycast(abovePos, aboveDir, 2.f);

  //gameapi -> drawLine(belowPos, gameapi -> moveRelative(belowPos, belowDir, 2.f), true, id, glm::vec4(1.f, 0.f, 0.f, 1.f),  std::nullopt, std::nullopt);
  //gameapi -> drawLine(abovePos, gameapi -> moveRelative(abovePos, aboveDir, 2.f), true, id, glm::vec4(0.f, 0.f, 1.f, 1.f),  std::nullopt, std::nullopt);

  //std::cout << "hitpoints:  low = " << belowHitpoints.size() << ", high = " << aboveHitpoints.size() << std::endl;
  return belowHitpoints.size() > 0 && aboveHitpoints.size() == 0;
}

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

MovementCollisions checkMovementCollisions(objid playerId, std::vector<glm::quat>& hitDirections, glm::quat rotationWithoutY){
  auto hitpoints = gameapi -> contactTest(playerId);
  //std::cout << "hitpoints: [ ";

  //for (auto &hitpoint : hitpoints){
  //  std::cout << "\n[ id = " << hitpoint.id << ", pos = " << print(hitpoint.point) << ", normal = " << serializeQuat(hitpoint.normal) << " ] " << std::endl;
  //  auto normal = 10.f * glm::normalize(hitpoint.normal * glm::vec3(0.f, 0.f, -1.f));
  //  gameapi -> drawLine(hitpoint.point,  hitpoint.point + normal, true, movement.playerId, std::nullopt, std::nullopt, std::nullopt);
  //}

  std::vector<objid> allCollisions;
  for (auto &hitpoint : hitpoints){
    hitDirections.push_back(hitpoint.normal);
    allCollisions.push_back(hitpoint.id);
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