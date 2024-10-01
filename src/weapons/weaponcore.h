#ifndef MOD_AFTERWORLD_WEAPON_CORE
#define MOD_AFTERWORLD_WEAPON_CORE

#include <string>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../materials.h"

struct WeaponParams {
  // gun intrinsic stuff
  std::string name;
  float firingRate;
  bool canHold;
  bool isIronsight;
  bool isRaycast;

  float minBloom;
  float totalBloom;
  float bloomLength;

  int totalAmmo;

  // model specific
  float recoilLength;
  float recoilPitchRadians;
  glm::vec3 recoilTranslate;
  glm::vec3 recoilZoomTranslate;

  std::optional<std::string> fireAnimation;
  std::optional<std::string> idleAnimation;
  glm::vec3 initialGunPos;
  glm::quat initialGunRot;
  glm::vec4 initialGunRotVec4;
  glm::quat ironSightAngle;
  glm::vec3 ironsightOffset;

  glm::vec3 scale;
  std::string soundpath;
  std::string modelpath;
  std::string script;

  std::string muzzleParticleStr;
  std::string hitParticleStr;
  std::string projectileParticleStr;

  float damage;
};

/*
gunType := registerGunType(name)
gunType := getGunType(name)
fireGun(gunType, glm::vec3(fromPos))

gunInstance = createGunInstance(gunType, parent) // actually created the gun model and stuff
fireGun(gunInstance)*/
WeaponParams queryWeaponParams(std::string gunName);


objid createWeaponInstance(WeaponParams& weaponParams, objid sceneId, objid playerId);

enum GunAnimation { GUN_RAISED, GUN_LOWERING };
struct WeaponState {
  float lastShootingTime;
  float recoilStart;
  GunAnimation gunState;
};

struct SoundResource {
  objid clipObjectId;
};

struct WeaponCore {
  std::string name;
  WeaponParams weaponParams;
  std::optional<SoundResource> soundResource;
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
};

GunCore createGunCoreInstance(std::string gun, objid sceneId);
std::optional<std::string*> getCurrentGunName(GunInstance& weaponValues);

void ensureGunInstance(GunInstance& _gunInstance, objid sceneId, objid playerId, bool createGunModel);
void changeGunAnimate(GunInstance& _weaponValues, std::string gun, objid sceneId, objid playerId, bool createGunModel);
void removeGun(GunInstance& weaponValues);

void deliverAmmo(objid inventory, std::string gunName, int ammo);

struct AmmoInfo { 
  int currentAmmo;
  int totalAmmo;
};

void saveGunTransform(GunInstance& weaponValues);

std::vector<HitObject> doRaycast(glm::vec3 orientationOffset, objid playerId);
std::vector<HitObject> doRaycastClosest(glm::vec3 orientationOffset, objid playerId);

struct GunFireInfo {
  bool didFire;
  std::optional<float> bloomAmount;
};
GunFireInfo fireGunAndVisualize(GunCore& gunCore, bool holding, bool fireOnce, std::optional<objid> gunId, std::optional<objid> muzzleId, objid id, objid inventory);

// Sway gun is completely comestic, no effect on gameplay
void swayGun(GunInstance& weaponValues, bool isGunZoomed, objid playerId, glm::vec2 lookVelocity, glm::vec3 movementVec);

#endif