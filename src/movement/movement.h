#ifndef MOD_AFTERWORLD_MOVEMENT
#define MOD_AFTERWORLD_MOVEMENT

#include <iostream>
#include <vector>
#include "./movementcore.h"
#include "../weapons/weapon.h"
#include "../controls.h"

struct MovementRequest {
  glm::vec3 position;
  float speed;
};

struct MovementEntity {
  objid playerId;
  MovementParams* moveParams;
  MovementState movementState;

  // when set the entity navigates to this location
  std::optional<MovementRequest> targetLocation;
};

struct ActiveEntity {
  int index;
  std::optional<ThirdPersonCameraInfo> managedCamera;
};

void setActiveEntity(objid id, std::optional<objid> managedCamera);
std::optional<objid> getNextEntity();


void setEntityTargetLocation(objid id, std::optional<MovementRequest> movementRequest);
void raycastFromCameraAndMoveTo(objid entityId);

CScriptBinding movementBinding(CustomApiBindings& api, const char* name);

#endif