#include "./ship.h"

extern CustomApiBindings* gameapi;

void onVehicleFrameShip(objid id, VehicleState& state, VehicleShip& vehicleShip, ControlParams& controlParams){
  if (state.occupied.has_value()){
    // Would be nice to make this work more similarly to movement system but this is simple so

    auto thirdPerson = lookThirdPersonCalc(state.managedCamera, id, false);

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

    auto newVelocity = (diffRot * state.controls) + (thirdPerson.rotation * newVelocityDiff);
    state.controls = newVelocity;
  }

  {
    auto oldPos = gameapi -> getGameObjectPos(id, true, "vehicle getPos");
    if (state.occupied.has_value()){
      setGameObjectVelocity(id, state.controls);
    }
    
    bool slowDown = controlParams.shiftModifier;
    float ratePerSecond = 100.f;
    float amountThisFrame = ratePerSecond * gameapi -> timeElapsed();
    if (slowDown){
      if (state.controls.x > 0){
        state.controls.x -= amountThisFrame;
        if (state.controls.x < 0){
          state.controls.x = 0;
        }
      }
      if (state.controls.x < 0){
        state.controls.x += amountThisFrame;
        if (state.controls.x > 0){
          state.controls.x = 0;
        }
      }
      if (state.controls.y > 0){
        state.controls.y -= amountThisFrame;
        if (state.controls.y < 0){
          state.controls.y = 0;
        }
      }
      if (state.controls.y < 0){
        state.controls.y += amountThisFrame;
        if (state.controls.y > 0){
          state.controls.y = 0;
        }
      }
      if (state.controls.z > 0){
        state.controls.z -= amountThisFrame;
        if (state.controls.z < 0){
          state.controls.z = 0;
        }
      }
      if (state.controls.z < 0){
        state.controls.z += amountThisFrame;
        if (state.controls.z > 0){
          state.controls.z = 0;
        }
      }
    }
  }
}