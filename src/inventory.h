#ifndef MOD_AFTERWORLD_INVENTORY
#define MOD_AFTERWORLD_INVENTORY

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

std::string& getUnlimitedInventory();

std::string& inventoryById(objid id);

int currentItemCount(std::string inventory, std::string name);
void updateItemCount(std::string inventory, std::string name, int count);

bool hasGun(std::string inventory, std::string& gun);
int ammoForGun(std::string inventory, std::string& gun);
void setGunAmmo(std::string inventory, std::string gun, int currentAmmo);

void debugPrintInventory();

#endif