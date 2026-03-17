#include "./vehicles.h"

extern CustomApiBindings* gameapi;

std::optional<objid> findChildObjBySuffix(objid id, const char* objName);
void disableEntity(objid id);
void reenableEntity(objid id, std::optional<glm::vec3> pos, std::optional<glm::quat> rot);

Vehicles createVehicles(){
  return Vehicles{};
}

bool vehicleOccupied(Vehicles& vehicles, objid vehicleId){
  return vehicles.vehicles.at(vehicleId).state.occupied.has_value();
}

void addVehicle(Vehicles& vehicles, objid vehicleId, bool isShip){
  auto soundId = findChildObjBySuffix(vehicleId, "sound");

  vehicles.vehicles[vehicleId] = Vehicle{
    .state = VehicleState {
      .controls = glm::vec3(0.f, -9.81f, 0.f),
      .managedCamera = ThirdPersonCameraInfo {
        .thirdPersonMode = false,
        .distanceFromTarget = -5.f,
        .angleX = 0.f,
        .angleY = 0.f,
        .actualDistanceFromTarget = -5.f,
        .additionalCameraOffset = glm::vec3(0.f, 2.5f, 0.f),
        .zoomOffset = glm::vec3(0.f, 0.f, 0.f),
        .actualZoomOffset = glm::vec3(0.f, 0.f, 0.f),
        .reverseCamera = false,
      },
      .sound = soundId,
    },
  };
  if (isShip){
    vehicles.vehicles.at(vehicleId).vehicle = VehicleShip{};
  }else{
    vehicles.vehicles.at(vehicleId).vehicle = doCreateVehicleBall(vehicleId);
  }

  modlog("vehicle add vehicle", std::to_string(vehicleId) + ", sound = " + print(soundId));
}

void removeVehicle(Vehicles& vehicles, objid vehicleId){
  vehicles.vehicles.erase(vehicleId);
  modlog("vehicle remove vehicle", std::to_string(vehicleId));
}

void enterVehicle(Vehicles& vehicles, objid vehicleId, objid id){
  modassert(vehicles.vehicles.find(vehicleId) != vehicles.vehicles.end(), "vehicle does not exist");

  Vehicle& vehicle = vehicles.vehicles.at(vehicleId);
  bool occupied = vehicle.state.occupied.has_value();
  modassert(!occupied, "vehicle already occupied");
  vehicles.vehicles.at(vehicleId).state.occupied = id;

  modlog("vehicle enter vehicle", std::to_string(vehicleId));

  if (vehicle.state.sound.has_value()){
    playGameplayClipById(vehicle.state.sound.value(), std::nullopt, std::nullopt); 
  }
  disableEntity(vehicle.state.occupied.value());
}

void exitVehicle(Vehicles& vehicles, objid vehicleId, objid id){
  //vehicles.vehicles.at(vehicleId).occupied = false;
  modlog("vehicle exit vehicle", std::to_string(vehicleId));
  Vehicle& vehicle = vehicles.vehicles.at(vehicleId);

  auto position = gameapi -> getGameObjectPos(vehicleId, true, "[gamelogic] exit vehicle get pos") + glm::vec3(0.f, 30.f, 0.f);
  auto vehicleRot = gameapi -> getGameObjectRotation(vehicleId, true, "[gamelogic] vehicle exit get rot"); 

  reenableEntity(vehicle.state.occupied.value(), position, vehicleRot);

  vehicle.state.occupied = std::nullopt;
  if (vehicle.state.sound.has_value()){
    gameapi -> stopClipById(vehicle.state.sound.value());
  }
}

void onVehicleKey(Vehicles& vehicles, int key, int action){
  for (auto &[id, vehicle] :  vehicles.vehicles){
    if (vehicle.state.occupied.has_value()){
      VehicleBall* vehicleBall = std::get_if<VehicleBall>(&vehicle.vehicle);
      if (vehicleBall){
        if(isJumpKey(key) /* space */ && action == 1){
          vehicleBall -> shouldJump = true;
        }
        if (isInteractKey(key) && action == 1){
          vehicleBall -> shouldUsePowerUp = true;
        }
      }
    }
  }
}

void onVehicleFrame(Vehicles& vehicles, ControlParams& controlParams){
  for (auto &[id, vehicle] :  vehicles.vehicles){
    if (vehicle.state.occupied.has_value()){
      vehicle.state.managedCamera.angleX += gameapi -> timeElapsed() * controlParams.lookVelocity.x;
      vehicle.state.managedCamera.angleY += gameapi -> timeElapsed() * controlParams.lookVelocity.y;
      vehicle.state.managedCamera.actualDistanceFromTarget += gameapi -> timeElapsed() * controlParams.zoom_delta * 20.f /*arbitary multiplier */;
    }

    {
      VehicleShip* vehicleShip = std::get_if<VehicleShip>(&vehicle.vehicle);
      if (vehicleShip){
        onVehicleFrameShip(id, vehicle.state, *vehicleShip, controlParams);
      }
    }
    {
      VehicleBall* vehicleBall = std::get_if<VehicleBall>(&vehicle.vehicle);
      if (vehicleBall){
        onVehicleFrameBall(id, vehicle.state, *vehicleBall, controlParams);
      }
    }
  }
}

bool isVehicle(Vehicles& vehicles, objid id){
  return vehicles.vehicles.find(id) != vehicles.vehicles.end();
}


void setPowerupBall(Vehicles& vehicles, objid vehicleId, std::optional<BallPowerup> powerup){
  auto& vehicle = vehicles.vehicles.at(vehicleId);
  VehicleBall* vehicleBall = std::get_if<VehicleBall>(&vehicle.vehicle);
  modassert(vehicleBall, "vehicle is not a ball");
  vehicleBall -> powerup = powerup;
}

std::optional<BallPowerup> getBallPowerup(Vehicles& vehicles, objid vehicleId){
  auto& vehicle = vehicles.vehicles.at(vehicleId);
  VehicleBall* vehicleBall = std::get_if<VehicleBall>(&vehicle.vehicle);
  modassert(vehicleBall, "vehicle is not a ball");
  return vehicleBall -> powerup;
}
