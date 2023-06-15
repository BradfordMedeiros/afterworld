#include "./vehicle.h"

extern CustomApiBindings* gameapi;

struct Input {
  bool goForward;
  bool goBackward;
  bool goLeft;
  bool goRight;
};


struct Vehicle {
  bool active;
  std::optional<objid> vehicleId;
  std::optional<objid> cameraId;
  Input input;

  float speed;

  float xRot;
  float yRot;
  float distance;

  glm::vec3 cameraOffset;
  std::optional<float> minXRot;
  std::optional<float> maxXRot;
  std::optional<float> minYRot;
  std::optional<float> maxYRot;

  std::optional<glm::vec2> wheelAngle;

};

void handleInput(Input& input, int key, int action){
  if (key == 'W'){
    if (action == 0){
      input.goForward = false;
    }else if (action == 1){
      input.goForward = true;
    }
    return;
  }
  if (key == 'S'){
    if (action == 0){
      input.goBackward = false;
    }else if (action == 1){
      input.goBackward = true;
    }
    return;
  }
  if (key == 'A'){
    if (action == 0){
      input.goLeft = false;
    }else if (action == 1){
      input.goLeft = true;
    }
    return;
  }
  if (key == 'D'){
    if (action == 0){
      input.goRight = false;
    }else if (action == 1){
      input.goRight = true;
    }
    return;
  }
}

glm::quat rotationFromAngle(float xRot, float yRot){
  auto forwardRot = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
  auto rotation = gameapi -> setFrontDelta(forwardRot, xRot, yRot, 0, 1.f);
  return rotation;
}

glm::vec3 offsetFromParams(float xRot, float yRot, float distance){
  auto rotation = rotationFromAngle(xRot, yRot);
  auto offsetVec = rotation * glm::vec3(0.f, 0.f, 1.f);
  return distance * offsetVec;
}

void setVehicleCamera(Vehicle& vehicle){
  if (!vehicle.active){
    return;
  }
  auto currCameraOffset = gameapi -> getGameObjectPos(vehicle.cameraId.value(), true);
  auto currVehiclePos = gameapi -> getGameObjectPos(vehicle.vehicleId.value(), true) + vehicle.cameraOffset;
  auto cameraTowardVehicle = gameapi -> orientationFromPos(currCameraOffset, currVehiclePos);
  auto cameraOffset = offsetFromParams(vehicle.xRot, vehicle.yRot, vehicle.distance);

  std::cout << "rot: " << print(glm::vec2(vehicle.xRot, vehicle.yRot)) << std::endl;
  gameapi -> setGameObjectPosition(vehicle.cameraId.value(), cameraOffset + vehicle.cameraOffset, false);
  gameapi -> setGameObjectRot(vehicle.cameraId.value(), cameraTowardVehicle, false);
}

void createVehicle(Vehicle& vehicle, std::string name, objid sceneId, glm::vec3 position){
  auto query = gameapi -> compileSqlQuery(std::string("select model, speed, camera_offset, physics_angle, physics_linear from vehicles where name = ") +  name, {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "query vehicle params invalid query");
  modassert(result.size() == 1, "invalid query result from vehicles");
  
  auto model = strFromFirstSqlResult(result, 0);
  auto speed = floatFromFirstSqlResult(result, 1);
  auto cameraOffset = vec3FromFirstSqlResult(result, 2);
  auto physicsAngle = vec3FromFirstSqlResult(result, 3);
  auto physicsLinear = vec3FromFirstSqlResult(result, 4);

  float mass = 10.f;


  GameobjAttributes attr {
    .stringAttributes = { { "mesh", model }, { "physics", "enabled" }, { "physics_type", "dynamic" } },
    .numAttributes = { { "mass", mass }},
    .vecAttr = {  .vec3 = { { "position", position }, { "physics_angle", physicsAngle }, { "physics_linear", physicsLinear }},  .vec4 = {} },
  };

  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto vehicleId = gameapi -> makeObjectAttr(sceneId, "code-vehicle", attr, submodelAttributes);
  vehicle.vehicleId = vehicleId;

  GameobjAttributes camAttr {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = {  .vec3 = {},  .vec4 = {} },
  };

  auto cameraId = gameapi -> makeObjectAttr(sceneId, ">code-vehicle-cam", camAttr, submodelAttributes);
  gameapi -> makeParent(cameraId.value(), vehicleId.value());
  vehicle.cameraId = cameraId;

  vehicle.speed = speed;
  vehicle.xRot = 0.f;
  vehicle.yRot = 0.f;
  vehicle.distance = 15.f; 

  vehicle.cameraOffset = cameraOffset;
  vehicle.minXRot = -2.f;
  vehicle.maxXRot = 2.f;
  vehicle.minYRot = 0.f;
  vehicle.maxYRot = 1.f;

  //vehicle.wheelAngle = glm::vec2(0.f, 0.f);
  vehicle.wheelAngle = std::nullopt;

  setVehicleCamera(vehicle);
}


void moveVehicle(Vehicle& vehicle, objid id, glm::vec3 direction, std::optional<glm::quat> wheelRotation){
  if (vehicle.vehicleId.has_value()){
    //std::cout << "move vehicle: " << print(direction) << std::endl;
    float time = gameapi -> timeElapsed();
    gameapi -> applyImpulseRel(vehicle.vehicleId.value(), time * glm::vec3(direction.x, direction.y, direction.z));

    // should limit turn speed
    if (wheelRotation.has_value()){
      gameapi -> setGameObjectRot(vehicle.vehicleId.value(), wheelRotation.value(), false);
    }
    
  }
}

CScriptBinding vehicleBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Vehicle* vehicle = new Vehicle;

    vehicle -> active = false;
    vehicle -> vehicleId = std::nullopt;
    vehicle -> cameraId = std::nullopt;

    vehicle -> input.goForward = false;
    vehicle -> input.goBackward = false;
    vehicle -> input.goLeft = false;
    vehicle -> input.goRight = false;

    vehicle -> speed = 1.f;

    createVehicle(*vehicle, "digmole", sceneId, glm::vec3(0.f, -10.f, 50.f));

    return vehicle;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    delete vehicle;
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    if (isPaused()){
      return;
    }
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    handleInput(vehicle -> input, key, action);
    if (key == 'E' && action == 1){
      gameapi -> sendNotifyMessage("request:release-control", vehicle -> cameraId.value());
      vehicle -> active = false;

      vehicle -> xRot = 0.f;
      vehicle -> yRot = 0.f;
      if (vehicle -> wheelAngle.has_value()){
        vehicle -> wheelAngle.value().x = 0.f;
        vehicle -> wheelAngle.value().y = 0.f;
      }
    }
  };

  binding.onFrame = [](int32_t id, void* data) -> void {
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    if (!vehicle -> active){
      return;
    }
    if (vehicle -> input.goForward){
      auto moveVec = vehicle -> speed * glm::vec3(0.f, 0.f, -1.f);
      std::optional<glm::quat> wheelRotation = std::nullopt;
      if (vehicle -> wheelAngle.has_value()){
        wheelRotation = rotationFromAngle(vehicle -> wheelAngle.value().x, vehicle -> wheelAngle.value().y);
        moveVec = wheelRotation.value() * moveVec;
      }
      moveVehicle(*vehicle, id, moveVec, wheelRotation);
    }
    if (vehicle -> input.goBackward){
      auto moveVec = vehicle -> speed * glm::vec3(0.f, 0.f, 1.f);
      std::optional<glm::quat> wheelRotation = std::nullopt;
      if (vehicle -> wheelAngle.has_value()){
        wheelRotation = rotationFromAngle(vehicle -> wheelAngle.value().x, vehicle -> wheelAngle.value().y);
        moveVec = wheelRotation.value() * moveVec;
      }
      moveVehicle(*vehicle, id, moveVec, wheelRotation);
    }
    if (vehicle -> input.goLeft){
      if (!vehicle -> wheelAngle.has_value()){
        auto moveVec = vehicle -> speed * glm::vec3(-1.f, 0.f, 0.f);
        moveVehicle(*vehicle, id, moveVec, std::nullopt);
      }
    }
    if (vehicle -> input.goRight){
      if (!vehicle -> wheelAngle.has_value()){
        auto moveVec = vehicle -> speed * glm::vec3(1.f, 0.f, 0.f);
        moveVehicle(*vehicle, id, moveVec, std::nullopt);
      }
    }
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    Vehicle* vehicle = static_cast<Vehicle*>(data);

    if (key == "selected"){  // maybe this logic should be somewhere else and not be in dialog
      if (vehicle -> vehicleId.has_value()){
        auto gameObjId = anycast<objid>(value); 
        modassert(gameObjId != NULL, "vehicle - selected value invalid");
        std::cout << "vehicle selected: " << *gameObjId << ", " << vehicle -> vehicleId.value() << std::endl;
        if (*gameObjId == vehicle -> vehicleId.value()){
          gameapi -> sendNotifyMessage("request:change-control", vehicle -> cameraId.value());
          vehicle -> active = true;

          vehicle -> xRot = 0.f;
          vehicle -> yRot = 0.f;
          if (vehicle -> wheelAngle.has_value()){
            vehicle -> wheelAngle.value().x = 0.f;
            vehicle -> wheelAngle.value().y = 0.f;
          }
          setVehicleCamera(*vehicle);
        }
      }
    }
  };

  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void {
    if (isPaused()){
      return;
    }
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    float xRadians = xPos / 400.f;
    float yRadians = yPos / 400.f;
    vehicle -> xRot = limitAngle(vehicle -> xRot + xRadians * 0.2f, vehicle -> minXRot, vehicle -> maxXRot);
    vehicle -> yRot = limitAngle(vehicle -> yRot + yRadians * 0.2f, vehicle -> minYRot, vehicle -> maxYRot);

    if (vehicle -> wheelAngle.has_value()){
      vehicle -> wheelAngle.value().x += xRadians * 0.2f;
      std::cout << "wheel angle: " << print(vehicle -> wheelAngle.value()) << std::endl;
    }
    setVehicleCamera(*vehicle);

  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    if (isPaused()){
      return;
    }
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    vehicle -> distance += amount;
    setVehicleCamera(*vehicle);
  };

  return binding;
}
