#include "./movement.h"

// For parity in behavior of all scripts need to restore 
//    ironsight mode
//    dash ability

extern CustomApiBindings* gameapi;

struct Movement {
  ControlParams controlParams; // controls

  std::optional<objid> playerId;   // target id 

  bool goForward;                 // control params
  bool goBackward;
  bool goLeft;
  bool goRight;
  bool active;
  glm::vec2 lookVelocity;       // control params

  MovementParams moveParams; // character params
  MovementState movementState;
};

std::string movementToStr(Movement& movement){
  std::string str;
  str += std::string("goForward: ") + (movement.goForward ? "true" : "false") + "\n";
  str += std::string("goBackward: ") + (movement.goBackward ? "true" : "false") + "\n";
  str += std::string("goLeft: ") + (movement.goLeft ? "true" : "false") + "\n";
  str += std::string("goRight: ") + (movement.goRight ? "true" : "false") + "\n";
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
    movement.goForward = false;
    movement.goBackward = false;
    movement.goLeft = false;
    movement.goRight = false;
    movement.active = active;

    movement.movementState.lastMoveSoundPlayTime = 0.f;
    movement.movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);


    movement.lookVelocity = glm::vec2(0.f, 0.f);
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

    movement -> movementState.lastMoveSoundPlayTime = 0.f;
    movement -> movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);

    movement -> lookVelocity = glm::vec2(0.f, 0.f);
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

    movement -> movementState = MovementState {

    };

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
      auto timeSinceLastCrouch = (gameapi -> timeSeconds(false) - movement -> movementState.lastCrouchTime) * 1000;
      if (action == 1 && movement -> moveParams.canCrouch && (timeSinceLastCrouch > movement -> moveParams.crouchDelay)){
        movement -> movementState.shouldBeCrouching = true;
      }else if (action == 0){
        movement -> movementState.shouldBeCrouching = false;
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
      jump(movement -> moveParams, movement -> movementState, movement -> playerId.value());
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
      if (movement -> movementState.facingLadder || movement -> movementState.attachedToLadder){
        moveUp(movement -> playerId.value(), moveVec);
      }else{
        shouldMoveXZ = true;
      }
    }
    if (movement -> goBackward){
      moveVec += glm::vec2(0.f, 1.f);
      if (movement -> movementState.facingLadder || movement -> movementState.attachedToLadder){
        moveDown(movement -> playerId.value(), moveVec);
      }else{
        shouldMoveXZ = true;
      }
    }
    if (movement -> goLeft){
      moveVec += glm::vec2(horzRelVelocity * -1.f, 0.f);
      if (!movement -> movementState.attachedToLadder){
        shouldMoveXZ = true;
        isSideStepping = true;
      }
      
    }
    if (movement -> goRight){
      moveVec += glm::vec2(horzRelVelocity * 1.f, 0.f);
      if (!movement -> movementState.attachedToLadder){
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

    movement -> movementState.lastFrameIsGrounded = movement -> movementState.isGrounded;

    auto collisions = checkMovementCollisions(movement -> playerId.value(), hitDirections, rotationWithoutY);
    bool isGrounded = collisions.movementCollisions.at(COLLISION_SPACE_DOWN);
   
    movement -> movementState.inWater = false;
    for (auto id : collisions.allCollisions){
      auto isWater = getSingleAttr(id, "water").has_value();
      if (isWater){
        movement -> movementState.inWater = true;
        break;
      }
    }
    movement -> movementState.isGrounded = isGrounded && !movement -> movementState.inWater; 

    if (movement -> movementState.isGrounded && !movement -> movementState.lastFrameIsGrounded){
      modlog("animation controller", "land");
      gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
        .entityId = movement -> playerId.value(),
        .transition = "land",
      });

      land(movement -> playerId.value());
    }


    float moveSpeed = getMoveSpeed(movement -> moveParams, movement -> movementState, false, isGrounded);
    //modlog("editor: move speed: ", std::to_string(moveSpeed) + ", is grounded = " + print(isGrounded));

    auto limitedMoveVec = limitMoveDirectionFromCollisions(glm::vec3(moveVec.x, 0.f, moveVec.y), hitDirections, rotationWithoutY);
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
  
    if (glm::length(currPos - movement -> movementState.lastMoveSoundPlayLocation) > movement -> moveParams.moveSoundDistance && isGrounded && getManagedSounds().moveSoundObjId.has_value() && ((currTime - movement -> movementState.lastMoveSoundPlayTime) > movement -> moveParams.moveSoundMintime)){
      // move-sound-distance:STRING move-sound-mintime:STRING
      std::cout << "should play move clip" << std::endl;
      gameapi -> playClipById(getManagedSounds().moveSoundObjId.value(), std::nullopt, std::nullopt);
      movement -> movementState.lastMoveSoundPlayTime = currTime;
      movement -> movementState.lastMoveSoundPlayLocation = currPos;
    }
    float elapsedTime = gameapi -> timeElapsed();
    look(movement -> moveParams, movement -> movementState, movement -> playerId.value(), elapsedTime, false, 0.5f, movement -> lookVelocity, movement -> controlParams); // (look elapsedTime ironsight-mode ironsight-turn)
    movement -> lookVelocity = glm::vec2(0.f, 0.f);

    bool movingDown = false;
    updateVelocity(movement -> movementState, movement -> playerId.value(), elapsedTime, currPos, &movingDown);
    updateFacingWall(movement -> movementState, movement -> playerId.value());
    restrictLadderMovement(movement -> movementState, movement -> playerId.value(), movingDown);
    updateCrouch(movement -> moveParams, movement -> movementState, movement -> playerId.value());

    auto shouldStep = shouldStepUp(movement -> playerId.value()) && movement -> goForward;
    //std::cout << "should step up: " << shouldStep << std::endl;
    if (shouldStep){
      gameapi -> applyImpulse(movement -> playerId.value(), glm::vec3(0.f, 0.4f, 0.f));
    }

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


