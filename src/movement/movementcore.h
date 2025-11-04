#ifndef MOD_AFTERWORLD_MOVEMENT_CORE
#define MOD_AFTERWORLD_MOVEMENT_CORE 

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../resources/resources.h"
#include "../curves.h"


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
  bool doGrind;
  bool doReverseGrind;
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

  glm::vec3 newVelocity;
  bool changedYVelocity;

  bool alive;
  bool falling;


  std::optional<float> reloading;
  std::optional<float> reloadingLength;
};

MovementParams* findMovementCore(std::string& name);
void loadMovementCore(std::string& coreName);
void removeAllMovementCores();

glm::quat weaponLookDirection(MovementState& movementState);

glm::vec3 getMovementControlDataFromTargetPos(glm::vec3 targetPosition, MovementState& movementState, objid playerId, bool* atTargetPos, bool moveVertical);

bool isReloading(MovementState& movementState);

struct FirstPersonCameraUpdate {
  glm::quat rotation;   // rotation here means orientation
  glm::quat yAxisRotation;
};

struct CameraUpdate {
  std::optional<ThirdPersonCameraUpdate> thirdPerson;
  FirstPersonCameraUpdate firstPerson;
};
CameraUpdate onMovementFrameCore(MovementParams& moveParams, MovementState& movementState, objid playerId, ThirdPersonCameraInfo& managedCamera, bool isGunZoomed, bool enableThirdPerson);

MovementState getInitialMovementState(objid playerId);
glm::vec2 pitchXAndYawYRadians(glm::quat currRotation);

enum COLLISION_SPACE_INDEX { COLLISION_SPACE_LEFT = 0, COLLISION_SPACE_RIGHT = 1, COLLISION_SPACE_DOWN = 3 };
struct MovementCollisions {
  std::vector<bool> movementCollisions;
  std::vector<objid> allCollisions;
};
MovementCollisions checkMovementCollisions(objid playerId, std::vector<glm::quat>& _hitDirections, glm::quat rotationWithoutY);


#endif
