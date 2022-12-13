#include "./movement.h"

extern CustomApiBindings* gameapi;

bool debugAssertions = false;
void assertForNow(bool valid, const char* message){
  if (debugAssertions){
    modassert(valid, message);
  }
}

// speed, speed-air, jump-height, gravity, restitution, friction, max-angleup, max-angledown, mass, jump-sound, land-sound, dash, dash-sound from traits
// "select xsensitivity, ysensitivity from settings

struct MovementParams {
  float moveSpeed;
  float moveSpeedAir;
  float jumpHeight;
};

struct ControlParams {
  float xsensitivity;
  float ysensitivity;
};

struct Movement {
  MovementParams moveParams;
  bool goForward;
  bool goBackward;
  bool goLeft;
  bool goRight;


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
  std::cout << "jump placeholder" << std::endl;
  glm::vec3 impulse(0, movement.moveParams.jumpHeight, 0);
  if (movement.groundedObjId.has_value()){
    gameapi -> applyImpulse(id, impulse);
    assertForNow(false, "need to add play clip on jump"); //     (if jump-obj-name (playclip jump-obj-name))  
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
  auto speed = glm::length(displacement);
  movement.lastPosition = currPos;
  //std::cout << "velocity = " << print(displacement) << ", speed = " << speed << std::endl;
  gameapi -> sendNotifyMessage("velocity", serializeVec(displacement));
}

float PI = 3.141592;
float TWO_PI = 2 * PI;

float clampPi(float value){
  /*(define (clamp-pi value)
  (if (>= value 0)
    (let* ((numtimes (floor (/ value 2pi))) (remain (- value (* 2pi numtimes))))
      (if (> remain pi) (- remain 2pi) remain)
    )
    (let* ((numtimes (floor (/ value (* -1 2pi)))) (remain (+ value (* 2pi numtimes))))
      (if (< remain (* -1 pi)) (+ remain 2pi) remain)
    )
  )
)*/
  return value;
  if (value > 0){
    int numTimes = glm::floor(value / TWO_PI);
    float remain =  value - (TWO_PI * numTimes);
    return (remain > PI) ? (- remain - TWO_PI) : remain;
  }

  int numTimes = glm::floor(value / (-1 * TWO_PI));
  float remain = value + (TWO_PI * numTimes);
  return (remain < (-1 * PI)) ? (remain + TWO_PI) : remain;
}


float maxAngleUp = -1.8f;
float maxAngleDown = 1.1f;

void look(Movement& movement, objid id, float elapsedTime, bool ironsight, float ironsight_turn){
  float xsensitivity = 1.f;
  float ysensitivity = 1.f;
  auto forwardVec = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));

  float raw_deltax = movement.lookVelocity.x * xsensitivity * elapsedTime;
  float raw_deltay = -1.f * movement.lookVelocity.y * ysensitivity * elapsedTime; 

  float deltax = ironsight ? (raw_deltax * ironsight_turn) : raw_deltax;
  float deltay = ironsight ? (raw_deltay * ironsight_turn) : raw_deltay;

  float targetXRot = clampPi(movement.xRot + deltax);
  float targetYRot = clampPi(movement.yRot + deltay);

  movement.xRot = targetXRot;
  movement.yRot = glm::min(maxAngleDown, glm::max(maxAngleUp, targetYRot));

  std::cout << "rot x = " << movement.xRot << ", " << " y = " << movement.yRot << std::endl;

  auto rotation = gameapi -> setFrontDelta(forwardVec, movement.xRot, movement.yRot, 0, 0.1f);
  gameapi -> setGameObjectRot(id, rotation);

  movement.lookVelocity = glm::vec2(0.f, 0.f);
}

void land(){
  //(define (land) (if land-obj-name (playclip land-obj-name)))
  assertForNow(false, "land not yet implemented");
}


float floatFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index){
  auto value = sqlResult.at(0).at(index);
  float number = 0.f;
  bool isFloat = maybeParseFloat(value, number);
  modassert(isFloat, "invalid float number");
  return number;
}

CScriptBinding movementBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Movement* movement = new Movement;
    movement -> goForward = false;
    movement -> goBackward = false;
    movement -> goLeft = false;
    movement -> goRight = false;

    movement -> lookVelocity = glm::vec2(0.f, 0.f);
    movement -> lastPosition = glm::vec3(0.f, 0.f, 0.f);
    movement -> xRot = 0.f;
    movement -> yRot = 0.f;
    movement -> groundedObjId = std::nullopt;

    ///Update config ////////////////
    auto query = gameapi -> compileSqlQuery(
      "select speed, speed-air, jump-height, gravity, restitution, friction, max-angleup, max-angledown, mass, jump-sound, land-sound, dash, dash-sound from traits"
    );
    bool validSql = false;
    auto result = gameapi -> executeSqlQuery(query, &validSql);
    modassert(validSql, "error executing sql query");
    movement -> moveParams.moveSpeed = floatFromFirstSqlResult(result, 0);
    movement -> moveParams.moveSpeedAir = floatFromFirstSqlResult(result, 1);
    movement -> moveParams.jumpHeight = floatFromFirstSqlResult(result, 2);
  
    /////
    return movement;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Movement* value = (Movement*)data;
    delete value;
  };
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    std::cout << "key: " << key << std::endl;
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
    std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    Movement* movement = static_cast<Movement*>(data);
    movement -> lookVelocity.x = xPos;
    movement -> lookVelocity.y = yPos;
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
        land();
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



  return binding;
}


