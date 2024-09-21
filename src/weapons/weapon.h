#ifndef MOD_AFTERWORLD_WEAPON
#define MOD_AFTERWORLD_WEAPON

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../materials.h"
#include "./weaponcore.h"
#include "../resources.h"
#include "../controls.h"

struct WeaponControls {
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;
  bool fireOnce;
};

struct WeaponEntityState {
  std::optional<objid> heldItem;
  bool isGunZoomed;
  GunInstance weaponValues;
};

struct Weapons {
  WeaponControls controls;
  WeaponEntityState state;
};

Weapons createWeapons();

void maybeChangeGun(Weapons& weapons, std::string gun, std::string& inventory, objid playerId);
void deliverAmmoToCurrentGun(Weapons& weapons, int amount, std::string& inventory);

struct WeaponsUiUpdate {
  std::optional<AmmoInfo> ammoInfo;
  bool showActivateUi;
};

WeaponsUiUpdate onWeaponsFrame(Weapons& weapons, std::string& inventory, objid playerId, glm::vec2 lookVelocity);
void removeActiveGun(Weapons& weapons);
void onWeaponsMouseCallback(Weapons& weapons, int button, int action, objid playerId, float selectDistance);
void onWeaponsKeyCallback(Weapons& weapons, int key, int action, objid playerId);

#endif