#include "./movement.h"


// For parity in behavior of all scripts need to restore 
//    ironsight mode
//    dash ability

extern CustomApiBindings* gameapi;

//////   resource manager       /////////////////////////////////////////////


///////////////////////////////////////////////////

struct MovementParams {
  float moveSpeed;
  float moveSpeedAir;
  float moveSpeedWater;
  float jumpHeight;
  float maxAngleUp;
  float maxAngleDown;
  float moveSoundDistance;
  float moveSoundMintime;
  float groundAngle;
  glm::vec3 gravity;
  bool canCrouch;
  float crouchSpeed;
  float crouchScale;
  float crouchDelay;
  float friction;
  float crouchFriction;
};

struct ControlParams {
  float xsensitivity;
  float ysensitivity;
};

struct Movement {
  MovementParams moveParams; // character params

  ControlParams controlParams; // controls

  std::optional<objid> playerId;   // target id 

  bool goForward;                 // control params
  bool goBackward;
  bool goLeft;
  bool goRight;
  bool active;

  float lastMoveSoundPlayTime;      // state
  glm::vec3 lastMoveSoundPlayLocation;

  glm::vec2 lookVelocity;       // control params
  float xRot;               
  float yRot;
  bool facingWall;              // state
  bool facingLadder;
  bool attachedToLadder;    
  std::set<objid> waterObjIds;  

  bool isGrounded;              // state
  bool lastFrameIsGrounded;
  bool isCrouching;
  bool shouldBeCrouching;
  float lastCrouchTime;

  glm::vec3 lastPosition;
};

std::string movementToStr(Movement& movement){
  std::string str;
  str += std::string("goForward: ") + (movement.goForward ? "true" : "false") + "\n";
  str += std::string("goBackward: ") + (movement.goBackward ? "true" : "false") + "\n";
  str += std::string("goLeft: ") + (movement.goLeft ? "true" : "false") + "\n";
  str += std::string("goRight: ") + (movement.goRight ? "true" : "false") + "\n";
  return str;
}

void jump(Movement& movement, objid id){
  glm::vec3 impulse(0, movement.moveParams.jumpHeight, 0);
  if (movement.isGrounded){
    gameapi -> applyImpulse(id, impulse);
    if (getManagedSounds().jumpSoundObjId.has_value()){
      gameapi -> playClipById(getManagedSounds().jumpSoundObjId.value(), std::nullopt, std::nullopt);
    }
  }
  if (movement.waterObjIds.size() > 0){
    gameapi -> applyImpulse(id, impulse);
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

void moveXZ(objid id, glm::vec2 direction){
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
float getMoveSpeed(Movement& movement, bool ironsight, bool isGrounded){
  auto inWater = movement.waterObjIds.size() > 0;
  auto baseMoveSpeed = movement.isCrouching ? movement.moveParams.crouchSpeed : movement.moveParams.moveSpeed;
  auto speed = isGrounded ? baseMoveSpeed : movement.moveParams.moveSpeedAir;
  speed = inWater ? movement.moveParams.moveSpeedWater : speed;
  if (ironsight){
    speed = speed * ironsightSpeedMultiplier;
  }
  return speed;
}

void updateVelocity(Movement& movement, objid id, float elapsedTime, glm::vec3 currPos, bool* _movingDown){ // returns if moveing down
  glm::vec3 displacement = (currPos - movement.lastPosition) / elapsedTime;
  //auto speed = glm::length(displacement);
  movement.lastPosition = currPos;
  //std::cout << "velocity = " << print(displacement) << ", speed = " << speed << std::endl;
  gameapi -> sendNotifyMessage("velocity", serializeVec(displacement));
  *_movingDown = displacement.y < 0.f;
}

void updateFacingWall(Movement& movement, objid id){
  auto mainobjPos = gameapi -> getGameObjectPos(id, true);
  auto rot = gameapi -> getGameObjectRotation(id, true);
  //  (define shotangle (if (should-zoom) rot (with-bloom rot)))
  auto hitpoints =  gameapi -> raycast(mainobjPos, rot, 2.f);

  if (hitpoints.size() > 0){
    movement.facingWall = true;
    auto hitpoint = hitpoints.at(closestHitpoint(hitpoints, mainobjPos));
    auto attr = gameapi -> getGameObjectAttr(hitpoint.id);
    movement.facingLadder = getStrAttr(attr, "ladder").has_value();
  }else{
    movement.facingWall = false;
    movement.facingLadder = false;
  }
}

void restrictLadderMovement(Movement& movement, objid id, bool movingDown){
  if (movement.attachedToLadder){
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



void look(Movement& movement, objid id, float elapsedTime, bool ironsight, float ironsight_turn){
  auto forwardVec = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));

  float raw_deltax = movement.lookVelocity.x * movement.controlParams.xsensitivity * elapsedTime;
  float raw_deltay = -1.f * movement.lookVelocity.y * movement.controlParams.ysensitivity * elapsedTime; 

  float deltax = ironsight ? (raw_deltax * ironsight_turn) : raw_deltax;
  float deltay = ironsight ? (raw_deltay * ironsight_turn) : raw_deltay;

  movement.xRot = limitAngle(movement.xRot + deltax, std::nullopt, std::nullopt);
  movement.yRot = limitAngle(movement.yRot + deltay, movement.moveParams.maxAngleUp, movement.moveParams.maxAngleDown); 
  auto rotation = gameapi -> setFrontDelta(forwardVec, movement.xRot, movement.yRot, 0, 1.f);
  gameapi -> setGameObjectRot(id, rotation, false);

  movement.lookVelocity = glm::vec2(0.f, 0.f);
}

void land(Movement& movement, objid id){
  if (getManagedSounds().landSoundObjId.has_value()){
    gameapi -> playClipById(getManagedSounds().landSoundObjId.value(), std::nullopt, std::nullopt);
  }
}


void updateObjectProperties(objid id, std::vector<std::vector<std::string>>& result, glm::vec3 physics_gravity, float friction){
  float physics_mass = floatFromFirstSqlResult(result, 8);
  float physics_restitution = floatFromFirstSqlResult(result, 4);
  GameobjAttributes attr {
    .stringAttributes = {
    },
    .numAttributes = {
      { "physics_mass", physics_mass },
      { "physics_restitution", physics_restitution },
      { "physics_friction", friction },
    },
    .vecAttr = { 
      .vec3 = {
        { "physics_gravity", physics_gravity },
      }, 
      .vec4 = {} 
    },
  };
  gameapi -> setGameObjectAttr(id, attr);
}

void updateTraitConfig(Movement& movement, std::vector<std::vector<std::string>>& result){
  movement.moveParams.moveSpeed = floatFromFirstSqlResult(result, 0);
  movement.moveParams.moveSpeedAir = floatFromFirstSqlResult(result, 1);
  movement.moveParams.moveSpeedWater = floatFromFirstSqlResult(result, 23);
  movement.moveParams.jumpHeight = floatFromFirstSqlResult(result, 2);
  movement.moveParams.maxAngleUp = floatFromFirstSqlResult(result, 6);
  movement.moveParams.maxAngleDown = floatFromFirstSqlResult(result, 7);
  movement.moveParams.moveSoundDistance = floatFromFirstSqlResult(result, 14);
  movement.moveParams.moveSoundMintime = floatFromFirstSqlResult(result, 15);
  movement.moveParams.groundAngle = glm::cos(glm::radians(floatFromFirstSqlResult(result, 16)));
  movement.moveParams.gravity = glm::vec3(0.f, floatFromFirstSqlResult(result, 3), 0.f);
  movement.moveParams.canCrouch = boolFromFirstSqlResult(result, 18);;
  movement.moveParams.crouchSpeed = floatFromFirstSqlResult(result, 19);
  movement.moveParams.crouchScale = floatFromFirstSqlResult(result, 20);
  movement.moveParams.crouchDelay = floatFromFirstSqlResult(result, 21);
  movement.moveParams.friction = floatFromFirstSqlResult(result, 5);
  movement.moveParams.crouchFriction = floatFromFirstSqlResult(result, 22);
}

void reloadMovementConfig(Movement& movement, objid id, std::string name){
  auto traitsQuery = gameapi -> compileSqlQuery(
    "select speed, speed-air, jump-height, gravity, restitution, friction, max-angleup, max-angledown, mass, jump-sound, land-sound, dash, dash-sound, move-sound, move-sound-distance, move-sound-mintime, ground-angle, gravity-water, crouch, crouch-speed, crouch-scale, crouch-delay, crouch-friction, speed-water from traits where profile = " + name,
    {}
  );
  bool validTraitSql = false;
  auto traitsResult = gameapi -> executeSqlQuery(traitsQuery, &validTraitSql);
  modassert(validTraitSql, "error executing sql query");
  updateTraitConfig(movement, traitsResult);
  updateObjectProperties(id, traitsResult, movement.moveParams.gravity, movement.moveParams.friction);

  ensureSoundsLoaded(gameapi -> listSceneId(id), traitsResult.at(0).at(9), traitsResult.at(0).at(10), traitsResult.at(0).at(13));
  movement.lastMoveSoundPlayTime = 0.f;
  movement.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);
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

void attachToLadder(Movement& movement){
  if (movement.facingLadder){
     movement.attachedToLadder = true;
  }
}
void releaseFromLadder(Movement& movement){
  movement.attachedToLadder = false;
}

void toggleCrouch(Movement& movement, objid id, bool shouldCrouch){
  modlog("movement", "toggle crouch: " + print(shouldCrouch));
  auto crouchScale = movement.moveParams.crouchScale;
  auto scale = shouldCrouch ? glm::vec3(crouchScale, crouchScale, crouchScale) : glm::vec3(1.f, 1.f, 1.f);
  GameobjAttributes newAttr {
    .stringAttributes = {},
    .numAttributes = {
      { "physics_friction", shouldCrouch ? movement.moveParams.crouchFriction : movement.moveParams.friction },
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
    //gameapi -> applyImpulse(id, glm::vec3(0.f, 1.f, 0.f)); 
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

void updateCrouch(Movement& movement, objid id){
  if (movement.shouldBeCrouching && !movement.isCrouching){
    movement.isCrouching = true;
    movement.lastCrouchTime = gameapi -> timeSeconds(false);
    toggleCrouch(movement, id, true);
  }else if (!movement.shouldBeCrouching && movement.isCrouching){
    auto playerPos = gameapi -> getGameObjectPos(id, true);
    auto canUncrouch = !somethingAbovePlayer(playerPos);
    if (canUncrouch){
      movement.isCrouching = false;
      toggleCrouch(movement, id, false);
    }
  }
}

bool shouldStepUp(Movement& movement, objid id){
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

enum COLLISION_SPACE_INDEX { COLLISION_SPACE_LEFT = 0, COLLISION_SPACE_RIGHT = 1, COLLISION_SPACE_DOWN = 3 };
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

std::vector<bool>  checkMovementCollisions(Movement& movement, std::vector<glm::quat>& hitDirections, glm::quat rotationWithoutY){
  auto hitpoints = gameapi -> contactTest(movement.playerId.value());
  //std::cout << "hitpoints: [ ";

  //for (auto &hitpoint : hitpoints){
  //  std::cout << "\n[ id = " << hitpoint.id << ", pos = " << print(hitpoint.point) << ", normal = " << serializeQuat(hitpoint.normal) << " ] " << std::endl;
  //  auto normal = 10.f * glm::normalize(hitpoint.normal * glm::vec3(0.f, 0.f, -1.f));
  //  gameapi -> drawLine(hitpoint.point,  hitpoint.point + normal, true, movement.playerId, std::nullopt, std::nullopt, std::nullopt);
  //}
  for (auto &hitpoint : hitpoints){
    hitDirections.push_back(hitpoint.normal);
  }

  auto collisions = getCollisionSpaces(hitpoints, rotationWithoutY);
  //std::cout << "collisions: " << print(collisions) << std::endl;

  //std::cout << " ]" << std::endl;
  return collisions;
}

glm::vec3 limitMoveDirectionFromCollisions(Movement& movement, glm::vec3 moveVec, std::vector<glm::quat>& hitDirections, glm::quat playerDirection){
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

void changeTargetId(Movement& movement, objid id, bool active){
    movement.playerId =  id;
    movement.goForward = false;
    movement.goBackward = false;
    movement.goLeft = false;
    movement.goRight = false;
    movement.active = active;

    movement.lastMoveSoundPlayTime = 0.f;
    movement.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);


    movement.lookVelocity = glm::vec2(0.f, 0.f);
    movement.lastPosition = glm::vec3(0.f, 0.f, 0.f);

    auto oldXYRot = pitchXAndYawYRadians(gameapi -> getGameObjectRotation(movement.playerId.value(), true));

    movement.xRot = oldXYRot.x;
    movement.yRot = oldXYRot.y;

    movement.isGrounded = false;
    movement.lastFrameIsGrounded = false;
    movement.facingWall = false;
    movement.facingLadder = false;
    movement.attachedToLadder = false;

    movement.waterObjIds = {};
    movement.isCrouching = false;
    movement.shouldBeCrouching = false;
    movement.lastCrouchTime = -10000.f;  // so can immediately crouch

    reloadMovementConfig(movement, movement.playerId.value(), "default");
    reloadSettingsConfig(movement, "default");

}

CScriptBinding movementBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid _, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Movement* movement = new Movement;
    movement -> playerId = std::nullopt;
    movement -> goForward = false;
    movement -> goBackward = false;
    movement -> goLeft = false;
    movement -> goRight = false;
    movement -> active = false;

    movement -> lastMoveSoundPlayTime = 0.f;
    movement -> lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);

    movement -> lookVelocity = glm::vec2(0.f, 0.f);
    movement -> lastPosition = glm::vec3(0.f, 0.f, 0.f);
    movement -> xRot = 0.f;
    movement -> yRot = 0.f;
    movement -> isGrounded = false;
    movement -> lastFrameIsGrounded = false;
    movement -> facingWall = false;
    movement -> facingLadder = false;
    movement -> attachedToLadder = false;

    movement -> waterObjIds = {};
    movement -> isCrouching = false;
    movement -> shouldBeCrouching = false;
    movement -> lastCrouchTime = -10000.f;  // so can immediately crouch
    return movement;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Movement* value = (Movement*)data;
    ensureSoundsUnloaded(gameapi -> listSceneId(id));
    delete value;
  };
  binding.onKeyCallback = [](int32_t _, void* data, int key, int scancode, int action, int mods) -> void {
    if (isPaused()){
      return;
    }
    Movement* movement = static_cast<Movement*>(data);
    if (!movement -> playerId.has_value()){
      return;
    }

    std::cout << "key is: " << key << std::endl;

    if (key == 341){  // ctrl
      auto timeSinceLastCrouch = (gameapi -> timeSeconds(false) - movement -> lastCrouchTime) * 1000;
      if (action == 1 && movement -> moveParams.canCrouch && (timeSinceLastCrouch > movement -> moveParams.crouchDelay)){
        movement -> shouldBeCrouching = true;
      }else if (action == 0){
        movement -> shouldBeCrouching = false;
      }
    }

    if (key == 'R') { 
      if (action == 1){
        attachToLadder(*movement);
      }else if (action == 0){
        releaseFromLadder(*movement);        
      }
      return;
    }

    if (key == 'W'){
      if (action == 0){
        movement -> goForward = false;
      }else if (action == 1){
        movement -> goForward = true;
      }
      return;
    }
    if (key == 'S'){
      if (action == 0){
        movement -> goBackward = false;
      }else if (action == 1){
        movement -> goBackward = true;
      }
      return;
    }
    if (key == 'A'){
      if (action == 0){
        movement -> goLeft = false;
      }else if (action == 1){
        movement -> goLeft = true;
      }
      return;
    }
    if (key == 'D'){
      if (action == 0){
        movement -> goRight = false;
      }else if (action == 1){
        movement -> goRight = true;
      }
      return;
    }

    if (key == 32 /* space */ && action == 1){
      jump(*movement, movement -> playerId.value());
      gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
        .entityId = movement -> playerId.value(),
        .transition = "jump",
      });
      return;
    }

    if (key == '6' && action == 1){
      std::cout << "movement: request change control placeholder" << std::endl;
    }else if (key == '7' && action == 1){
      auto obj = gameapi -> getGameObjectByName("enemy", gameapi -> listSceneId(movement -> playerId.value()), false).value();
      gameapi -> sendNotifyMessage("request:change-control", obj);
      // needs to create use a temporary camera mounted to the character
    }
    else if (key == '8' && action == 1){
      auto obj = gameapi -> getGameObjectByName(">maincamera", gameapi -> listSceneId(movement -> playerId.value()), false).value();
      gameapi -> sendNotifyMessage("request:change-control", obj);
    }else if (key == '9' && action == 1){
      auto obj = gameapi -> getGameObjectByName(">maincamera2", gameapi -> listSceneId(movement -> playerId.value()), false).value();
      gameapi -> sendNotifyMessage("request:change-control", obj);
    }

  };
  binding.onMouseCallback = [](objid _, void* data, int button, int action, int mods) -> void {
    if (isPaused()){
      return;
    }
    Movement* movement = static_cast<Movement*>(data);
    if (button == 1){
      if (action == 0){
        std::vector<glm::quat> hitDirections;
        //checkMovementCollisions(*movement, hitDirections);
        for (auto &hitDirection : hitDirections){
          std::cout << "hit direction is: " << print(directionFromQuat(hitDirection)) << std::endl;
        }
        //checkMovementCollisions(*movement);
      }
    }
  };

  binding.onMouseMoveCallback = [](objid _, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void {
    if (isPaused()){
      return;
    }
    //std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    Movement* movement = static_cast<Movement*>(data);
    if (!movement -> active){
      return;
    }
    movement -> lookVelocity = glm::vec2(xPos, yPos);
  };
  binding.onFrame = [](int32_t _, void* data) -> void {
    if (isPaused()){
      return;
    }
    Movement* movement = static_cast<Movement*>(data);
    //checkMovementCollisions(*movement);

    if (!movement -> active){
      return;
    }
    if (!movement -> playerId.has_value()){
      return;
    }


    float horzRelVelocity = 0.8f;
    glm::vec2 moveVec(0.f, 0.f);

    bool shouldMoveXZ = false;
    bool isSideStepping = false;
    if (movement -> goForward){
      std::cout << "should move forward" << std::endl;
      moveVec += glm::vec2(0.f, -1.f);
      if (movement -> facingLadder || movement -> attachedToLadder){
        moveUp(movement -> playerId.value(), moveVec);
      }else{
        shouldMoveXZ = true;
      }
    }
    if (movement -> goBackward){
      moveVec += glm::vec2(0.f, 1.f);
      if (movement -> facingLadder || movement -> attachedToLadder){
        moveDown(movement -> playerId.value(), moveVec);
      }else{
        shouldMoveXZ = true;
      }
    }
    if (movement -> goLeft){
      moveVec += glm::vec2(horzRelVelocity * -1.f, 0.f);
      if (!movement -> attachedToLadder){
        shouldMoveXZ = true;
        isSideStepping = true;
      }
      
    }
    if (movement -> goRight){
      moveVec += glm::vec2(horzRelVelocity * 1.f, 0.f);
      if (!movement -> attachedToLadder){
        shouldMoveXZ = true;
        isSideStepping = true;
      }
    }

    std::vector<glm::quat> hitDirections;

    //std::vector<glm::quat> hitDirections;
    auto playerDirection = gameapi -> getGameObjectRotation(movement -> playerId.value(), true);
    auto directionVec = playerDirection * glm::vec3(0.f, 0.f, -1.f); 
    directionVec.y = 0.f;
    auto rotationWithoutY = quatFromDirection(directionVec);

    movement -> lastFrameIsGrounded = movement -> isGrounded;
    bool isGrounded = checkMovementCollisions(*movement, hitDirections, rotationWithoutY).at(COLLISION_SPACE_DOWN);
    movement -> isGrounded = isGrounded;

    if (movement -> isGrounded && !movement -> lastFrameIsGrounded){
      modlog("animation controller", "land");
      gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
        .entityId = movement -> playerId.value(),
        .transition = "land",
      });
    }

    float moveSpeed = getMoveSpeed(*movement, false, isGrounded);
    //modlog("editor: move speed: ", std::to_string(moveSpeed) + ", is grounded = " + print(isGrounded));

    auto limitedMoveVec = limitMoveDirectionFromCollisions(*movement, glm::vec3(moveVec.x, 0.f, moveVec.y), hitDirections, rotationWithoutY);
    //auto limitedMoveVec = moveVec;
    auto direction = glm::vec2(limitedMoveVec.x, limitedMoveVec.z);
    if (shouldMoveXZ){
      moveXZ(movement -> playerId.value(), moveSpeed * direction);
      if (isSideStepping){
        gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
          .entityId = movement -> playerId.value(),
          .transition = "sidestep",
        });
      }else{
        gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
          .entityId = movement -> playerId.value(),
          .transition = "walking",
        });
      }

    }else{
      gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
        .entityId = movement -> playerId.value(),
        .transition = "not-walking",
      });
    }

    auto currPos = gameapi -> getGameObjectPos(movement -> playerId.value(), true);
    auto currTime = gameapi -> timeSeconds(false);
  
    if (glm::length(currPos - movement -> lastMoveSoundPlayLocation) > movement -> moveParams.moveSoundDistance && isGrounded && getManagedSounds().moveSoundObjId.has_value() && ((currTime - movement -> lastMoveSoundPlayTime) > movement -> moveParams.moveSoundMintime)){
      // move-sound-distance:STRING move-sound-mintime:STRING
      std::cout << "should play move clip" << std::endl;
      gameapi -> playClipById(getManagedSounds().moveSoundObjId.value(), std::nullopt, std::nullopt);
      movement -> lastMoveSoundPlayTime = currTime;
      movement -> lastMoveSoundPlayLocation = currPos;
    }
    float elapsedTime = gameapi -> timeElapsed();
    look(*movement, movement -> playerId.value(), elapsedTime, false, 0.5f); // (look elapsedTime ironsight-mode ironsight-turn)

    bool movingDown = false;
    updateVelocity(*movement, movement -> playerId.value(), elapsedTime, currPos, &movingDown);
    updateFacingWall(*movement, movement -> playerId.value());
    restrictLadderMovement(*movement, movement -> playerId.value(), movingDown);
    updateCrouch(*movement, movement -> playerId.value());

    auto shouldStep = shouldStepUp(*movement, movement -> playerId.value()) && movement -> goForward;
    //std::cout << "should step up: " << shouldStep << std::endl;
    if (shouldStep){
      gameapi -> applyImpulse(movement -> playerId.value(), glm::vec3(0.f, 0.4f, 0.f));
    }

    //std::cout << "mounted to wall: " << print(movement -> facingWall) << ", facing ladder = " << print(movement -> facingLadder) << ", attached = " << print(movement -> attachedToLadder)  << ", grounded = " << print(movement -> groundedObjIds.size() > 0) <<  ", inwater = " << print(movement -> waterObjIds.size() > 0) << std::endl;
    //std::cout << movementToStr(*movement) << std::endl;
  };
  binding.onCollisionEnter = [](objid _, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal) -> void {
    Movement* movement = static_cast<Movement*>(data);
    if (!movement -> playerId.has_value()){
      return;
    }
    auto id = movement -> playerId.value();
    modlog("movement", "on collision enter: " + std::to_string(obj1) + ", " + std::to_string(obj2));
    if (id != obj1 && id != obj2){
      return; 
    }
    auto otherNormal = (id == obj2) ? normal : oppositeNormal;
    auto value = glm::dot(glm::normalize(otherNormal), glm::vec3(0.f, -1.f, 0.f));
    modlog("movement", "y component angleToCompare: " + std::to_string(movement -> moveParams.groundAngle) + ", reverse = " + std::to_string(glm::degrees(glm::acos(value))));
    modlog("movement", "y component dot: " + std::to_string(value));

    objid otherObjectId = id == obj1 ? obj2 : obj1;


    auto attr = gameapi -> getGameObjectAttr(otherObjectId);
    auto isWater = getStrAttr(attr, "water").has_value();

    if (isWater){
      movement -> waterObjIds.insert(otherObjectId);
    }
    if (!isWater && value >= movement -> moveParams.groundAngle){
      if (!movement -> lastFrameIsGrounded && movement -> isGrounded){
        land(*movement, id);
      }
    }
  };
  binding.onCollisionExit = [](objid _, void* data, int32_t obj1, int32_t obj2) -> void {
    modlog("movement", "on collision exit: " + std::to_string(obj1) + ", " + std::to_string(obj2));
    Movement* movement = static_cast<Movement*>(data);
    if (!movement -> playerId.has_value()){
      return;
    }
    auto id = movement -> playerId.value();
    auto otherObjectId = (id == obj1) ? obj2 : obj1;
    if (movement -> waterObjIds.count(otherObjectId) > 0){
      movement -> waterObjIds.erase(otherObjectId);
    }
  };

  binding.onMessage = [](int32_t _, void* data, std::string& key, std::any& value){
    Movement* movement = static_cast<Movement*>(data);
    if (key == "request:change-control"){
      Movement* movement = static_cast<Movement*>(data);
      auto objIdValue = anycast<objid>(value); 
      modassert(objIdValue != NULL, "movement - request change control value invalid");
      changeTargetId(*movement, *objIdValue, true);
    }
    if (!movement -> playerId.has_value()){
      return;
    }
    auto id = movement -> playerId.value();
    if (key == "reload-config:movement"){
      Movement* movement = static_cast<Movement*>(data);
      auto strValue = anycast<std::string>(value); 
      modassert(strValue != NULL, "reload-config:movement reload value invalid");
      reloadMovementConfig(*movement, id, *strValue);
    }else if (key == "reload-config:settings"){
      Movement* movement = static_cast<Movement*>(data);
      auto strValue = anycast<std::string>(value);
      modassert(strValue != NULL, "reload-config:settings reload value invalid");
      reloadSettingsConfig(*movement, *strValue);      
    }
  };

  return binding;
}


