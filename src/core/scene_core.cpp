#include "./scene_core.h"

extern CustomApiBindings* gameapi;

extern Vehicles vehicles;

std::vector<int> getVehicleIds(){
  std::vector<int>  vehicleIds;
  for (auto& [id, vehicle] : vehicles.vehicles){
    vehicleIds.push_back(id);
  }
  return vehicleIds;
}

float querySelectDistance(){
  return 100.f;
}

bool canExitVehicleValue = true;
void setCanExitVehicle(bool canExit){
  canExitVehicleValue = canExit;
}
bool canExitVehicle(){
  return canExitVehicleValue;
}

bool isControlledVehicle(int vehicleId){
  if (vehicles.vehicles.find(vehicleId) == vehicles.vehicles.end()){
    return false;
  }
  return vehicles.vehicles.at(vehicleId).state.occupied.has_value();
}

