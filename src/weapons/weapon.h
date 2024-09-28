#ifndef MOD_AFTERWORLD_WEAPON
#define MOD_AFTERWORLD_WEAPON

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../materials.h"
#include "./weaponcore.h"
#include "../resources.h"
#include "../controls.h"

struct WeaponEntityState {
  bool isHoldingFire;
  bool fireOnce;
  std::optional<objid> heldItem;
  bool isGunZoomed;
  GunInstance weaponValues;
};

struct Weapons {
  std::unordered_map<objid, WeaponEntityState> idToWeapon;
};

Weapons createWeapons();
void addWeaponId(Weapons& weapons, objid id);
void removeWeaponId(Weapons& weapons, objid id);

WeaponEntityState& getWeaponState(Weapons& weapons, objid id);

void maybeChangeGun(WeaponEntityState& weaponState, std::string gun, std::string& inventory, objid playerId);
void deliverAmmoToCurrentGun(WeaponEntityState& weaponState, int amount, std::string& inventory);

struct WeaponsUiUpdate {
  std::optional<AmmoInfo> ammoInfo;
  bool showActivateUi;
};

WeaponsUiUpdate onWeaponsFrame(WeaponEntityState& weaponState, std::string& inventory, objid playerId, glm::vec2 lookVelocity, glm::vec3 playerVelocity);

struct WeaponsMouseUpdate {
  std::optional<float> zoomAmount;
  std::optional<objid> selectItem;
};
WeaponsMouseUpdate onWeaponsMouseCallback(WeaponEntityState& weaponsState, int button, int action, objid playerId, float selectDistance);
void onWeaponsKeyCallback(WeaponEntityState& weaponsState, int key, int action, objid playerId);

#endif