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
  int playerId;
  std::optional<ThirdPersonCameraInfo> managedCamera;
};

struct MovementEntityData {
  std::unordered_map<objid, MovementEntity> movementEntities;
  std::optional<ActiveEntity> activeEntity;
};

MovementEntityData& getMovementData();
void setActiveEntity(MovementEntityData& movementEntityData, objid id, std::optional<objid> managedCamera);
std::optional<objid> getNextEntity(MovementEntityData& movementEntityData);

void setEntityTargetLocation(MovementEntityData& movementEntityData, objid id, std::optional<MovementRequest> movementRequest);
void raycastFromCameraAndMoveTo(MovementEntityData& movementEntityData, objid entityId);

void maybeAddMovementEntity(MovementEntityData& movementEntityData, objid id);
void maybeRemoveMovementEntity(MovementEntityData& movementEntityData, objid id);

CScriptBinding movementBinding(CustomApiBindings& api, const char* name);

#endif