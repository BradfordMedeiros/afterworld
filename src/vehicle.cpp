#include "./vehicle.h"

extern CustomApiBindings* gameapi;

struct Input {
  bool goForward;
  bool goBackward;
  bool goLeft;
  bool goRight;
};



struct Vehicle {
  std::optional<objid> vehicleId;
  Input input;
  float speed;
};

void handleInput(Input& input, int key, int action){
  if (key == 265){
    if (action == 0){
      input.goForward = false;
    }else if (action == 1){
      input.goForward = true;
    }
    return;
  }
  if (key == 264){
    if (action == 0){
      input.goBackward = false;
    }else if (action == 1){
      input.goBackward = true;
    }
    return;
  }
  if (key == 263){
    if (action == 0){
      input.goLeft = false;
    }else if (action == 1){
      input.goLeft = true;
    }
    return;
  }
  if (key == 262){
    if (action == 0){
      input.goRight = false;
    }else if (action == 1){
      input.goRight = true;
    }
    return;
  }
}


void createVehicle(Vehicle& vehicle, std::string name, objid sceneId, glm::vec3 position){
  auto query = gameapi -> compileSqlQuery(std::string("select model, speed, camera_offset, physics_angle, physics_linear from vehicles where name = ") +  name);
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


  vehicle.speed = speed;


  //#bot:physics_angle:0 1 0
  //bot:physics_mass:0.1
  //bot:script:native/vehicle  

}

void moveVehicle(Vehicle& vehicle, objid id, glm::vec3 direction){
  if (vehicle.vehicleId.has_value()){
    float time = gameapi -> timeElapsed();
    gameapi -> applyImpulseRel(vehicle.vehicleId.value(), time * glm::vec3(direction.x, direction.y, direction.z));
  }
}

CScriptBinding vehicleBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Vehicle* vehicle = new Vehicle;

    vehicle -> vehicleId = std::nullopt;

    vehicle -> input.goForward = false;
    vehicle -> input.goBackward = false;
    vehicle -> input.goLeft = false;
    vehicle -> input.goRight = false;

    vehicle -> speed = 1.f;

    createVehicle(*vehicle, "digmole", sceneId, glm::vec3(0.f, -10.f, 50.f));

    return vehicle;
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    Vehicle* vehicle = static_cast<Vehicle*>(data);
    handleInput(vehicle -> input, key, action);
  };

  binding.onFrame = [](int32_t id, void* data) -> void {
    Vehicle* vehicle = static_cast<Vehicle*>(data);
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

  return binding;
}
