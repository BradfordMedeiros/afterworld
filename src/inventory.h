#ifndef MOD_AFTERWORLD_INVENTORY
#define MOD_AFTERWORLD_INVENTORY

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

objid getUnlimitedInventory();
objid getDefaultInventory();

objid inventoryById(objid id);

void addInventory(objid id);
void removeInventory(objid id);

int currentItemCount(objid inventory, std::string name);
void updateItemCount(objid inventory, std::string name, int count);

bool hasGun(objid inventory, std::string& gun);
int ammoForGun(objid inventory, std::string& gun);
void setGunAmmo(objid inventory, std::string gun, int currentAmmo);


void debugPrintInventory();

#endif