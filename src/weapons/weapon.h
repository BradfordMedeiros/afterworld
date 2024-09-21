#ifndef MOD_AFTERWORLD_WEAPON
#define MOD_AFTERWORLD_WEAPON

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../global.h"
#include "../materials.h"
#include "./weaponcore.h"
#include "../resources.h"
#include "../controls.h"

struct WeaponControls {
  bool isHoldingLeftMouse;  // control param
  bool isHoldingRightMouse; // control param
  bool fireOnce;            // control param
};

struct WeaponEntityState {

};

struct Weapons {
  WeaponControls controls;

  GunInstance weaponValues; // state

  std::optional<objid> heldItem;  // state

  bool isGunZoomed; // state 
  std::optional<objid> activateableItem; //state
};

Weapons createWeapons();

bool getIsGunZoomed(Weapons& weapons);
void maybeChangeGun(Weapons& weapons, std::string gun, std::string& inventory, objid playerId);
void deliverAmmoToCurrentGun(Weapons& weapons, objid targetId, int amount, std::string& inventory, objid playerId);
AmmoInfo currentAmmoInfo(Weapons& weapons, std::string& inventory);

struct WeaponsUiUpdate {
  std::optional<AmmoInfo> ammoInfo;
  bool showActivateUi;
};

WeaponsUiUpdate onWeaponsFrame(Weapons& weapons, std::string& inventory, objid playerId, glm::vec2 lookVelocity);
void removeActiveGun(Weapons& weapons);
void onWeaponsMouseCallback(Weapons& weapons, int button, int action, objid playerId, float selectDistance);
void onWeaponsKeyCallback(Weapons& weapons, int key, int action, objid playerId);

#endif