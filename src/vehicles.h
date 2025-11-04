#ifndef MOD_AFTERWORLD_VEHICLES
#define MOD_AFTERWORLD_VEHICLES

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./controls.h"
#include "./movement/movementcore.h"

struct VehicleShip {};

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
};

typedef std::variant<VehicleShip, VehicleBall> VehicleType;

struct Vehicle {
  std::optional<objid> occupied;

  glm::vec3 controls;
  ThirdPersonCameraInfo managedCamera;

  std::optional<objid> sound;

  VehicleType vehicle;
};

struct Vehicles {
  std::unordered_map<objid, Vehicle> vehicles;
};

Vehicles createVehicles();
void addVehicle(Vehicles& vehicles, objid vehicleId, bool isShip);
void removeVehicle(Vehicles& vehicles, objid vehicleId);
void enterVehicle(Vehicles& vehicle, objid vehicleId, objid id);
void exitVehicle(Vehicles& vehicle, objid vehicleId, objid id);
bool isVehicle(Vehicles& vehicle, objid id);

void onVehicleKey(Vehicles& vehicle, int key, int action);
void onVehicleFrame(Vehicles& vehicles, ControlParams& controlParams);

#endif
