#ifndef MOD_AFTERWORLD_INVENTORY
#define MOD_AFTERWORLD_INVENTORY

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

void inventoryOnCollision(int32_t obj1, int32_t obj2);
void setGunAmmo(std::string gun, int currentAmmo);
void requestChangeGun(std::string gun);

#endif