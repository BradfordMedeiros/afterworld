#include "./ball.h"

extern CustomApiBindings* gameapi;

std::optional<objid> findChildObjBySuffix(objid id, const char* objName);


/*
  TODO 
  - maybe move collision disallow to move into a wall (option, kind of like how it works atm)
  - experiment with downward along the ramp
  
  TODO - tweraks params
*/

glm::vec3 getSurfaceVelocityModifiers(objid id);

VehicleBall doCreateVehicleBall(objid vehicleId, VehicleState& state){
  BallConfig ballConfig {
    .magnitude = 100.f,
    .torque = 50.f,
    .jumpMagnitude = 100.f,
    .mass = 10.f,
    .friction = 1.f,
    .restitution = 0.5f,
    .gravity = -9.81f,
  };
  VehicleBall vehicleBall {
    .ballConfig = ballConfig,
    .isGrounded = false,
    .shouldJump = false,
    .shouldUsePowerUp = false,
    .teleportPosition = std::nullopt,
    .powerup = std::nullopt,
  };
  setGameObjectPhysics(vehicleId, ballConfig.mass, ballConfig.restitution, ballConfig.friction, glm::vec3(0.f, ballConfig.gravity, 0.f));

  return vehicleBall;
}

const float AERIAL_MAG_SCALE = 0.5f;
const float AERIAL_TORQUE_SCALE = 1.f;
const bool RELATIVE_JUMP = true;

std::optional<HitObject> checkIfGrounded(objid id){
  auto playerDirection = gameapi -> getGameObjectRotation(id, true, "vehicle - ball - check grounded");
  auto directionVec = playerDirection * glm::vec3(0.f, 0.f, -1.f); 
  directionVec.y = 0.f;
  auto rotationWithoutY = quatFromDirection(directionVec);

   
  std::vector<glm::quat> hitDirections;
  auto collisions = checkMovementCollisions(id, hitDirections, rotationWithoutY);

  auto downCollision = collisions.movementCollisions.at(COLLISION_SPACE_DOWN);
  return downCollision;
}

void printBallDebug(VehicleBall& vehicleBall){
  std::cout << "is grounded: " << vehicleBall.isGrounded << std::endl;

}

void onVehicleFrameBall(objid id, VehicleState& state, VehicleBall& vehicleBall, ControlParams& controlParams){
  if (!state.occupied.has_value()){
    return;
  }

  if (!vehicleBall.soundId.has_value()){
    auto ballSound = findChildObjBySuffix(id, "ballsoundmove");
    if (ballSound.has_value()){
      vehicleBall.soundId = ballSound.value();
      playGameplayClipById(vehicleBall.soundId.value(), std::nullopt, std::nullopt, false);
    }
  }

  auto velocity = getGameObjectVelocity(id);
  std::cout << "ball velocity: " << glm::length(velocity) << std::endl;
  float percentage = glm::length(velocity) / 10.f;
  if (percentage > 2.f){
    percentage = 2.f;
  }
  percentage *= 0.5f;

  if (!vehicleBall.isGrounded){
    percentage = 0.f;
  }

  //gameapi -> setSoundPitch(vehicleBall.soundId.value(), percentage);
  gameapi -> setSoundVolume(vehicleBall.soundId.value(), 5 * percentage);

 
  auto groundHit = checkIfGrounded(id);
  vehicleBall.isGrounded = groundHit.has_value();
  auto rotation = lookThirdPersonCalc(state.managedCamera, id).yAxisRotation;

  //////////// CORE MOVEMENT ////////////
  std::cout << "onVehicleFrameBall onFrame" << std::endl;
  {
    auto magnitude = vehicleBall.ballConfig.magnitude *    (vehicleBall.isGrounded ? 1.f : AERIAL_MAG_SCALE);
    auto torqueMagnitude = vehicleBall.ballConfig.torque * (vehicleBall.isGrounded ? 1.f : AERIAL_TORQUE_SCALE);

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
    if (glm::length(direction) > 0.001f){
      direction = glm::normalize(direction);
    }

    auto extraVelocityForce = getSurfaceVelocityModifiers(id);
    std::cout << "extra vel: " << print(extraVelocityForce) << std::endl;

    // should normalize the direction here
    auto amount = rotation * glm::vec3(magnitude * direction.x, magnitude * direction.y, magnitude * direction.z);

    gameapi -> applyForce(id, amount + extraVelocityForce);

    auto torqueAmount = rotation * glm::vec3(torqueMagnitude * direction.z, torqueMagnitude * direction.y, -1 * torqueMagnitude * direction.x);
    gameapi -> applyTorque(id, torqueAmount);

    if (vehicleBall.shouldJump && vehicleBall.isGrounded){
      if (RELATIVE_JUMP){
        auto jumpImpulse = glm::normalize(groundHit.value().normal) * glm::vec3(0.f, 0.f, -1.f * vehicleBall.ballConfig.jumpMagnitude); 
        gameapi -> applyImpulse(id, jumpImpulse);
      }else{
        auto jumpImpulse = glm::vec3(0.f, vehicleBall.ballConfig.jumpMagnitude, 0.f); 
        gameapi -> applyImpulse(id, jumpImpulse);
      }
      playGameplayClipCentered(getManagedSounds().balljumpObjId.value());
    }
    vehicleBall.shouldJump = false;

    /// debug visualization
    auto position  =  gameapi -> getGameObjectPos(id, true, "[gamelogic] get ball position for debug");
    gameapi -> drawLine(position, position + glm::vec3(0.f, 1.f, 0.f), false, id, std::nullopt, std::nullopt, std::nullopt);
    //gameapi -> drawLine(position, position + amount, false, id, std::nullopt, std::nullopt, std::nullopt);

    if (groundHit.has_value()){
      auto delta = 5.f * (groundHit.value().normal * glm::vec3(0.f, 0.f, -1.f));
      gameapi -> drawLine(groundHit.value().point, groundHit.value().point + delta, false, id, std::nullopt, std::nullopt, std::nullopt);
    }
  }

  ////////// POWERUP ////////////////////////////
  {
    if (vehicleBall.shouldUsePowerUp){
      if (vehicleBall.teleportPosition.has_value()){
        // TODO This needs to preserve the direction too
        gameapi -> setGameObjectPosition(id, vehicleBall.teleportPosition.value(), true, Hint { .hint = "vehicle teleport position" });
        vehicleBall.teleportPosition = std::nullopt;
      }
      if (vehicleBall.powerup.has_value()){
        if (vehicleBall.powerup.value() == BIG_JUMP){
          gameapi -> applyImpulse(id, glm::vec3(0.f, 2 * vehicleBall.ballConfig.jumpMagnitude, 0.f));
        }else if (vehicleBall.powerup.value() == LAUNCH_FORWARD){
          auto direction = rotation * glm::vec3(0.f, vehicleBall.ballConfig.jumpMagnitude, -1 * vehicleBall.ballConfig.jumpMagnitude);
          gameapi -> applyImpulse(id, direction);
        }else if (vehicleBall.powerup.value() == LOW_GRAVITY){
          setGameObjectGravity(id, glm::vec3(0.f, 0.2f * vehicleBall.ballConfig.gravity, 0.f));
        }else if (vehicleBall.powerup.value() == REVERSE_GRAVITY){
          // This needs changes in the camera to feel correct
          setGameObjectGravity(id, glm::vec3(0.f, -1.f * vehicleBall.ballConfig.gravity, 0.f));
        } else if (vehicleBall.powerup.value() == TELEPORT){
          vehicleBall.teleportPosition =  gameapi -> getGameObjectPos(id, true, "[gamelogic] get ball position for teleport");
        }
        vehicleBall.powerup = std::nullopt;
      }

      playGameplayClipById(getManagedSounds().powerupObjId.value(), std::nullopt, std::nullopt, false);
    }
    vehicleBall.shouldUsePowerUp = false;
  }

  //std::cout << "onVehicleFrameBall: " << id << " , apply force = " << print(amount) <<  ", apply torque = " << print(torqueAmount) <<  ", name = " << gameapi -> getGameObjNameForId(id).value() << ", occupied = " << gameapi -> getGameObjNameForId(vehicle.occupied.value()).value() << std::endl;
}