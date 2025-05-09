#ifndef MOD_AFTERWORLD_INVENTORY
#define MOD_AFTERWORLD_INVENTORY

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

struct Inventory {
  bool infinite;
  std::unordered_map<std::string, float> items;
};

objid getUnlimitedInventory();
objid getDefaultInventory();

objid inventoryById(objid id);

void addInventory(std::unordered_map<objid, Inventory>& scopenameToInventory, objid id);
void removeInventory(std::unordered_map<objid, Inventory>& scopenameToInventory, objid id);

int currentItemCount(std::unordered_map<objid, Inventory>& scopenameToInventory, objid inventory, std::string name);
void updateItemCount(std::unordered_map<objid, Inventory>& scopenameToInventory, objid inventory, std::string name, int count);

bool hasGun(objid inventory, std::string& gun);
int ammoForGun(objid inventory, std::string& gun);
void setGunAmmo(objid inventory, std::string gun, int currentAmmo);

std::set<std::string>& listGems();
void pickupGem(std::string name);

void debugPrintInventory(std::unordered_map<objid, Inventory>& scopenameToInventory);

#endif