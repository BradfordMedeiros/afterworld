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
};

/*
gunType := registerGunType(name)
gunType := getGunType(name)
fireGun(gunType, glm::vec3(fromPos))

gunInstance = createGunInstance(gunType, parent) // actually created the gun model and stuff
fireGun(gunInstance)*/
WeaponParams queryWeaponParams(std::string gunName);


struct WeaponInstance {
  objid gunId;
  std::optional<objid> soundId;
  std::optional<std::string> soundClipObj;

  std::optional<objid> muzzleParticle;  // particle right in front of the muzzle, eg for a smoke effect
  std::optional<objid> hitParticles;    // default hit particle for the gun, used if there is no material particle
  std::optional<objid> projectileParticles;  // eg for a grenade launched from the gun
};

WeaponInstance createWeaponInstance(WeaponParams& weaponParams, objid sceneId, objid playerId);
void removeWeaponInstance(WeaponInstance& weaponInstance);

enum GunAnimation { GUN_RAISED, GUN_LOWERING };
struct WeaponState {
  float lastShootingTime;
  float recoilStart;
  int currentAmmo;
  GunAnimation gunState;
};

struct WeaponValues {
  WeaponParams weaponParams;
  std::optional<WeaponInstance> weaponInstance;
  WeaponState weaponState;
};
void changeGun(WeaponValues& _weaponValues, std::string gun, int ammo, objid sceneId, objid playerId);
void deliverAmmo(WeaponValues& _weaponValues, int ammo);

void saveGunTransform(WeaponValues& weaponValues);

std::vector<HitObject> doRaycast(glm::vec3 orientationOffset, objid playerId);
void tryFireGun(WeaponValues& weaponValues, objid sceneId, float bloomAmount, objid playerId, std::vector<MaterialToParticle>& materials);

// Sway gun is completely comestic, no effect on gameplay
void swayGun(WeaponValues& weaponValues, bool isGunZoomed, objid playerId, glm::vec2 lookVelocity, glm::vec3 movementVec);

float calcRecoilSlerpAmount(WeaponValues& weaponValues, float length,  bool reset);

void registerGunType(std::string gunName);

#endif