#include "./movement.h"

// For parity in behavior of all scripts need to restore 
//    ironsight mode
//    dash ability

extern CustomApiBindings* gameapi;

struct Movement {
  ControlParams controlParams; // controls

  std::optional<objid> playerId;   // target id 
  bool active;

  MovementParams moveParams; // character params
  MovementState movementState;
};

std::string movementToStr(Movement& movement){
  std::string str;
  str += std::string("goForward: ") + (movement.controlParams.goForward ? "true" : "false") + "\n";
  str += std::string("goBackward: ") + (movement.controlParams.goBackward ? "true" : "false") + "\n";
  str += std::string("goLeft: ") + (movement.controlParams.goLeft ? "true" : "false") + "\n";
  str += std::string("goRight: ") + (movement.controlParams.goRight ? "true" : "false") + "\n";
  return str;
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
  movement.movementState.lastMoveSoundPlayTime = 0.f;
  movement.movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);
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

void changeTargetId(Movement& movement, objid id, bool active){
    movement.playerId =  id;
    movement.controlParams.goForward = false;
    movement.controlParams.goBackward = false;
    movement.controlParams.goLeft = false;
    movement.controlParams.goRight = false;
    movement.active = active;

    movement.movementState.lastMoveSoundPlayTime = 0.f;
    movement.movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);


    movement.controlParams.lookVelocity = glm::vec2(0.f, 0.f);
    movement.movementState.lastPosition = glm::vec3(0.f, 0.f, 0.f);

    auto oldXYRot = pitchXAndYawYRadians(gameapi -> getGameObjectRotation(movement.playerId.value(), true));

    movement.movementState.xRot = oldXYRot.x;
    movement.movementState.yRot = oldXYRot.y;

    movement.movementState.isGrounded = false;
    movement.movementState.lastFrameIsGrounded = false;
    movement.movementState.facingWall = false;
    movement.movementState.facingLadder = false;
    movement.movementState.attachedToLadder = false;

    movement.movementState.inWater = {};
    movement.movementState.isCrouching = false;
    movement.movementState.shouldBeCrouching = false;
    movement.movementState.lastCrouchTime = -10000.f;  // so can immediately crouch

    reloadMovementConfig(movement, movement.playerId.value(), "default");
    reloadSettingsConfig(movement, "default");
}

void onMovementFrame(MovementParams& moveParams, MovementState& movementState, objid playerId, ControlParams& controlParams, bool shouldMoveXZ, bool isSideStepping, glm::vec2 moveVec){
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

  

  float moveSpeed = getMoveSpeed(moveParams, movementState, false, isGrounded);
  //modlog("editor: move speed: ", std::to_string(moveSpeed) + ", is grounded = " + print(isGrounded));
  auto limitedMoveVec = limitMoveDirectionFromCollisions(glm::vec3(moveVec.x, 0.f, moveVec.y), hitDirections, rotationWithoutY);
  //auto limitedMoveVec = moveVec;
  auto direction = glm::vec2(limitedMoveVec.x, limitedMoveVec.z);

  if (shouldMoveXZ){
    moveXZ(playerId, moveSpeed * direction);
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
    gameapi -> playClipById(getManagedSounds().moveSoundObjId.value(), std::nullopt, std::nullopt);
    movementState.lastMoveSoundPlayTime = currTime;
    movementState.lastMoveSoundPlayLocation = currPos;
  }
    

  float elapsedTime = gameapi -> timeElapsed();

  look(moveParams, movementState, playerId, elapsedTime, false, 0.5f, controlParams.lookVelocity, controlParams); // (look elapsedTime ironsight-mode ironsight-turn)

  bool movingDown = false;
  updateVelocity(movementState, playerId, elapsedTime, currPos, &movingDown);

  updateFacingWall(movementState, playerId);
  restrictLadderMovement(movementState, playerId, movingDown);
  updateCrouch(moveParams, movementState, playerId);

  auto shouldStep = shouldStepUp(playerId) && controlParams.goForward;
  //std::cout << "should step up: " << shouldStep << std::endl;
  if (shouldStep){
    gameapi -> applyImpulse(playerId, glm::vec3(0.f, 0.4f, 0.f));
  }
}


CScriptBinding movementBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid _, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Movement* movement = new Movement;

    movement -> playerId = std::nullopt;
    movement -> active = false;

    movement -> controlParams.goForward = false;
    movement -> controlParams.goBackward = false;
    movement -> controlParams.goLeft = false;
    movement -> controlParams.goRight = false;
    movement -> controlParams.lookVelocity = glm::vec2(0.f, 0.f);

    movement -> movementState = MovementState {};
    movement -> movementState.lastMoveSoundPlayTime = 0.f;
    movement -> movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);

    movement -> movementState.lastPosition = glm::vec3(0.f, 0.f, 0.f);
    movement -> movementState.xRot = 0.f;
    movement -> movementState.yRot = 0.f;
    movement -> movementState.isGrounded = false;
    movement -> movementState.lastFrameIsGrounded = false;
    movement -> movementState.facingWall = false;
    movement -> movementState.facingLadder = false;
    movement -> movementState.attachedToLadder = false;

    movement -> movementState.inWater = false;
    movement -> movementState.isCrouching = false;
    movement -> movementState.shouldBeCrouching = false;
    movement -> movementState.lastCrouchTime = -10000.f;  // so can immediately crouch

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
      if (action == 0 || action == 1){
        maybeToggleCrouch(movement -> moveParams, movement -> movementState, action == 1);
      }
    }

    if (key == 'R') { 
      if (action == 1){
        attachToLadder(movement -> movementState);
      }else if (action == 0){
        releaseFromLadder(movement -> movementState);        
      }
      return;
    }

    if (key == 'W'){
      if (action == 0){
        movement -> controlParams.goForward = false;
      }else if (action == 1){
        movement -> controlParams.goForward = true;
      }
      return;
    }
    if (key == 'S'){
      if (action == 0){
        movement -> controlParams.goBackward = false;
      }else if (action == 1){
        movement -> controlParams.goBackward = true;
      }
      return;
    }
    if (key == 'A'){
      if (action == 0){
        movement -> controlParams.goLeft = false;
      }else if (action == 1){
        movement -> controlParams.goLeft = true;
      }
      return;
    }
    if (key == 'D'){
      if (action == 0){
        movement -> controlParams.goRight = false;
      }else if (action == 1){
        movement -> controlParams.goRight = true;
      }
      return;
    }

    if (key == 32 /* space */ && action == 1){
      jump(movement -> moveParams, movement -> movementState, movement -> playerId.value());
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
    movement -> controlParams.lookVelocity = glm::vec2(xPos, yPos);
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
    if (movement -> controlParams.goForward){
      std::cout << "should move forward" << std::endl;
      moveVec += glm::vec2(0.f, -1.f);
      if (movement -> movementState.facingLadder || movement -> movementState.attachedToLadder){
        moveUp(movement -> playerId.value(), moveVec);
      }else{
        shouldMoveXZ = true;
      }
    }
    if (movement -> controlParams.goBackward){
      moveVec += glm::vec2(0.f, 1.f);
      if (movement -> movementState.facingLadder || movement -> movementState.attachedToLadder){
        moveDown(movement -> playerId.value(), moveVec);
      }else{
        shouldMoveXZ = true;
      }
    }
    if (movement -> controlParams.goLeft){
      moveVec += glm::vec2(horzRelVelocity * -1.f, 0.f);
      if (!movement -> movementState.attachedToLadder){
        shouldMoveXZ = true;
        isSideStepping = true;
      }
      
    }
    if (movement -> controlParams.goRight){
      moveVec += glm::vec2(horzRelVelocity * 1.f, 0.f);
      if (!movement -> movementState.attachedToLadder){
        shouldMoveXZ = true;
        isSideStepping = true;
      }
    }


    onMovementFrame(movement -> moveParams, movement -> movementState, movement -> playerId.value(), movement -> controlParams, shouldMoveXZ, isSideStepping, moveVec);

    movement -> controlParams.lookVelocity = glm::vec2(0.f, 0.f);

    //std::cout << movementToStr(*movement) << std::endl;
  };

  binding.onMessage = [](int32_t _, void* data, std::string& key, std::any& value){
    Movement* movement = static_cast<Movement*>(data);
    if (key == "active-player-change"){
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

  binding.onObjectRemoved = [](int32_t _, void* data, int32_t idRemoved) -> void {
    Movement* movement = static_cast<Movement*>(data);
    if (movement -> playerId.has_value() && movement -> playerId.value() == idRemoved){
      movement -> playerId = std::nullopt;
    }
  };

  return binding;
}


