#include "./movement.h"


// For parity in behavior of all scripts need to restore 
//    ironsight mode
//    dash ability

extern CustomApiBindings* gameapi;

struct MovementParams {
  float moveSpeed;
  float moveSpeedAir;
  float jumpHeight;
  float maxAngleUp;
  float maxAngleDown;
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


  std::optional<objid> jumpSoundObjId;
  std::optional<objid> landSoundObjId;

  glm::vec2 lookVelocity;
  float xRot; // up and down
  float yRot; // left and right
  std::optional<objid> groundedObjId;

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
  if (movement.groundedObjId.has_value()){
    gameapi -> applyImpulse(id, impulse);
    if (movement.jumpSoundObjId.has_value()){
      gameapi -> playClip("&code-movement-jump", gameapi -> listSceneId(id));
    }
  }
}


void dash(Movement& movement){
  std::cout << "dash placeholder" << std::endl;
}

void moveXZ(objid id, glm::vec2 direction){
  float time = gameapi -> timeElapsed();
  gameapi -> applyImpulseRel(id, time * glm::vec3(direction.x, 0.f, direction.y));
}

float ironsightSpeedMultiplier = 0.4f;
float getMoveSpeed(Movement& movement, bool ironsight){
  auto speed = movement.groundedObjId.has_value() ? movement.moveParams.moveSpeed : movement.moveParams.moveSpeedAir;
  if (ironsight){
    speed = speed * ironsightSpeedMultiplier;
  }
  return speed;
}

void updateVelocity(Movement& movement, objid id, float elapsedTime){
  auto currPos = gameapi -> getGameObjectPos(id, true);
  glm::vec3 displacement = (currPos - movement.lastPosition) / elapsedTime;
  //auto speed = glm::length(displacement);
  movement.lastPosition = currPos;
  //std::cout << "velocity = " << print(displacement) << ", speed = " << speed << std::endl;
  gameapi -> sendNotifyMessage("velocity", serializeVec(displacement));
}

float PI = 3.141592;
float TWO_PI = 2 * PI;
float clampPi(float value){ 
  if (value > 0){
    int numTimes = glm::floor(value / TWO_PI);
    float remain =  value - (TWO_PI * numTimes);
    return (remain > PI) ? (- remain - TWO_PI) : remain;
  }

  int numTimes = glm::floor(value / (-1 * TWO_PI));
  float remain = value + (TWO_PI * numTimes);
  return (remain < (-1 * PI)) ? (remain + TWO_PI) : remain;
}

void look(Movement& movement, objid id, float elapsedTime, bool ironsight, float ironsight_turn){
  auto forwardVec = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));

  float raw_deltax = movement.lookVelocity.x * movement.controlParams.xsensitivity * elapsedTime;
  float raw_deltay = -1.f * movement.lookVelocity.y * movement.controlParams.ysensitivity * elapsedTime; 

  float deltax = ironsight ? (raw_deltax * ironsight_turn) : raw_deltax;
  float deltay = ironsight ? (raw_deltay * ironsight_turn) : raw_deltay;

  float targetXRot = clampPi(movement.xRot + deltax);
  float targetYRot = clampPi(movement.yRot + deltay);

  movement.xRot = targetXRot;
  movement.yRot = glm::min(movement.moveParams.maxAngleDown, glm::max(movement.moveParams.maxAngleUp, targetYRot));

  auto rotation = gameapi -> setFrontDelta(forwardVec, movement.xRot, movement.yRot, 0, 1.f);
  gameapi -> setGameObjectRot(id, rotation);

  movement.lookVelocity = glm::vec2(0.f, 0.f);
}

void land(Movement& movement, objid id){
  if (movement.landSoundObjId.has_value()){
    gameapi -> playClip("&code-movement-land", gameapi -> listSceneId(id));
  }
}


void updateObjectProperties(objid id, std::vector<std::vector<std::string>>& result){
  float physics_mass = floatFromFirstSqlResult(result, 8);
  float physics_restitution = floatFromFirstSqlResult(result, 4);
  float physics_friction = floatFromFirstSqlResult(result, 5);
  auto physics_gravity = glm::vec3(0.f, floatFromFirstSqlResult(result, 3), 0.f);
  GameobjAttributes attr {
    .stringAttributes = {
    },
    .numAttributes = {
      { "physics_mass", physics_mass },
      { "physics_restitution", physics_restitution },
      { "physics_friction", physics_friction },
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
  movement.moveParams.jumpHeight = floatFromFirstSqlResult(result, 2);
  movement.moveParams.maxAngleUp = floatFromFirstSqlResult(result, 6);
  movement.moveParams.maxAngleDown = floatFromFirstSqlResult(result, 7);
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
};
void updateSoundConfig(Movement& movement, objid id, SoundConfig config){
  movement.jumpSoundObjId = createSound(id, "&code-movement-jump", config.jumpClip);
  movement.landSoundObjId = createSound(id, "&code-movement-land", config.landClip);
}

void reloadMovementConfig(Movement& movement, objid id, std::string name){
  auto traitsQuery = gameapi -> compileSqlQuery(
    "select speed, speed-air, jump-height, gravity, restitution, friction, max-angleup, max-angledown, mass, jump-sound, land-sound, dash, dash-sound from traits where profile = " + name
  );
  bool validTraitSql = false;
  auto traitsResult = gameapi -> executeSqlQuery(traitsQuery, &validTraitSql);
  modassert(validTraitSql, "error executing sql query");
  updateTraitConfig(movement, traitsResult);
  updateObjectProperties(id, traitsResult);
  updateSoundConfig(movement, id, SoundConfig {
    .jumpClip = traitsResult.at(0).at(9),
    .landClip = traitsResult.at(0).at(10),
  });
}
void reloadSettingsConfig(Movement& movement, std::string name){
  auto settingQuery = gameapi -> compileSqlQuery(
    "select xsensitivity, ysensitivity from settings where profile = " + name
  );
  bool validSettingsSql = false;
  auto settingsResult = gameapi -> executeSqlQuery(settingQuery, &validSettingsSql);
  modassert(validSettingsSql, "error executing sql query");
  movement.controlParams.xsensitivity = floatFromFirstSqlResult(settingsResult, 0);
  movement.controlParams.ysensitivity = floatFromFirstSqlResult(settingsResult, 1);
}


CScriptBinding movementBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Movement* movement = new Movement;
    movement -> goForward = false;
    movement -> goBackward = false;
    movement -> goLeft = false;
    movement -> goRight = false;

    movement -> jumpSoundObjId = std::nullopt;
    movement -> landSoundObjId = std::nullopt;

    movement -> lookVelocity = glm::vec2(0.f, 0.f);
    movement -> lastPosition = glm::vec3(0.f, 0.f, 0.f);
    movement -> xRot = 0.f;
    movement -> yRot = 0.f;
    movement -> groundedObjId = std::nullopt;

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
    movement -> lookVelocity = glm::vec2(xPos, yPos);
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    Movement* movement = static_cast<Movement*>(data);
    float moveSpeed = getMoveSpeed(*movement, false);
    float horzRelVelocity = 0.8f;
    if (movement -> goForward){
      auto moveVec = moveSpeed * glm::vec2(0.f, -1.f);
      moveXZ(id, moveVec);
    }
    if (movement -> goBackward){
      auto moveVec = moveSpeed * glm::vec2(0.f, 1.f);
      moveXZ(id, moveVec);
    }
    if (movement -> goLeft){
      auto moveVec = moveSpeed * glm::vec2(horzRelVelocity * -1.f, 0.f);
      moveXZ(id, moveVec);
    }
    if (movement -> goRight){
      auto moveVec = moveSpeed * glm::vec2(horzRelVelocity * 1.f, 0.f);
      moveXZ(id, moveVec);
    }
    float elapsedTime = gameapi -> timeElapsed();
    look(*movement, id, elapsedTime, false, 0.5f); // (look elapsedTime ironsight-mode ironsight-turn)
    updateVelocity(*movement, id, elapsedTime);

    //std::cout << movementToStr(*movement) << std::endl;
  };
  binding.onCollisionEnter = [](objid id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal) -> void {
    if (id != obj1 && id != obj2){
      return;
    }
    Movement* movement = static_cast<Movement*>(data);
    float yComponent = ((id == obj2) ? normal : oppositeNormal).y;
    objid otherObjectId = id == obj1 ? obj2 : obj1;
    if (yComponent <= 0){
      if (!movement -> groundedObjId.has_value()){
        land(*movement, id);
      }
      movement -> groundedObjId = otherObjectId;
    }
  };
  binding.onCollisionExit = [](objid id, void* data, int32_t obj1, int32_t obj2) -> void {
    Movement* movement = static_cast<Movement*>(data);
    auto otherObjectId = (id == obj1) ? obj2 : obj1;
    if (movement -> groundedObjId.has_value() && movement -> groundedObjId.value() == otherObjectId){
      movement -> groundedObjId = std::nullopt;
    }
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
    }
  };

  return binding;
}


