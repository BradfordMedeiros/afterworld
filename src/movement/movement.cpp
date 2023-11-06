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

MovementParams getMovementParams(std::vector<std::vector<std::string>>& result){
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
  moveParams.canCrouch = boolFromFirstSqlResult(result, 18);;
  moveParams.crouchSpeed = floatFromFirstSqlResult(result, 19);
  moveParams.crouchScale = floatFromFirstSqlResult(result, 20);
  moveParams.crouchDelay = floatFromFirstSqlResult(result, 21);
  moveParams.friction = floatFromFirstSqlResult(result, 5);
  moveParams.crouchFriction = floatFromFirstSqlResult(result, 22);
  return moveParams;
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

void reloadMovementConfig(Movement& movement, objid id, std::string name){
  auto traitsQuery = gameapi -> compileSqlQuery(
    "select speed, speed-air, jump-height, gravity, restitution, friction, max-angleup, max-angledown, mass, jump-sound, land-sound, dash, dash-sound, move-sound, move-sound-distance, move-sound-mintime, ground-angle, gravity-water, crouch, crouch-speed, crouch-scale, crouch-delay, crouch-friction, speed-water from traits where profile = " + name,
    {}
  );
  bool validTraitSql = false;
  auto traitsResult = gameapi -> executeSqlQuery(traitsQuery, &validTraitSql);
  modassert(validTraitSql, "error executing sql query");
  movement.moveParams = getMovementParams(traitsResult);

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
    movement.active = active;

    movement.controlParams.goForward = false;
    movement.controlParams.goBackward = false;
    movement.controlParams.goLeft = false;
    movement.controlParams.goRight = false;
    movement.controlParams.lookVelocity = glm::vec2(0.f, 0.f);
    movement.movementState = getInitialMovementState(movement.playerId);

    reloadMovementConfig(movement, movement.playerId.value(), "default");
    reloadSettingsConfig(movement, "default");
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

    movement -> movementState = getInitialMovementState(movement -> playerId);

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

    onMovementFrame(movement -> moveParams, movement -> movementState, movement -> playerId.value(), movement -> controlParams);

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


