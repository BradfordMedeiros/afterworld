#ifndef MOD_AFTERWORLD_VEHICLES
#define MOD_AFTERWORLD_VEHICLES

#include "./common.h"
#include "./ball.h"
#include "./ship.h"

typedef std::variant<VehicleShip, VehicleBall> VehicleType;

struct Vehicle {
  VehicleState state;
  VehicleType vehicle;
};

struct Vehicles {
  std::unordered_map<objid, Vehicle> vehicles;
};

Vehicles createVehicles();
void addVehicle(Vehicles& vehicles, objid vehicleId, bool isShip);
void removeVehicle(Vehicles& vehicles, objid vehicleId);
void enterVehicle(Vehicles& vehicle, objid vehicleId, objid id);

struct ExitVehicleInfo {
  glm::vec3 position;
  glm::quat rotation;
};
ExitVehicleInfo exitVehicle(Vehicles& vehicle, objid vehicleId, objid id);


bool isVehicle(Vehicles& vehicle, objid id);

std::vector<int> getVehicleIds();

void onVehicleKey(Vehicles& vehicle, int key, int action);
void onVehicleMouseClick(Vehicles& vehicle, int button, int action, int mods);

void onVehicleFrame(Vehicles& vehicles, ControlParams& controlParams);

/// vehicle type specific stuff
std::optional<VehicleBall*> getVehicleBall(Vehicles& vehicles, objid vehicleId);

#endif
