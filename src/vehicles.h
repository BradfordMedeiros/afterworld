#ifndef MOD_AFTERWORLD_VEHICLES
#define MOD_AFTERWORLD_VEHICLES

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./controls.h"

struct Vehicle {
  bool occupied;

  glm::vec3 controls;

  glm::vec2 angleControls;

  ThirdPersonCameraInfo managedCamera;
  
};

struct Vehicles {
  std::unordered_map<objid, Vehicle> vehicles;
};

Vehicles createVehicles();
void addVehicle(Vehicles& vehicles, objid vehicleId);
void removeVehicle(Vehicles& vehicles, objid vehicleId);
void enterVehicle(Vehicles& vehicle, objid vehicleId, objid id);
void exitVehicle(Vehicles& vehicle, objid vehicleId, objid id);
bool isVehicle(Vehicles& vehicle, objid id);

void onVehicleKey(Vehicles& vehicle, int key, int action);
void onVehicleFrame(Vehicles& vehicles, ControlParams& controlParams);

#endif
