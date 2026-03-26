#ifndef MOD_AFTERWORLD_VEHICLES_BALL
#define MOD_AFTERWORLD_VEHICLES_BALL

#include "./common.h"

struct BallConfig {
  float magnitude;
  float torque;
  float jumpMagnitude;
  float mass;
  float friction;
  float restitution;
  float gravity;
};

struct VehicleBall {
  BallConfig ballConfig;

  // state
  bool isGrounded;
  bool shouldJump;
  bool shouldUsePowerUp;

  std::optional<glm::vec3> teleportPosition;

  std::optional<BallPowerup> powerup;

  std::optional<objid> soundId;
  std::optional<OneShot> oneshotSoundSource;

  float lastGravityTime = 0.f;
  bool inGravityWell = false;

  float lastPlayTime = 0.f;


};

VehicleBall doCreateVehicleBall(objid vehicleId, VehicleState& state);
void onVehicleFrameBall(objid id, VehicleState& state, VehicleBall& vehicleBall, ControlParams& controlParams);
void setBallGravityWell(objid id, VehicleBall& vehicleBall, bool enter);

#endif
