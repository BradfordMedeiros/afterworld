#include "./movement.h"

extern CustomApiBindings* gameapi;

struct MovementEntity {
  objid playerId;
  MovementParams* moveParams;
  MovementState movementState;
};

struct Movement {
  ControlParams controlParams;
};

std::optional<int> activeEntity;
std::vector<MovementEntity> movementEntities;

std::optional<int> entityIndex(objid playerId){
  for (int i = 0; i < movementEntities.size(); i++){
    if (movementEntities.at(i).playerId == playerId){
      return i;
    }
  }
  return std::nullopt;
}
void setActiveEntity(objid id){
  activeEntity = entityIndex(id).value();
}
std::optional<objid> setNextEntity(){
  if (!activeEntity.has_value()){
    if (movementEntities.size() > 0){
      activeEntity = 0;
      return movementEntities.at(0).playerId;
    }
    return std::nullopt;
  }
  auto nextIndex = activeEntity.value() + 1;
  if (nextIndex >= movementEntities.size()){
    nextIndex = 0;
  }
  activeEntity = nextIndex;
  return movementEntities.at(nextIndex).playerId;
}

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

MovementEntity createMovementEntity(objid id, std::string&& name){
  MovementEntity movementEntity {
    .playerId = id,
  };
  movementEntity.movementState = getInitialMovementState(movementEntity.playerId);

  loadMovementCore(name);
  movementEntity.moveParams = findMovementCore(name);
  modassert(movementEntity.moveParams, "could not find movement core");
  movementEntity.movementState.lastMoveSoundPlayTime = 0.f;
  movementEntity.movementState.lastMoveSoundPlayLocation = glm::vec3(0.f, 0.f, 0.f);

  updateObjectProperties(id, *movementEntity.moveParams);

  return movementEntity;
}

bool movementEntityExists(objid id){
  for (auto &movementEntity : movementEntities){
    if (movementEntity.playerId == id){
      return true;
    }
  }
  return false;
}
void maybeAddMovementEntity(objid id){
  if (movementEntityExists(id)){
    return;
  }
  auto player = getSingleAttr(id, "player");
  if (player.has_value()){
    movementEntities.push_back(createMovementEntity(id, "default"));
  }
}
void maybeRemoveMovementEntity(objid id){
  std::vector<MovementEntity> newEntities;
  for (int i = 0; i < movementEntities.size(); i++){
    MovementEntity& movementEntity = movementEntities.at(i);
    if (movementEntity.playerId != id){
      newEntities.push_back(movementEntity);
    }else if (i == activeEntity.value()){
      activeEntity = std::nullopt;
    }
  }
  movementEntities = newEntities;
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
  movement.controlParams.goForward = false;
  movement.controlParams.goBackward = false;
  movement.controlParams.goLeft = false;
  movement.controlParams.goRight = false;
  movement.controlParams.doJump = false;
  movement.controlParams.lookVelocity = glm::vec2(0.f, 0.f);  
  reloadSettingsConfig(movement, "default");
}

CScriptBinding movementBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid _, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Movement* movement = new Movement;

    movement -> controlParams.goForward = false;
    movement -> controlParams.goBackward = false;
    movement -> controlParams.goLeft = false;
    movement -> controlParams.goRight = false;
    movement -> controlParams.doJump = false;
    movement -> controlParams.lookVelocity = glm::vec2(0.f, 0.f);

    for (auto id : gameapi -> getObjectsByAttr("player", std::nullopt, std::nullopt)){
      maybeAddMovementEntity(id);
    }
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
    if (!activeEntity.has_value()){
      return;
    }

    std::cout << "key is: " << key << std::endl;

    if (key == 341){  // ctrl
      if (action == 0 || action == 1){
        MovementEntity& entity = movementEntities.at(activeEntity.value());
        maybeToggleCrouch(*entity.moveParams, entity.movementState, action == 1);
      }
    }

    if (key == 'R') { 
      if (action == 1){
        MovementEntity& entity = movementEntities.at(activeEntity.value());
        attachToLadder(entity.movementState);
      }else if (action == 0){
        MovementEntity& entity = movementEntities.at(activeEntity.value());
        releaseFromLadder(entity.movementState);        
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
      Movement* movement = static_cast<Movement*>(data);
      movement -> controlParams.doJump = true;
      return;
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
    Movement* movement = static_cast<Movement*>(data);
    if (!activeEntity.has_value()){
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
    if (!activeEntity.has_value()){
      return;
    }
    MovementEntity& entity = movementEntities.at(activeEntity.value());
    onMovementFrame(*entity.moveParams, entity.movementState, entity.playerId, movement -> controlParams);

    movement -> controlParams.lookVelocity = glm::vec2(0.f, 0.f);
    movement -> controlParams.doJump = false;

    modlog("movement num entitiies: ", std::to_string(movementEntities.size()));
  };

  binding.onMessage = [](int32_t _, void* data, std::string& key, std::any& value){
    Movement* movement = static_cast<Movement*>(data);
    if (key == "active-player-change"){
      Movement* movement = static_cast<Movement*>(data);
      auto objIdValue = anycast<objid>(value); 
      modassert(objIdValue != NULL, "movement - request change control value invalid");
      changeTargetId(*movement, *objIdValue);
    }
  };

  binding.onObjectAdded = [](int32_t _, void* data, int32_t idAdded) -> void {
    maybeAddMovementEntity(idAdded);
  };
  binding.onObjectRemoved = [](int32_t _, void* data, int32_t idRemoved) -> void {
    maybeRemoveMovementEntity(idRemoved);
  };

  return binding;
}


