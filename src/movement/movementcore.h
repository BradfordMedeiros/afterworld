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

  bool goForward;                
  bool goBackward;
  bool goLeft;
  bool goRight;

  glm::vec2 lookVelocity;

  bool doJump;   
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
  float physicsMass;
  float physicsRestitution;

  std::string jumpSound;
  std::string landSound;
  std::string moveSound;
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

MovementParams* findMovementCore(std::string& name);
void loadMovementCore(std::string& coreName);
void removeAllMovementCores();

MovementParams getMovementParams(std::string name);

void jump(MovementParams& moveParams, MovementState& movementState, objid id);
void attachToLadder(MovementState& movementState);
void releaseFromLadder(MovementState& movementState);
void maybeToggleCrouch(MovementParams& moveParams, MovementState& movementState, bool crouchDown);
void onMovementFrame(MovementParams& moveParams, MovementState& movementState, objid playerId, ControlParams& controlParams);

MovementState getInitialMovementState(objid playerId);

std::string movementToStr(ControlParams& controlParams);

#endif