#ifndef MOD_AFTERWORLD_MOVEMENT_CORE
#define MOD_AFTERWORLD_MOVEMENT_CORE 

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../resources.h"

struct ThirdPersonCameraInfo {
  bool thirdPersonMode;
  float distanceFromTarget;
  float angleX;
  float angleY;
  float actualDistanceFromTarget;
  glm::vec3 additionalCameraOffset;
  glm::vec3 zoomOffset;
  glm::vec3 actualZoomOffset;
  bool reverseCamera;
};

enum CrouchType { CROUCH_NONE, CROUCH_UP, CROUCH_DOWN };
struct ControlParams {
  float xsensitivity;
  float ysensitivity;

  bool goForward;                
  bool goBackward;
  bool goLeft;
  bool goRight;
  bool shiftModifier;

  glm::vec2 lookVelocity;
  float zoom_delta;

  bool doJump;
  bool doAttachToLadder;
  bool doReleaseFromLadder;
  CrouchType crouchType;
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
  bool moveVertical;
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
  // control data  this should be reset each frame
  glm::vec3 moveVec;
  float speed;
  float zoom_delta;
  bool doJump;
  bool doAttachToLadder;
  bool doReleaseFromLadder;
  float raw_deltax;
  float raw_deltay;
  CrouchType crouchType;
  ///////////////////////////////////

  ////
  float lastMoveSoundPlayTime;
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
  glm::vec3 velocity;

  glm::vec3 initialScale;
};

MovementParams* findMovementCore(std::string& name);
void loadMovementCore(std::string& coreName);
void removeAllMovementCores();


glm::vec3 getMovementControlDataFromTargetPos(glm::vec3 targetPosition, MovementState& movementState, objid playerId, bool* atTargetPos);

struct ThirdPersonCameraUpdate {
  glm::vec3 position;
  glm::quat rotation;
};
struct FirstPersonCameraUpdate {
  glm::quat rotation;   // rotation here means orientation
  glm::quat yAxisRotation;
};

struct CameraUpdate {
  std::optional<ThirdPersonCameraUpdate> thirdPerson;
  std::optional<FirstPersonCameraUpdate> firstPerson;
};
CameraUpdate onMovementFrameCore(MovementParams& moveParams, MovementState& movementState, objid playerId, ThirdPersonCameraInfo& managedCamera, bool isGunZoomed);

MovementState getInitialMovementState(objid playerId);


#endif
