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
void setShowActivate(bool showActivate);
void showLetterBox(std::string title);

extern Component hudComponent;

#endif

