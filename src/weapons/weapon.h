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

struct PlayerInfo {
  objid playerId;
};
struct Weapons {
  std::optional<PlayerInfo> player;
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;
  bool fireOnce;
  float selectDistance;

  GunInstance weaponValues;

  glm::vec2 lookVelocity;
  std::optional<objid> heldItem;

  bool isGunZoomed;
  std::optional<objid> activateableItem;
};

Weapons createWeapons();

bool getIsGunZoomed(Weapons& weapons);
void maybeChangeGun(Weapons& weapons, std::string gun, std::string& inventory);
void changeWeaponTargetId(Weapons& weapons, objid id, std::string inventory);
void deliverAmmoToCurrentGun(Weapons& weapons, objid targetId, int amount, std::string& inventory);
AmmoInfo currentAmmoInfo(Weapons& weapons, std::string& inventory);

std::optional<AmmoInfo> onWeaponsFrame(Weapons& weapons, std::string& inventory);
void onWeaponsObjectRemoved(Weapons& weapons, objid idRemoved);
void onWeaponsMouseCallback(Weapons& weapons, int button, int action);
void onWeaponsKeyCallback(Weapons& weapons, int key, int action);
void onWeaponsMouseMove(Weapons& weapons, double xPos, double yPos);

#endif