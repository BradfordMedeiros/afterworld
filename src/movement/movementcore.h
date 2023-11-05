#ifndef MOD_AFTERWORLD_MOVEMENT_CORE
#define MOD_AFTERWORLD_MOVEMENT_CORE 

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../global.h"
#include "../resources.h"
#include "../activeplayer.h"

struct ControlParams {
  float xsensitivity;
  float ysensitivity;
};

struct MovementParams {
  float moveSpeed;
  float moveSpeedAir;
  float moveSpeedWater;
  float jumpHeight;
  float maxAngleUp;
  float maxAngleDown;
  float moveSoundDistance;
  float moveSoundMintime;
  float groundAngle;
  glm::vec3 gravity;
  bool canCrouch;
  float crouchSpeed;
  float crouchScale;
  float crouchDelay;
  float friction;
  float crouchFriction;
};
struct MovementState {
  float lastMoveSoundPlayTime;      // state
  glm::vec3 lastMoveSoundPlayLocation;
  float xRot;               
  float yRot;
  bool facingWall;
  bool facingLadder;
  bool attachedToLadder;    
  bool inWater;  
  bool isGrounded;              
  bool lastFrameIsGrounded;
  bool isCrouching;
  bool shouldBeCrouching;
  glm::vec3 lastPosition;
  float lastCrouchTime;
};

void jump(MovementParams& moveParams, MovementState& movementState, objid id);
void land(objid id);

void moveUp(objid id, glm::vec2 direction);
void moveDown(objid id, glm::vec2 direction);
void moveXZ(objid id, glm::vec2 direction);

float getMoveSpeed(MovementParams& moveParams, MovementState& movementState, bool ironsight, bool isGrounded);

void updateVelocity(MovementState& movementState, objid id, float elapsedTime, glm::vec3 currPos, bool* _movingDown);
void updateFacingWall(MovementState& movementState, objid id);

void restrictLadderMovement(MovementState& movementState, objid id, bool movingDown);

void look(MovementParams& moveParams, MovementState& movementState, objid id, float elapsedTime, bool ironsight, float ironsight_turn, glm::vec2 lookVelocity, ControlParams& controlParams);

void attachToLadder(MovementState& movementState);
void releaseFromLadder(MovementState& movementState);

void toggleCrouch(MovementParams& moveParams, MovementState& movementState, objid id, bool shouldCrouch);
void updateCrouch(MovementParams& moveParams, MovementState& movementState, objid id);


bool shouldStepUp(objid id);


enum COLLISION_SPACE_INDEX { COLLISION_SPACE_LEFT = 0, COLLISION_SPACE_RIGHT = 1, COLLISION_SPACE_DOWN = 3 };
std::vector<bool> getCollisionSpaces(std::vector<HitObject>& hitpoints, glm::quat rotationWithoutY);

struct MovementCollisions {
  std::vector<bool> movementCollisions;
  std::vector<objid> allCollisions;
};
MovementCollisions checkMovementCollisions(objid playerId, std::vector<glm::quat>& hitDirections, glm::quat rotationWithoutY);

glm::vec3 limitMoveDirectionFromCollisions(glm::vec3 moveVec, std::vector<glm::quat>& hitDirections, glm::quat playerDirection);


#endif