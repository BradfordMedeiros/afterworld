#ifndef MOD_AFTERWORLD_WEAPON
#define MOD_AFTERWORLD_WEAPON

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../global.h"
#include "../materials.h"
#include "./weaponcore.h"
#include "./weapon_vector.h"
#include "../activeplayer.h"

struct PlayerInfo {
  objid playerId;
  std::string inventory;
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
};

Weapons createWeapons();

bool getIsGunZoomed();
void maybeChangeGun(Weapons& weapons, std::string gun);
void changeWeaponTargetId(Weapons& weapons, objid id, std::string inventory);
void deliverAmmoToCurrentGun(Weapons& weapons, objid targetId, int amount);
AmmoInfo currentAmmoInfo(Weapons& weapons);

std::optional<AmmoInfo> onWeaponsFrame(Weapons& weapons);
void onWeaponsObjectRemoved(Weapons& weapons, objid idRemoved);
void onWeaponsMouseCallback(Weapons& weapons, int button, int action);
void onWeaponsKeyCallback(Weapons& weapons, int key, int action);
void onWeaponsMouseMove(Weapons& weapons, double xPos, double yPos);
void onWeaponsMessage(Weapons& weapons, std::string& key);

#endif