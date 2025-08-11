#include "./vehicles.h"

extern CustomApiBindings* gameapi;

Vehicles createVehicles(){
  return Vehicles{};
}

bool vehicleOccupied(Vehicles& vehicles, objid vehicleId){
  return vehicles.vehicles.at(vehicleId).occupied;
}

void addVehicle(Vehicles& vehicles, objid vehicleId){
  vehicles.vehicles[vehicleId] = Vehicle{
    .controls = glm::vec3(0.f, -9.81f, 0.f),
    .angleControls = glm::vec2(0.f, 0.f),
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
  };
  modlog("vehicle add vehicle", std::to_string(vehicleId));
}

void removeVehicle(Vehicles& vehicles, objid vehicleId){
  vehicles.vehicles.erase(vehicleId);
  modlog("vehicle remove vehicle", std::to_string(vehicleId));
}

void enterVehicle(Vehicles& vehicles, objid vehicleId, objid id){
  modassert(vehicles.vehicles.find(vehicleId) != vehicles.vehicles.end(), "vehicle does not exist");
  bool occupied = vehicles.vehicles.at(vehicleId).occupied;
  modassert(!occupied, "vehicle already occupied");
  vehicles.vehicles.at(vehicleId).occupied = true;
  modlog("vehicle enter vehicle", std::to_string(vehicleId));
}

void exitVehicle(Vehicles& vehicles, objid vehicleId, objid id){
  //vehicles.vehicles.at(vehicleId).occupied = false;
  modlog("vehicle exit vehicle", std::to_string(vehicleId));
  vehicles.vehicles.at(vehicleId).occupied = false;
}

void onVehicleKey(Vehicles& vehicles, int key, int action){
}

void onVehicleFrame(Vehicles& vehicles, ControlParams& controlParams){
  for (auto &[id, vehicle] :  vehicles.vehicles){
    if (vehicle.occupied){

      // Would be nice to make this work more similarly to movement system but this is simple so
      vehicle.managedCamera.angleX += gameapi -> timeElapsed() * controlParams.lookVelocity.x;
      vehicle.managedCamera.angleY += gameapi -> timeElapsed() * controlParams.lookVelocity.y;
      vehicle.managedCamera.actualDistanceFromTarget += gameapi -> timeElapsed() * controlParams.zoom_delta * 20.f /*arbitary multiplier */;
      auto thirdPerson = lookThirdPersonCalc(vehicle.managedCamera, id);

      auto oldRotation = gameapi -> getGameObjectRotation(id, true, "[gamelogic] vehicle velocity rot"); 

      gameapi -> setGameObjectRot(id, thirdPerson.rotation, true, Hint { .hint = "[gamelogic] lateUpdate - set vehicle rotn" }); // shouldn't be instant

      glm::vec3 newVelocityDiff(0.f, 0.f, 0.f);
      if (controlParams.goForward){
        newVelocityDiff.z -= 100.f * gameapi -> timeElapsed();
      }
      if (controlParams.goBackward){
        newVelocityDiff.z += 100.f * gameapi -> timeElapsed();
      }
      if (controlParams.goLeft){
        newVelocityDiff.x -= 100.f * gameapi -> timeElapsed();
      }
      if (controlParams.goRight){
        newVelocityDiff.x += 100.f * gameapi -> timeElapsed();
      }
      //if (controlParams.shiftModifier){
      //  newVelocityDiff.y += 100.f * gameapi -> timeElapsed();
      //}

      auto diffRot = thirdPerson.rotation * glm::inverse(oldRotation); // I shouldn't apply all of it, but how much?
      //auto diffRot = glm::inverse(thirdPerson.rotation) * oldRotationdf; // I shouldn't apply all of it, but how much?

      auto newVelocity = (diffRot * vehicle.controls) + (thirdPerson.rotation * newVelocityDiff);
      vehicle.controls = newVelocity;


    }
  }

  for (auto &[id, vehicle] :  vehicles.vehicles){
    auto oldPos = gameapi -> getGameObjectPos(id, true, "vehicle getPos");
    if (vehicle.occupied){
      setGameObjectVelocity(id, vehicle.controls);
    }
    
    bool slowDown = controlParams.shiftModifier;
    float ratePerSecond = 100.f;
    float amountThisFrame = ratePerSecond * gameapi -> timeElapsed();
    if (slowDown){
      if (vehicle.controls.x > 0){
        vehicle.controls.x -= amountThisFrame;
        if (vehicle.controls.x < 0){
          vehicle.controls.x = 0;
        }
      }
      if (vehicle.controls.x < 0){
        vehicle.controls.x += amountThisFrame;
        if (vehicle.controls.x > 0){
          vehicle.controls.x = 0;
        }
      }
      if (vehicle.controls.y > 0){
        vehicle.controls.y -= amountThisFrame;
        if (vehicle.controls.y < 0){
          vehicle.controls.y = 0;
        }
      }
      if (vehicle.controls.y < 0){
        vehicle.controls.y += amountThisFrame;
        if (vehicle.controls.y > 0){
          vehicle.controls.y = 0;
        }
      }
      if (vehicle.controls.z > 0){
        vehicle.controls.z -= amountThisFrame;
        if (vehicle.controls.z < 0){
          vehicle.controls.z = 0;
        }
      }
      if (vehicle.controls.z < 0){
        vehicle.controls.z += amountThisFrame;
        if (vehicle.controls.z > 0){
          vehicle.controls.z = 0;
        }
      }
    }
    /*vehicle.controls.y += gameapi -> timeElapsed() * -1.f;
    if (vehicle.controls.y < -9.81){
      vehicle.controls.y = -9.81;
    }

    // shitty inertia
    if (vehicle.controls.x > 0.1f){
      vehicle.controls.x -= 15.f * gameapi -> timeElapsed();
      if (vehicle.controls.x < 0){
        vehicle.controls.x = 0;
      }
      if (vehicle.controls.x < 0){
        vehicle.controls.x = 0;
      }
    }
    if (vehicle.controls.x < -0.1f){
      vehicle.controls.x += 15.f * gameapi -> timeElapsed();
      if (vehicle.controls.x > 0){
        vehicle.controls.x = 0;
      }
      if (vehicle.controls.x > 0){
        vehicle.controls.x = 0;
      }
    }
    if (vehicle.controls.z > 0.1f){
      vehicle.controls.z -= 15.f * gameapi -> timeElapsed();
      if (vehicle.controls.z < 0){
        vehicle.controls.z = 0;
      }
      if (vehicle.controls.z < 0){
        vehicle.controls.z = 0;
      }
    }
    if (vehicle.controls.z < -0.1f){
      vehicle.controls.z += 15.f * gameapi -> timeElapsed();
      if (vehicle.controls.z > 0){
        vehicle.controls.z = 0;
      }
      if (vehicle.controls.z > 0){
        vehicle.controls.z = 0;
      }
    }*/
  }
}

bool isVehicle(Vehicles& vehicles, objid id){
  return vehicles.vehicles.find(id) != vehicles.vehicles.end();
}