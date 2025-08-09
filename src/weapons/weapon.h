#ifndef MOD_AFTERWORLD_WEAPON
#define MOD_AFTERWORLD_WEAPON

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../resources/materials.h"
#include "./weaponcore.h"
#include "../resources/resources.h"
#include "../controls.h"

struct MessageWithId {
  objid id;
  std::optional<std::string> value;
};

enum HoldToggle { HOLD_TOGGLE_NONE, HOLD_TOGGLE_PICKUP, HOLD_TOGGLE_RELEASE };
struct WeaponEntityState {
  bool isHoldingFire;
  bool fireOnce;
  bool activate;
  std::optional<objid> heldItem;
  HoldToggle holdToggle;
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

void maybeChangeGun(WeaponEntityState& weaponState, std::string gun, objid inventory);
void deliverAmmoToCurrentGun(WeaponEntityState& weaponState, int amount, objid inventory);

struct WeaponsUiUpdate {
  std::optional<AmmoInfo> ammoInfo;
  bool showActivateUi;
  std::optional<float> bloomAmount;
  std::optional<std::string*> currentGunName;
  bool didFire;
  std::optional<objid> raycastId;
};

struct WeaponEntityData {
  objid inventory;
  glm::vec2 lookVelocity;
  glm::vec3 velocity;
  bool thirdPersonMode;
  FiringTransform fireTransform;
};
WeaponsUiUpdate onWeaponsFrame(Weapons& weapons, objid playerId, glm::vec2 lookVelocity, glm::vec3 playerVelocity, std::function<WeaponEntityData(objid)> getWeaponEntityData, std::function<objid(objid)> getWeaponParentId, ThirdPersonWeapon thirdPersonWeapon, bool disableShowGun, bool isPlayerAlive);

struct WeaponsMouseUpdate {
  std::optional<float> zoomAmount;
  std::optional<bool> zoomUpdate;
};
WeaponsMouseUpdate onWeaponsMouseCallback(WeaponEntityState& weaponsState, int button, int action, objid playerId, float selectDistance);
void onWeaponsKeyCallback(WeaponEntityState& weaponsState, int key, int action, objid playerId);

void fireGun(Weapons& weapons, objid playerId);

void setShowWeaponModel(bool showModel);

#endif