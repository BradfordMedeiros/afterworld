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
  auto traitQuery = gameapi -> compileSqlQuery("select select-distance from traits where profile = ?", { "default" });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(traitQuery, &validSql);
  modassert(validSql, "error executing sql query");
  float selectDistance = floatFromFirstSqlResult(result, 0);
  return selectDistance;
}

