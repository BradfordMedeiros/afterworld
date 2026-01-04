#include "./vehicles.h"

extern CustomApiBindings* gameapi;

std::optional<objid> findChildObjBySuffix(objid id, const char* objName);
void disableEntity(objid id);
void reenableEntity(objid id, std::optional<glm::vec3> pos, std::optional<glm::quat> rot);

Vehicles createVehicles(){
  return Vehicles{};
}

bool vehicleOccupied(Vehicles& vehicles, objid vehicleId){
  return vehicles.vehicles.at(vehicleId).occupied.has_value();
}

void addVehicle(Vehicles& vehicles, objid vehicleId, bool isShip){
  auto soundId = findChildObjBySuffix(vehicleId, "sound");

  vehicles.vehicles[vehicleId] = Vehicle{
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
  };
  if (isShip){
    vehicles.vehicles.at(vehicleId).vehicle = VehicleShip{};
  }else{
    BallConfig ballConfig {
      .magnitude = 100.f,
      .torque = 50.f,
      .jumpMagnitude = 100.f,
      .mass = 10.f,
      .friction = 1.f,
      .restitution = 0.5f,
      .gravity = -9.81f,
    };
    setGameObjectPhysics(vehicleId, ballConfig.mass, ballConfig.restitution, ballConfig.friction, glm::vec3(0.f, ballConfig.gravity, 0.f));
    vehicles.vehicles.at(vehicleId).vehicle = VehicleBall{
      .ballConfig = ballConfig,
      .isGrounded = false,
      .shouldJump = false,
      .shouldUsePowerUp = false,
      .teleportPosition = std::nullopt,
      .powerup = std::nullopt,
    };
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
  bool occupied = vehicle.occupied.has_value();
  modassert(!occupied, "vehicle already occupied");
  vehicles.vehicles.at(vehicleId).occupied = id;

  modlog("vehicle enter vehicle", std::to_string(vehicleId));

  if (vehicle.sound.has_value()){
    playGameplayClipById(vehicle.sound.value(), std::nullopt, std::nullopt); 
  }
  disableEntity(vehicle.occupied.value());
}

void exitVehicle(Vehicles& vehicles, objid vehicleId, objid id){
  //vehicles.vehicles.at(vehicleId).occupied = false;
  modlog("vehicle exit vehicle", std::to_string(vehicleId));
  Vehicle& vehicle = vehicles.vehicles.at(vehicleId);

  auto position = gameapi -> getGameObjectPos(vehicleId, true, "[gamelogic] exit vehicle get pos") + glm::vec3(0.f, 30.f, 0.f);
  auto vehicleRot = gameapi -> getGameObjectRotation(vehicleId, true, "[gamelogic] vehicle exit get rot"); 

  reenableEntity(vehicle.occupied.value(), position, vehicleRot);

  vehicle.occupied = std::nullopt;
  if (vehicle.sound.has_value()){
    gameapi -> stopClipById(vehicle.sound.value());
  }
}

void onVehicleKey(Vehicles& vehicles, int key, int action){
  for (auto &[id, vehicle] :  vehicles.vehicles){
    if (vehicle.occupied.has_value()){
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

void onVehicleFrameShip(objid id, Vehicle& vehicle, ControlParams& controlParams){
  if (vehicle.occupied.has_value()){
    // Would be nice to make this work more similarly to movement system but this is simple so

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

  {
    auto oldPos = gameapi -> getGameObjectPos(id, true, "vehicle getPos");
    if (vehicle.occupied.has_value()){
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


bool checkIfGrounded(objid id){
  auto playerDirection = gameapi -> getGameObjectRotation(id, true, "vehicle - ball - check grounded");
  auto directionVec = playerDirection * glm::vec3(0.f, 0.f, -1.f); 
  directionVec.y = 0.f;
  auto rotationWithoutY = quatFromDirection(directionVec);

   
  std::vector<glm::quat> hitDirections;
  auto collisions = checkMovementCollisions(id, hitDirections, rotationWithoutY);
  bool isGrounded = collisions.movementCollisions.at(COLLISION_SPACE_DOWN);
  return isGrounded;
}

void onVehicleFrameBall(objid id, Vehicle& vehicle, ControlParams& controlParams){
  if (!vehicle.occupied.has_value()){
    return;
  }

  VehicleBall* vehicleBall = std::get_if<VehicleBall>(&vehicle.vehicle);
  modassert(vehicleBall, "onVehicleFrame incorrect type");
  if (vehicleBall){
    auto isGrounded = checkIfGrounded(id);
    std::cout << "is grounded: " << isGrounded << std::endl;
    vehicleBall -> isGrounded = isGrounded;
  }

  if (vehicleBall -> shouldJump && vehicleBall -> isGrounded){
    gameapi -> applyImpulse(id, glm::vec3(0.f, vehicleBall -> ballConfig.jumpMagnitude, 0.f));
  }
  vehicleBall -> shouldJump = false;

  auto rotation = lookThirdPersonCalc(vehicle.managedCamera, id).yAxisRotation;

  if (vehicleBall -> shouldUsePowerUp){
    if (vehicleBall -> teleportPosition.has_value()){
      // TODO This needs to preserve the direction too
      gameapi -> setGameObjectPosition(id, vehicleBall -> teleportPosition.value(), true, Hint { .hint = "vehicle teleport position" });
      vehicleBall -> teleportPosition = std::nullopt;
    }
    
    if (vehicleBall -> powerup.has_value()){
      if (vehicleBall -> powerup.value() == BIG_JUMP){
        gameapi -> applyImpulse(id, glm::vec3(0.f, 5 * vehicleBall -> ballConfig.jumpMagnitude, 0.f));
      }else if (vehicleBall -> powerup.value() == LAUNCH_FORWARD){
        auto direction = rotation * glm::vec3(0.f, vehicleBall -> ballConfig.jumpMagnitude, -1 * vehicleBall -> ballConfig.jumpMagnitude);
        gameapi -> applyImpulse(id, direction);
      }else if (vehicleBall -> powerup.value() == LOW_GRAVITY){
        setGameObjectGravity(id, glm::vec3(0.f, 0.2f * vehicleBall -> ballConfig.gravity, 0.f));
      }else if (vehicleBall -> powerup.value() == REVERSE_GRAVITY){
        // This needs changes in the camera to feel correct
        setGameObjectGravity(id, glm::vec3(0.f, -1.f * vehicleBall -> ballConfig.gravity, 0.f));
      } else if (vehicleBall -> powerup.value() == TELEPORT){
        vehicleBall -> teleportPosition =  gameapi -> getGameObjectPos(id, true, "[gamelogic] get ball position for teleport");
      }
      vehicleBall -> powerup = std::nullopt;
    }
  }
  vehicleBall -> shouldUsePowerUp = false;


  std::cout << "onVehicleFrameBall onFrame" << std::endl;

  auto magnitude = vehicleBall -> ballConfig.magnitude * gameapi -> timeElapsed();
  auto torqueMagnitude = vehicleBall -> ballConfig.torque;

  glm::vec3 direction(0.f, 0.f, 0.f);

  if (controlParams.goForward){
    direction.z = -1.f;
  }
  if (controlParams.goBackward){
    direction.z = 1.f;
  }
  if (controlParams.goLeft){
    direction.x = -1.f;
  }
  if (controlParams.goRight){
    direction.x = 1.f;
  }

  // should normalize the direction here
  auto amount = rotation * glm::vec3(magnitude * direction.x, magnitude * direction.y, magnitude * direction.z);
  gameapi -> applyImpulse(id, amount);

  auto torqueAmount = rotation * glm::vec3(torqueMagnitude * direction.z, torqueMagnitude * direction.y, -1 * torqueMagnitude * direction.x);
  gameapi -> applyTorque(id, torqueAmount);

  //std::cout << "onVehicleFrameBall: " << id << " , apply force = " << print(amount) <<  ", apply torque = " << print(torqueAmount) <<  ", name = " << gameapi -> getGameObjNameForId(id).value() << ", occupied = " << gameapi -> getGameObjNameForId(vehicle.occupied.value()).value() << std::endl;
}

void onVehicleFrame(Vehicles& vehicles, ControlParams& controlParams){
  for (auto &[id, vehicle] :  vehicles.vehicles){
    if (vehicle.occupied.has_value()){
      vehicle.managedCamera.angleX += gameapi -> timeElapsed() * controlParams.lookVelocity.x;
      vehicle.managedCamera.angleY += gameapi -> timeElapsed() * controlParams.lookVelocity.y;
      vehicle.managedCamera.actualDistanceFromTarget += gameapi -> timeElapsed() * controlParams.zoom_delta * 20.f /*arbitary multiplier */;
    }

    {
      VehicleShip* vehicleShip = std::get_if<VehicleShip>(&vehicle.vehicle);
      if (vehicleShip){
        onVehicleFrameShip(id, vehicle, controlParams);
      }
    }
    {
      VehicleBall* vehicleBall = std::get_if<VehicleBall>(&vehicle.vehicle);
      if (vehicleBall){
        onVehicleFrameBall(id, vehicle, controlParams);
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
