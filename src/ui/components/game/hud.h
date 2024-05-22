#ifndef MOD_AFTERWORLD_COMPONENTS_HUD
#define MOD_AFTERWORLD_COMPONENTS_HUD

#include "../common.h"
#include "../../../global.h"

struct AmmoHudInfo {
  int currentAmmo;
  int totalAmmo;
};
void setUIAmmoCount(AmmoHudInfo ammoInfo);
void setHealth(float health);

extern Component hudComponent;

#endif

