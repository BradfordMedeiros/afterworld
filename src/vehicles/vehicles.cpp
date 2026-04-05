#include "./vehicles.h"

extern CustomApiBindings* gameapi;

std::optional<objid> findChildObjBySuffix(objid id, const char* objName);
void disableEntity(objid id);
void reenableEntity(objid id, std::optional<glm::vec3> pos, std::optional<glm::quat> rot);

const bool SHOULD_SMOOTH_ANGLES = false;      
const bool SHOULD_SMOOTH_ZOOM = true;
const bool SHOULD_LIMIT_CAMERA = true;

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
        .targetAngleX = 0.f,
        .targetAngleY = 0.f,
        .actualDistanceFromTarget = -5.f,
        .additionalCameraOffset = glm::vec3(0.f, 1.f, 0.f),
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

    vehicles.vehicles.at(vehicleId).vehicle = doCreateVehicleBall(vehicleId, vehicles.vehicles.at(vehicleId).state);
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
    playGameplayClipById(vehicle.state.sound.value(), std::nullopt, std::nullopt, false); 
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
        onVehicleBallKey(*vehicleBall, key, action);
      }
    }
  }
}

void onVehicleMouseClick(Vehicles& vehicles, int button, int action, int mods){
  for (auto &[id, vehicle] :  vehicles.vehicles){
    if (vehicle.state.occupied.has_value()){
      VehicleBall* vehicleBall = std::get_if<VehicleBall>(&vehicle.vehicle);
      if (vehicleBall){
        onVehicleBallMouse(*vehicleBall, button, action, mods);
      }
    }
  }
}

float limitCameraDistance(objid id, glm::vec3 position, glm::quat rotation, glm::vec3 cameraPos, ThirdPersonCameraInfo& camera){
  if (!SHOULD_LIMIT_CAMERA){
    return camera.distanceFromTarget;
  }
  auto distance = glm::distance(position, cameraPos);
  auto hitobjects = gameapi -> raycast(position, rotation, -1 * distance, std::nullopt);

  float limitedDistance = camera.distanceFromTarget;
  for (auto& hitobject : hitobjects){
    //gameapi -> drawLine(hitobject.point, hitobject.point + glm::vec3(0.f, 10.f, 0.f), true, id, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, std::nullopt);
    auto distance = glm::distance(position, hitobject.point);
    if (camera.distanceFromTarget > 0){
      if (limitedDistance > distance){
        limitedDistance = (distance - 0.001f);
      }
    }else{
      // negative camera distance
      distance *= -1.f;
      if (limitedDistance < distance){
        limitedDistance = (distance + 0.001f);
      }
    }
  }

  std::cout << "limited distance: " << limitedDistance << std::endl;
  return limitedDistance;
}

void onVehicleFrame(Vehicles& vehicles, ControlParams& controlParams){
  for (auto &[id, vehicle] :  vehicles.vehicles){
    if (vehicle.state.occupied.has_value()){
      vehicle.state.managedCamera.distanceFromTarget += gameapi -> timeElapsed() * controlParams.zoom_delta * 20.f /*arbitary multiplier */;
      vehicle.state.managedCamera.targetAngleX += gameapi -> timeElapsed() * controlParams.lookVelocity.x;
      vehicle.state.managedCamera.targetAngleY += gameapi -> timeElapsed() * controlParams.lookVelocity.y;

      auto position  =  gameapi -> getGameObjectPos(id, true, "[gamelogic] get vehicle position for camera");
      auto camera = lookThirdPersonCalc(vehicle.state.managedCamera, id, false);
      auto adjustedDistance = limitCameraDistance(id, position, camera.rotation, camera.position, vehicle.state.managedCamera);

      auto deltaTime = gameapi -> timeElapsed();
      float smoothTime = 0.05f; // seconds to catch up
      float alpha = 1.0f - std::exp(-deltaTime / smoothTime);


      if (SHOULD_SMOOTH_ZOOM){
        vehicle.state.managedCamera.actualDistanceFromTarget = glm::mix(vehicle.state.managedCamera.actualDistanceFromTarget, adjustedDistance, alpha);
      }else{
        vehicle.state.managedCamera.actualDistanceFromTarget = adjustedDistance;
      }
      
      if (SHOULD_SMOOTH_ANGLES){
        vehicle.state.managedCamera.angleX = glm::mix(vehicle.state.managedCamera.angleX, vehicle.state.managedCamera.targetAngleX, alpha);
        vehicle.state.managedCamera.angleY = glm::mix(vehicle.state.managedCamera.angleY, vehicle.state.managedCamera.targetAngleY, alpha);
      }else{
        vehicle.state.managedCamera.angleX = vehicle.state.managedCamera.targetAngleX;
        vehicle.state.managedCamera.angleY = vehicle.state.managedCamera.targetAngleY;
      }

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

std::optional<VehicleBall*> getVehicleBall(Vehicles& vehicles, objid vehicleId){
  auto& vehicle = vehicles.vehicles.at(vehicleId);
  VehicleBall* vehicleBall = std::get_if<VehicleBall>(&vehicle.vehicle);
  if (vehicleBall){
    return vehicleBall;
  }
  return std::nullopt;
}