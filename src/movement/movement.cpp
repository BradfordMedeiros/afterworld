#include "./movement.h"

// For parity in behavior of all scripts need to restore 
//    ironsight mode
//    dash ability

extern CustomApiBindings* gameapi;

struct MovementEntity {
  objid playerId;
  MovementParams* moveParams;
  MovementState movementState;
};


struct Movement {
  ControlParams controlParams; // controls
  std::optional<MovementEntity> entity;
};


void updateObjectProperties(objid id, MovementParams& moveParams){
  GameobjAttributes attr {
    .stringAttributes = {},
    .numAttributes = {
      { "physics_mass", moveParams.physicsMass },
      { "physics_restitution", moveParams.physicsRestitution },
      { "physics_friction", moveParams.friction },
    },
    .vecAttr = { 
      .vec3 = {
        { "physics_gravity", moveParams.gravity },
      }, 
      .vec4 = {} 
    },
  };
  gameapi -> setGameObjectAttr(id, attr);
}

void reloadMovementConfig(Movement& movement, objid id, std::string name){
  loadMovementCore(name);
  movement.entity.value().moveParams = findMovementCore(name);
  modassert(movement.entity.value().moveParams, "could not find movement core");

  updateObjectProperties(id, *movement.entity.value().moveParams);

  movement.entity.value().movementState.lastMoveSoundPlayTime = 0.f;
  movement.entity.value().movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);
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


void changeTargetId(Movement& movement, objid id){
  movement.entity = MovementEntity{
    .playerId = id,
  };

  movement.controlParams.goForward = false;
  movement.controlParams.goBackward = false;
  movement.controlParams.goLeft = false;
  movement.controlParams.goRight = false;
  movement.controlParams.lookVelocity = glm::vec2(0.f, 0.f);
  movement.entity.value().movementState = getInitialMovementState(movement.entity.value().playerId);

  reloadMovementConfig(movement, movement.entity.value().playerId, "default");
  reloadSettingsConfig(movement, "default");
}


CScriptBinding movementBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid _, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Movement* movement = new Movement;

    movement -> entity = std::nullopt;

    movement -> controlParams.goForward = false;
    movement -> controlParams.goBackward = false;
    movement -> controlParams.goLeft = false;
    movement -> controlParams.goRight = false;
    movement -> controlParams.lookVelocity = glm::vec2(0.f, 0.f);

    return movement;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Movement* value = (Movement*)data;
    removeAllMovementCores();
    delete value;
  };
  binding.onKeyCallback = [](int32_t _, void* data, int key, int scancode, int action, int mods) -> void {
    if (isPaused()){
      return;
    }
    Movement* movement = static_cast<Movement*>(data);
    if (!movement -> entity.has_value()){
      return;
    }

    std::cout << "key is: " << key << std::endl;

    if (key == 341){  // ctrl
      if (action == 0 || action == 1){
        maybeToggleCrouch(*movement -> entity.value().moveParams, movement -> entity.value().movementState, action == 1);
      }
    }

    if (key == 'R') { 
      if (action == 1){
        attachToLadder(movement -> entity.value().movementState);
      }else if (action == 0){
        releaseFromLadder(movement -> entity.value().movementState);        
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
      jump(*movement -> entity.value().moveParams, movement -> entity.value().movementState, movement -> entity.value().playerId);
      return;
    }

    if (key == '6' && action == 1){
      std::cout << "movement: request change control placeholder" << std::endl;
    }else if (key == '7' && action == 1){
      auto obj = gameapi -> getGameObjectByName("enemy", gameapi -> listSceneId(movement -> entity.value().playerId), false).value();
      gameapi -> sendNotifyMessage("request:change-control", obj);
      // needs to create use a temporary camera mounted to the character
    }
    else if (key == '8' && action == 1){
      auto obj = gameapi -> getGameObjectByName(">maincamera", gameapi -> listSceneId(movement -> entity.value().playerId), false).value();
      gameapi -> sendNotifyMessage("request:change-control", obj);
    }else if (key == '9' && action == 1){
      auto obj = gameapi -> getGameObjectByName(">maincamera2", gameapi -> listSceneId(movement -> entity.value().playerId), false).value();
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
    if (!movement -> entity.has_value()){
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

    if (!movement -> entity.has_value()){
      return;
    }

    onMovementFrame(*movement -> entity.value().moveParams, movement -> entity.value().movementState, movement -> entity.value().playerId, movement -> controlParams);

    movement -> controlParams.lookVelocity = glm::vec2(0.f, 0.f);

    //std::cout << movementToStr(*movement) << std::endl;
  };

  binding.onMessage = [](int32_t _, void* data, std::string& key, std::any& value){
    Movement* movement = static_cast<Movement*>(data);
    if (key == "active-player-change"){
      Movement* movement = static_cast<Movement*>(data);
      auto objIdValue = anycast<objid>(value); 
      modassert(objIdValue != NULL, "movement - request change control value invalid");
      changeTargetId(*movement, *objIdValue);
    }
    if (!movement -> entity.has_value()){
      return;
    }
    auto id = movement -> entity.value().playerId;
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
    if (movement -> entity.has_value() && movement -> entity.value().playerId == idRemoved){
      movement -> entity = std::nullopt;
    }
  };

  return binding;
}


