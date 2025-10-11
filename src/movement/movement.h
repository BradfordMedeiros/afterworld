#ifndef MOD_AFTERWORLD_MOVEMENT
#define MOD_AFTERWORLD_MOVEMENT

#include <iostream>
#include <vector>
#include "./movementcore.h"
#include "../controls.h"

struct Movement {
  std::vector<ControlParams> controlParams;
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

  float zoomSensitivity = 1.f;

};

struct MovementEntityData {
  std::unordered_map<objid, MovementEntity> movementEntities;
};

Movement createMovement();
void addPlayerPortToMovement(Movement& movement, int port);
void removePlayerPortFromMovement(Movement& movement, int port);
ControlParams& getControlParamsByPort(Movement& movement, int playerIndex);

void onMovementKeyCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeEntity, int key, int action, int playerIndex);
void onMovementMouseMoveCallback(MovementEntityData& movementEntityData, Movement& movement, objid activeId, double xPos, double yPos, int playerPort);
void onMovementScrollCallback(Movement& movement, double amount, int playerPort);

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

struct MovementActivePlayer {
  objid activeId;
  int playerPort;
};
UiMovementUpdate onMovementFrame(MovementEntityData& movementEntityData, Movement& movement, std::function<bool(objid)> isGunZoomed, bool disableThirdPersonMesh, std::vector<EntityUpdate>& _entityUpdates, std::vector<MovementActivePlayer>& player);

void setActiveMovementEntity(Movement& movement, bool observeMode, int playerPort);
std::optional<objid> getNextEntity(MovementEntityData& movementEntityData, std::optional<objid> activeId);

void setEntityTargetLocation(MovementEntityData& movementEntityData, objid id, std::optional<MovementRequest> movementRequest);
void setEntityTargetRotation(MovementEntityData& movementEntityData, objid id, std::optional<glm::quat> rotation);
void raycastFromCameraAndMoveTo(MovementEntityData& movementEntityData, objid entityId, int viewportIndex);

void changeMovementEntityType(MovementEntityData& movementEntityData, objid id, std::string name);
bool maybeAddMovementEntity(MovementEntityData& movementEntityData, objid id);
void maybeRemoveMovementEntity(Movement& movement, MovementEntityData& movementEntityData, objid id);

void setZoomSensitivity(MovementEntityData& movementEntityData, float multiplier, objid id);

void setMovementEntityRotation(MovementEntityData& movementEntityData, objid id, glm::quat rotation);

#endif