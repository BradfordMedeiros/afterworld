#include "./movement.h"

extern CustomApiBindings* gameapi;

bool debugAssertions = false;
void assertForNow(bool valid, const char* message){
  if (debugAssertions){
    modassert(valid, message);
  }
}

struct MovementParams {

};

struct Movement {
  bool goForward;
  bool goBackward;
  bool goLeft;
  bool goRight;


  glm::vec2 lookVelocity;
  float xRot; // up and down
  float yRot; // left and right
  std::optional<objid> groundedObjId;

  float currentSpeed;
};

std::string movementToStr(Movement& movement){
  std::string str;
  str += std::string("goForward: ") + (movement.goForward ? "true" : "false") + "\n";
  str += std::string("goBackward: ") + (movement.goBackward ? "true" : "false") + "\n";
  str += std::string("goLeft: ") + (movement.goLeft ? "true" : "false") + "\n";
  str += std::string("goRight: ") + (movement.goRight ? "true" : "false") + "\n";
  return str;
}


void populateMovementParams(Movement& movement){
  //auto query = gameapi -> compileSqlQuery("select filepath, name from levels");
  //bool validSql = false;
  //auto result = gameapi -> executeSqlQuery(query, &validSql);
  //modassert(validSql, "error executing sql query");
  //std::vector<Level> levels = {};
  //for (auto &row : result){
  //  levels.push_back(Level {
  //    .scene = row.at(0),
  //    .name = row.at(1),
  //  });
  //}
}

void jump(Movement& movement, objid id){
  std::cout << "jump placeholder" << std::endl;
  float jumpHeight = 10.f;
  glm::vec3 impulse(0, jumpHeight, 0);
  if (movement.groundedObjId.has_value()){
    gameapi -> applyImpulse(id, impulse);
    assertForNow(false, "need to add play clip on jump"); //     (if jump-obj-name (playclip jump-obj-name))  
  }
}


void dash(Movement& movement){
  std::cout << "dash placeholder" << std::endl;
}
void moveXZ(objid id, glm::vec2 direction){
  std::cout << "move placeholder" << std::endl;
  float time = gameapi -> timeElapsed();
  gameapi -> applyImpulseRel(id, time * glm::vec3(direction.x, 0.f, direction.y));
}

float getMoveSpeed(){
  //(define (get-move-speed)
  //(define basespeed (if is-grounded movement-speed movement-speed-air))
  //(if ironsight-mode (* ironsight-speed basespeed) basespeed)
  //)
  return 20.f;
}

void updateVelocity(){
  /*(define (update-velocity elapsedTime)
  (define currpos (gameobj-pos mainobj))
  (set! velocity (calc-velocity elapsedTime currpos lastpos))
  (set! lastpos currpos)
  ; todo, sendnotify should be able to send any type
  ;(format #t "velocity is: ~a\n" velocity)
  (sendnotify "velocity" (
    string-join 
    (list 
      (number->string (car velocity))
      (number->string (cadr velocity))
      (number->string (caddr velocity))
    ) 
    " "
  ))
  )*/
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


float maxAngleUp = -1.8f;
float maxAngleDown = 1.1f;

void look(Movement& movement, objid id, float elapsedTime, bool ironsight, float ironsight_turn){
  float xsensitivity = 20.f;
  float ysensitivity = 20.f;
  auto forwardVec = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));

  float raw_deltax = movement.lookVelocity.x * xsensitivity * elapsedTime;
  float raw_deltay = -1.f * movement.lookVelocity.y * ysensitivity * elapsedTime; 

  float deltax = ironsight ? (raw_deltax * ironsight_turn) : raw_deltax;
  float deltay = ironsight ? (raw_deltay * ironsight_turn) : raw_deltay;

  float targetXRot = clampPi(movement.xRot * deltax);
  float targetYRot = clampPi(movement.yRot * deltay);

  movement.xRot = targetXRot;
  movement.yRot = glm::min(maxAngleDown, glm::max(maxAngleUp, targetYRot));

  auto rotation = gameapi -> setFrontDelta(forwardVec, movement.xRot, movement.yRot, 0, 1.f);
  gameapi -> setGameObjectRot(id, rotation);

  movement.lookVelocity = glm::vec2(0.f, 0.f);
  movement.xRot = 0.f;
  movement.yRot = 0.f;
}

void land(){
  //(define (land) (if land-obj-name (playclip land-obj-name)))
  assertForNow(false, "land not yet implemented");
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

    movement -> currentSpeed = 0.f;
    movement -> xRot = 0.f;
    movement -> yRot = 0.f;
    movement -> groundedObjId = std::nullopt;

    populateMovementParams(*movement);
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
    float moveSpeed = getMoveSpeed();
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
    look(*movement, id, gameapi -> timeElapsed(), false, 0.5f); // (look elapsedTime ironsight-mode ironsight-turn)
    updateVelocity();
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


