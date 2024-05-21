#ifndef MOD_AFTERWORLD_MOVEMENT
#define MOD_AFTERWORLD_MOVEMENT

#include <iostream>
#include <vector>
#include "./movementcore.h"
#include "../weapons/weapon.h"
#include "../controls.h"


void setActiveEntity(objid id, std::optional<objid> managedCamera);
std::optional<objid> getNextEntity();

struct MovementRequest {
  glm::vec3 position;
  float speed;
};
void setEntityTargetLocation(objid id, std::optional<MovementRequest> movementRequest);
void raycastFromCameraAndMoveTo(objid entityId);

CScriptBinding movementBinding(CustomApiBindings& api, const char* name);

#endif