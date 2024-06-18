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

struct Weapons {
  std::optional<objid> playerId;
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;
  bool fireOnce;
  float selectDistance;

  GunInstance weaponValues;

  glm::vec2 lookVelocity;
  std::optional<objid> heldItem;
};

Weapons createWeapons();
Weapons& getWeaponsPtr();

bool getIsGunZoomed();
void maybeChangeGun(Weapons& weapons, std::string gun);
void changeWeaponTargetId(objid id);
void deliverAmmoToCurrentGun(Weapons& weapons, objid targetId, int amount);

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name);


#endif