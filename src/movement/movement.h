#ifndef MOD_AFTERWORLD_MOVEMENT
#define MOD_AFTERWORLD_MOVEMENT

#include <iostream>
#include <vector>
#include "./movementcore.h"
#include "../controls.h"

struct Movement {
  ControlParams controlParams;
  std::set<objid> disabledMeshes;
};


struct MovementRequest {
  glm::vec3 position;
  float speed;
};

struct MovementEntity {
  objid playerId;
  MovementParams* moveParams;
  MovementState movementState;
  ThirdPersonCameraInfo managedCamera;

  // when set the entity navigates to this location
  std::optional<MovementRequest> targetLocation;
  std::optional<glm::quat> targetRotation;

};

struct MovementEntityData {
  std::unordered_map<objid, MovementEntity> movementEntities;
};

Movement createMovement();
void reloadSettingsConfig(Movement& movement, std::string name);

void onMovementKeyCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeEntity, int key, int action);
void onMovementMouseMoveCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeId, double xPos, double yPos);
void onMovementScrollCallback(Movement& movement, double amount);

glm::quat getLookDirection(MovementEntity& movementEntity);

struct UiMovementUpdate {
  std::optional<glm::vec3> velocity;
  std::optional<glm::vec2> lookVelocity;
};

struct EntityUpdate {
  objid id;
  std::optional<glm::vec3> pos;
  std::optional<glm::quat> rot;
  const char* posHint = NULL;
  const char* rotHint = NULL;
};

UiMovementUpdate onMovementFrame(MovementEntityData& movementEntityData, Movement& movement, objid activeEntity, std::function<bool(objid)> isGunZoomed, objid thirdPersonCamera, bool disableThirdPersonMesh, std::vector<EntityUpdate>& entityUpdates);
void onMovementFrameLateUpdate(MovementEntityData& movementEntityData, Movement& movement, objid activeId);

void setActiveMovementEntity(Movement& movement);
std::optional<objid> getNextEntity(MovementEntityData& movementEntityData, std::optional<objid> activeId);

void setEntityTargetLocation(MovementEntityData& movementEntityData, objid id, std::optional<MovementRequest> movementRequest);
void setEntityTargetRotation(MovementEntityData& movementEntityData, objid id, std::optional<glm::quat> rotation);
void raycastFromCameraAndMoveTo(MovementEntityData& movementEntityData, objid entityId);

void changeMovementEntityType(MovementEntityData& movementEntityData, objid id, std::string name);
bool maybeAddMovementEntity(MovementEntityData& movementEntityData, objid id);
void maybeRemoveMovementEntity(Movement& movement, MovementEntityData& movementEntityData, objid id);

void setZoomSensitivity(float multiplier);

#endif