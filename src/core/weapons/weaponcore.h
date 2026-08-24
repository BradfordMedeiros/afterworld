#pragma once 

#include <string>
#include <vector>
#include "./weapondata.h"
#include "../../../../ModEngine/src/cscript/cscript_binding.h"
#include "../../util.h"
#include "../../resources/materials.h"
#include "../../resources/layer.h"
#include "../../resources/sound.h"


WeaponParams queryWeaponParams(std::string gunName);

enum GunAnimation { GUN_RAISED, GUN_LOWERING };
struct WeaponState {
  float lastShootingTime;
  float recoilStart;
  GunAnimation gunState;
};

struct WeaponCore {
  std::string name;
  WeaponParams weaponParams;
  std::optional<objid> muzzleParticle;
  std::optional<objid> hitParticles;
  std::optional<objid> projectileParticles;
  bool removeProjectileOnExit;
};

struct GunCore {
  WeaponCore* weaponCore = NULL;
  WeaponState weaponState;
};

struct GunInstance {
  std::string desiredGun;
  float changeGunTime = 0.f;

  GunCore gunCore;
  std::optional<objid> gunId;
  std::optional<objid> muzzleId;

  std::optional<objid> thirdPersonGunId;
};

GunCore createGunCoreInstance(std::string gun, objid sceneId);
std::optional<std::string*> getCurrentGunName(GunInstance& weaponValues);

struct ThirdPersonWeapon {
  std::function<std::optional<objid>(objid)> getWeaponParentId;
};
void ensureGunInstance(GunInstance& _gunInstance, objid parentId, bool createGunModel, bool showThirdPersonGun, std::function<objid(objid)> getWeaponParentId, ThirdPersonWeapon thirdPersonWeapon);
void changeGunAnimate(GunInstance& _weaponValues, std::string gun);
void removeGun(GunInstance& weaponValues);

void deliverAmmo(objid inventory, std::string gunName, int ammo);

struct AmmoInfo { 
  int currentAmmo;
  int totalAmmo;
};


std::vector<HitObject> doRaycast(glm::vec3 orientationOffset, glm::vec3 pos, glm::quat rotation, std::optional<int> mask);
std::vector<HitObject> doRaycastClosest(objid playerId, glm::vec3 orientationOffset, std::optional<objid> excludeHitpoint, std::optional<int> mask);

struct GunFireInfo {
  bool didFire;
  std::optional<float> bloomAmount;
};

struct FiringTransform {
  glm::vec3 position;
  glm::quat rotation;
};
GunFireInfo fireGunAndVisualize(GunCore& gunCore, bool holding, bool fireOnce, std::optional<objid> gunId, std::optional<objid> muzzleId, objid id, objid inventory, FiringTransform& transform, bool isInShootingMode);

// Sway gun is completely comestic, no effect on gameplay
void swayGun(GunInstance& weaponValues, bool isGunZoomed, objid playerId, glm::vec2 lookVelocity, glm::vec3 movementVec);
