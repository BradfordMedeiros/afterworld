#ifndef MOD_AFTERWORLD_SCENE_CORE
#define MOD_AFTERWORLD_SCENE_CORE

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "./vehicles/vehicles.h"

std::vector<int> getVehicleIds();
float querySelectDistance();

void setCanExitVehicle(bool canExit);
bool canExitVehicle();

#endif