#ifndef MOD_AFTERWORLD_INVENTORY
#define MOD_AFTERWORLD_INVENTORY

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

// Inventory should be scoped toward different entities
int currentItemCount(std::string name);
void updateItemCount(std::string name, int count);

bool hasGun(std::string& gun);
int ammoForGun(std::string inventory, std::string& gun);
void setGunAmmo(std::string inventory, std::string gun, int currentAmmo);

#endif