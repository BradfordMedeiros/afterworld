#ifndef MOD_AFTERWORLD_INVENTORY
#define MOD_AFTERWORLD_INVENTORY

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

struct GunInfo {
  float ammo;
};
std::optional<GunInfo> getGunInventoryInfo(std::string gun);
void setGunAmmo(std::string gun, int currentAmmo);

int currentItemCount(std::string name);
void updateItemCount(std::string name, int count);

#endif