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

  float xRotDegrees;
  float yRotDegrees;
  float distance;
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

glm::vec3 offsetFromParams(float xRot, float yRot, float distance){
  auto forwardRot = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
  auto rotation = gameapi -> setFrontDelta(forwardRot, xRot, yRot, 0, 1.f);
  auto offsetVec = rotation * glm::vec3(0.f, 0.f, -1.f);
  return distance * offsetVec;
}

void setVehicleCamera(Vehicle& vehicle){
  auto currCameraOffset = gameapi -> getGameObjectPos(vehicle.cameraId.value(), true);
  auto currVehiclePos = gameapi -> getGameObjectPos(vehicle.vehicleId.value(), true);
  auto cameraTowardVehicle = gameapi -> orientationFromPos(currCameraOffset, currVehiclePos);
  auto cameraOffset = offsetFromParams(vehicle.xRotDegrees, vehicle.yRotDegrees, vehicle.distance);
  gameapi -> setGameObjectPosRelative(vehicle.cameraId.value(), cameraOffset);
  gameapi -> setGameObjectRot(vehicle.cameraId.value(), cameraTowardVehicle);
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
    .vecAttr = {  .vec3 = { { "position", cameraOffset }},  .vec4 = {} },
  };

  auto cameraId = gameapi -> makeObjectAttr(sceneId, ">code-vehicle-cam", camAttr, submodelAttributes);
  gameapi -> makeParent(cameraId.value(), vehicleId.value());
  vehicle.cameraId = cameraId;

  vehicle.speed = speed;
  vehicle.xRotDegrees = 0.f;
  vehicle.yRotDegrees = 0.f;
  vehicle.distance = 15.f; 

  setVehicleCamera(vehicle);
}


void moveVehicle(Vehicle& vehicle, objid id, glm::vec3 direction){
  if (vehicle.vehicleId.has_value()){
    std::cout << "move vehicle: " << print(direction) << std::endl;
    float time = gameapi -> timeElapsed();
    gameapi -> applyImpulseRel(vehicle.vehicleId.value(), time * glm::vec3(direction.x, direction.y, direction.z));
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
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    handleInput(vehicle -> input, key, action);
    if (key == 'E' && action == 1){
      gameapi -> sendNotifyMessage("request:release-control", std::to_string(vehicle -> cameraId.value()));
      vehicle -> active = false;
    }
  };

  binding.onFrame = [](int32_t id, void* data) -> void {
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    if (!vehicle -> active){
      return;
    }
    if (vehicle -> input.goForward){
      auto moveVec = vehicle -> speed * glm::vec3(0.f, 0.f, -1.f);
      moveVehicle(*vehicle, id, moveVec);
    }
    if (vehicle -> input.goBackward){
      auto moveVec = vehicle -> speed * glm::vec3(0.f, 0.f, 1.f);
      moveVehicle(*vehicle, id, moveVec);
    }
    if (vehicle -> input.goLeft){
      auto moveVec = vehicle -> speed * glm::vec3(-1.f, 0.f, 0.f);
      moveVehicle(*vehicle, id, moveVec);
    }
    if (vehicle -> input.goRight){
      auto moveVec = vehicle -> speed * glm::vec3(1.f, 0.f, 0.f);
      moveVehicle(*vehicle, id, moveVec);
    }
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
    Vehicle* vehicle = static_cast<Vehicle*>(data);

    if (key == "selected"){  // maybe this logic should be somewhere else and not be in dialog
      if (vehicle -> vehicleId.has_value()){
        auto strValue = std::get_if<std::string>(&value); 
        modassert(strValue != NULL, "selected value invalid");
        auto gameObjId = std::atoi(strValue -> c_str());

        std::cout << "vehicle selected: " << gameObjId << ", " << vehicle -> vehicleId.value() << std::endl;
        if (gameObjId == vehicle -> vehicleId.value()){
          gameapi -> sendNotifyMessage("request:change-control", std::to_string(vehicle -> cameraId.value()));
          vehicle -> active = true;
        }
      }
    }
  };

  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    float xDegrees = xPos / 360.f;
    float yDegrees = yPos / 360.f;
    vehicle -> xRotDegrees += xDegrees * 0.2f;
    vehicle -> yRotDegrees += yDegrees * 0.2f;
    setVehicleCamera(*vehicle);
  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    vehicle -> distance += amount;
    setVehicleCamera(*vehicle);
  };

  return binding;
}
