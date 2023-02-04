#include "./movement.h"


// For parity in behavior of all scripts need to restore 
//    ironsight mode
//    dash ability

extern CustomApiBindings* gameapi;

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
  glm::vec3 waterGravity;
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
  MovementParams moveParams;
  ControlParams controlParams;

  bool goForward;
  bool goBackward;
  bool goLeft;
  bool goRight;
  bool active;

  std::optional<objid> jumpSoundObjId;
  std::optional<objid> landSoundObjId;
  std::optional<objid> moveSoundObjId;
  float lastMoveSoundPlayTime;
  glm::vec3 lastMoveSoundPlayLocation;

  glm::vec2 lookVelocity;
  float xRot; // up and down
  float yRot; // left and right
  std::set<objid> groundedObjIds; // a rigid body can have collision spots with multiple grounds at once
  bool facingWall;
  bool facingLadder;
  bool attachedToLadder;
  std::set<objid> waterObjIds;

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
  if (movement.groundedObjIds.size() > 0){
    gameapi -> applyImpulse(id, impulse);
    if (movement.jumpSoundObjId.has_value()){
      gameapi -> playClip("&code-movement-jump", gameapi -> listSceneId(id), std::nullopt, std::nullopt);
    }
  }

  if (movement.waterObjIds.size() > 0){
    gameapi -> applyImpulse(id, impulse);
  }
}


void dash(Movement& movement){
  std::cout << "dash placeholder" << std::endl;
}

void moveUp(objid id, glm::vec2 direction){
  std::cout << "move up" << std::endl;
  float time = gameapi -> timeElapsed();
  gameapi -> applyImpulse(id, time * glm::vec3(0.f, -direction.y, 0.f));
}
void moveDown(objid id, glm::vec2 direction){
  float time = gameapi -> timeElapsed();
  gameapi -> applyImpulse(id, time * glm::vec3(0.f, -direction.y, 0.f));
}
void moveXZ(objid id, glm::vec2 direction){
  float time = gameapi -> timeElapsed();
  gameapi -> applyImpulseRel(id, time * glm::vec3(direction.x, 0.f, direction.y));
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


void look(Movement& movement, objid id, float elapsedTime, bool ironsight, float ironsight_turn){
  auto forwardVec = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));

  float raw_deltax = movement.lookVelocity.x * movement.controlParams.xsensitivity * elapsedTime;
  float raw_deltay = -1.f * movement.lookVelocity.y * movement.controlParams.ysensitivity * elapsedTime; 

  float deltax = ironsight ? (raw_deltax * ironsight_turn) : raw_deltax;
  float deltay = ironsight ? (raw_deltay * ironsight_turn) : raw_deltay;

  movement.xRot = limitAngle(movement.xRot + deltax, std::nullopt, std::nullopt);
  movement.yRot = limitAngle(movement.yRot + deltay, movement.moveParams.maxAngleUp, movement.moveParams.maxAngleDown); 

  auto rotation = gameapi -> setFrontDelta(forwardVec, movement.xRot, movement.yRot, 0, 1.f);
  gameapi -> setGameObjectRot(id, rotation);

  movement.lookVelocity = glm::vec2(0.f, 0.f);
}

void land(Movement& movement, objid id){
  if (movement.landSoundObjId.has_value()){
    gameapi -> playClip("&code-movement-land", gameapi -> listSceneId(id), std::nullopt, std::nullopt);
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
  movement.moveParams.waterGravity = glm::vec3(0.f, floatFromFirstSqlResult(result, 17), 0.f);
  movement.moveParams.canCrouch = boolFromFirstSqlResult(result, 18);;
  movement.moveParams.crouchSpeed = floatFromFirstSqlResult(result, 19);
  movement.moveParams.crouchScale = floatFromFirstSqlResult(result, 20);
  movement.moveParams.crouchDelay = floatFromFirstSqlResult(result, 21);
  movement.moveParams.friction = floatFromFirstSqlResult(result, 5);
  movement.moveParams.crouchFriction = floatFromFirstSqlResult(result, 22);
}

objid createSound(objid mainobjId, std::string soundObjName, std::string clip){
  modassert(soundObjName.at(0) == '&', "sound obj must start with &");
  auto sceneId = gameapi -> listSceneId(mainobjId);
  GameobjAttributes attr {
    .stringAttributes = {
      { "clip", clip },
    },
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto soundObjId = gameapi -> makeObjectAttr(sceneId, soundObjName, attr, submodelAttributes);
  modassert(soundObjId.has_value(), "sound already exists in scene: " + std::to_string(sceneId));
  gameapi -> makeParent(soundObjId.value(), mainobjId);
  return soundObjId.value();
}
struct SoundConfig {
  std::string jumpClip;
  std::string landClip;
  std::string moveClip;
};
void updateSoundConfig(Movement& movement, objid id, SoundConfig config){
  if (config.jumpClip != ""){
    movement.jumpSoundObjId = createSound(id, "&code-movement-jump", config.jumpClip);
  }
  if (config.landClip != ""){
    movement.landSoundObjId = createSound(id, "&code-movement-land", config.landClip);
  }
  movement.lastMoveSoundPlayTime = 0.f;
  movement.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);
  if (config.moveClip != ""){
    movement.moveSoundObjId = createSound(id, "&code-move", config.moveClip);
  }
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
  updateSoundConfig(movement, id, SoundConfig {
    .jumpClip = traitsResult.at(0).at(9),
    .landClip = traitsResult.at(0).at(10),
    .moveClip = traitsResult.at(0).at(13),
  });
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

void changeWaterGravity(Movement& movement, objid id){
  auto inWater = movement.waterObjIds.size() > 0;
  auto gravity = inWater ? movement.moveParams.waterGravity : movement.moveParams.gravity;
  GameobjAttributes newAttr {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = { 
      .vec3 = { { "physics_gravity", gravity }}, 
      .vec4 = { } 
    },
  };
  //std::cout << "new gravity is: " << print(gravity) << ", in water = " << print(inWater) << std::endl;
  gameapi -> setGameObjectAttr(id, newAttr);
}

void toggleCrouch(Movement& movement, objid id, bool shouldCrouch){
  std::cout << "toggle crouch: " << shouldCrouch << std::endl;
  auto crouchScale = movement.moveParams.crouchScale;
  GameobjAttributes newAttr {
    .stringAttributes = {},
    .numAttributes = {
      { "physics_friction", shouldCrouch ? movement.moveParams.crouchFriction : movement.moveParams.friction },
    },
    .vecAttr = { 
      .vec3 = { 
        { "scale", shouldCrouch ? glm::vec3(crouchScale, crouchScale, crouchScale) : glm::vec3(1.f, 1.f, 1.f) },
      },
      .vec4 = { } 
    },
  };
  gameapi -> setGameObjectAttr(id, newAttr);
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

CScriptBinding movementBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Movement* movement = new Movement;
    movement -> goForward = false;
    movement -> goBackward = false;
    movement -> goLeft = false;
    movement -> goRight = false;
    movement -> active = false;

    movement -> jumpSoundObjId = std::nullopt;
    movement -> landSoundObjId = std::nullopt;
    movement -> moveSoundObjId = std::nullopt;
    movement -> lastMoveSoundPlayTime = 0.f;
    movement -> lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);

    movement -> lookVelocity = glm::vec2(0.f, 0.f);
    movement -> lastPosition = glm::vec3(0.f, 0.f, 0.f);
    movement -> xRot = 0.f;
    movement -> yRot = 0.f;
    movement -> groundedObjIds = {};
    movement -> facingWall = false;
    movement -> facingLadder = false;
    movement -> attachedToLadder = false;
    movement -> waterObjIds = {};
    movement -> isCrouching = false;
    movement -> shouldBeCrouching = false;
    movement -> lastCrouchTime = -10000.f;  // so can immediately crouch

    reloadMovementConfig(*movement, id, "default");
    reloadSettingsConfig(*movement, "default");

    return movement;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Movement* value = (Movement*)data;
    delete value;
  };
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    Movement* movement = static_cast<Movement*>(data);

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
      jump(*movement, id);
      return;
    }
    if (key == 340 /* left shift */ && action == 1){
      dash(*movement);
      return;
    }
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    //std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    Movement* movement = static_cast<Movement*>(data);
    if (!movement -> active){
      return;
    }
    movement -> lookVelocity = glm::vec2(xPos, yPos);
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    Movement* movement = static_cast<Movement*>(data);
    if (!movement -> active){
      return;
    }

    bool isGrounded = movement -> groundedObjIds.size() > 0;
    float moveSpeed = getMoveSpeed(*movement, false, isGrounded);
    float horzRelVelocity = 0.8f;

    glm::vec2 moveVec(0.f, 0.f);
    if (movement -> goForward){
      moveVec = moveSpeed * glm::vec2(0.f, -1.f);
      if (movement -> facingLadder || movement -> attachedToLadder){
        moveUp(id, moveVec);
      }else{
        moveXZ(id, moveVec);
      }
    }
    if (movement -> goBackward){
      moveVec = moveSpeed * glm::vec2(0.f, 1.f);
      if (movement -> facingLadder || movement -> attachedToLadder){
        moveDown(id, moveVec);
      }else{
        moveXZ(id, moveVec);
      }
    }
    if (movement -> goLeft){
      moveVec = moveSpeed * glm::vec2(horzRelVelocity * -1.f, 0.f);
      if (!movement -> attachedToLadder){
        moveXZ(id, moveVec);
      }
      
    }
    if (movement -> goRight){
      moveVec = moveSpeed * glm::vec2(horzRelVelocity * 1.f, 0.f);
      if (!movement -> attachedToLadder){
        moveXZ(id, moveVec);
      }
    }


    auto currPos = gameapi -> getGameObjectPos(id, true);
    auto currTime = gameapi -> timeSeconds(false);
  
    if (glm::length(currPos - movement -> lastMoveSoundPlayLocation) > movement -> moveParams.moveSoundDistance && isGrounded && movement -> moveSoundObjId.has_value() && ((currTime - movement -> lastMoveSoundPlayTime) > movement -> moveParams.moveSoundMintime)){
      // move-sound-distance:STRING move-sound-mintime:STRING
      std::cout << "should play move clip" << std::endl;
      gameapi -> playClip("&code-move", gameapi -> listSceneId(id), std::nullopt, std::nullopt);
      movement -> lastMoveSoundPlayTime = currTime;
      movement -> lastMoveSoundPlayLocation = currPos;
    }
    float elapsedTime = gameapi -> timeElapsed();
    look(*movement, id, elapsedTime, false, 0.5f); // (look elapsedTime ironsight-mode ironsight-turn)

    bool movingDown = false;
    updateVelocity(*movement, id, elapsedTime, currPos, &movingDown);
    updateFacingWall(*movement, id);
    restrictLadderMovement(*movement, id, movingDown);
    updateCrouch(*movement, id);

    auto shouldStep = shouldStepUp(*movement, id) && movement -> goForward;
    //std::cout << "should step up: " << shouldStep << std::endl;
    if (shouldStep){
      gameapi -> applyImpulse(id, glm::vec3(0.f, 0.4f, 0.f));
    }

    //std::cout << "mounted to wall: " << print(movement -> facingWall) << ", facing ladder = " << print(movement -> facingLadder) << ", attached = " << print(movement -> attachedToLadder)  << ", grounded = " << print(movement -> groundedObjIds.size() > 0) <<  ", inwater = " << print(movement -> waterObjIds.size() > 0) << std::endl;
    //std::cout << movementToStr(*movement) << std::endl;
  };
  binding.onCollisionEnter = [](objid id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal) -> void {
    modlog("movement", "on collision enter: " + std::to_string(obj1) + ", " + std::to_string(obj2));
    if (id != obj1 && id != obj2){
      return; 
    }
    Movement* movement = static_cast<Movement*>(data);
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
      if (movement -> groundedObjIds.size() == 0){
        land(*movement, id);
      }
      movement -> groundedObjIds.insert(otherObjectId);
    }

    changeWaterGravity(*movement, id);

    // waterObjIdsattr

  };
  binding.onCollisionExit = [](objid id, void* data, int32_t obj1, int32_t obj2) -> void {
    modlog("movement", "on collision exit: " + std::to_string(obj1) + ", " + std::to_string(obj2));
    Movement* movement = static_cast<Movement*>(data);
    auto otherObjectId = (id == obj1) ? obj2 : obj1;
    if (movement -> groundedObjIds.count(otherObjectId) > 0){
      movement -> groundedObjIds.erase(otherObjectId);
    }
    if (movement -> waterObjIds.count(otherObjectId) > 0){
      movement -> waterObjIds.erase(otherObjectId);
    }
    changeWaterGravity(*movement, id);
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
    if (key == "reload-config:movement"){
      Movement* movement = static_cast<Movement*>(data);
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "reload-config:movement reload value invalid");
      reloadMovementConfig(*movement, id, *strValue);
    }else if (key == "reload-config:settings"){
      Movement* movement = static_cast<Movement*>(data);
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "reload-config:settings reload value invalid");
      reloadSettingsConfig(*movement, *strValue);      
    }else if (key == "request:change-control"){
      Movement* movement = static_cast<Movement*>(data);
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "selected value invalid");
      auto gameObjId = std::atoi(strValue -> c_str());
      movement -> active = gameObjId == id;
    }
  };

  return binding;
}


