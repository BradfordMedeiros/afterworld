#ifndef MOD_AFTERWORLD_MOVEMENT
#define MOD_AFTERWORLD_MOVEMENT

#include <iostream>
#include <vector>
#include "./movementcore.h"
#include "../weapons/weapon.h"
#include "../controls.h"

struct Movement {
  ControlParams controlParams;
};


struct MovementRequest {
  glm::vec3 position;
  float speed;
};

struct MovementEntity {
  objid playerId;
  MovementParams* moveParams;
  MovementState movementState;
  std::optional<ThirdPersonCameraInfo> managedCamera;

  // when set the entity navigates to this location
  std::optional<MovementRequest> targetLocation;

};

struct MovementEntityData {
  std::unordered_map<objid, MovementEntity> movementEntities;
};

Movement createMovement();
void onMovementKeyCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeEntity, int key, int action);
void onMovementMouseMoveCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeId, double xPos, double yPos);
void onMovementScrollCallback(Movement& movement, double amount);
void onMovementFrame(MovementEntityData& movementEntityData, Movement& movement, objid activeEntity);

void setActiveMovementEntity(Movement& movement, MovementEntityData& movementEntityData, objid id, std::optional<objid> managedCamera);
std::optional<objid> getNextEntity(MovementEntityData& movementEntityData, std::optional<objid> activeId);

void setEntityTargetLocation(MovementEntityData& movementEntityData, objid id, std::optional<MovementRequest> movementRequest);
void raycastFromCameraAndMoveTo(MovementEntityData& movementEntityData, objid entityId);

void maybeAddMovementEntity(MovementEntityData& movementEntityData, objid id);
void maybeRemoveMovementEntity(MovementEntityData& movementEntityData, objid id);

void setZoomSensitivity(float multiplier);

#endif