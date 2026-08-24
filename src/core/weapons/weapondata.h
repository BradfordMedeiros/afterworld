#pragma once 

#include "../../../../ModEngine/src/cscript/cscript_binding.h"

struct WeaponParams {
  std::string name;
  float firingRate = 1.f;
  bool canHold = false;
  bool isIronsight = false;
  bool isRaycast = true;

  float minBloom = 0.f;
  float totalBloom = 1.f;
  float bloomLength = 1.f;

  int totalAmmo = 0;

  // model specific
  float recoilLength = 0.f;
  float recoilPitchRadians = 0.f;
  glm::vec3 recoilTranslate = glm::vec3(0.f, 0.f, 0.f);
  glm::vec3 recoilZoomTranslate = glm::vec3(0.f, 0.f, 0.f);

  std::optional<std::string> fireAnimation;
  std::optional<std::string> idleAnimation;
  glm::vec3 initialGunPos = glm::vec3(0.f, 0.f, 0.f);
  glm::quat initialGunRot = glm::identity<glm::quat>();
  glm::vec4 initialGunRotVec4 = glm::vec4(0.f, 0.f, -1.f, 0.f);
  glm::quat ironSightAngle = glm::identity<glm::quat>();
  glm::vec4 initialIronSightAngle = glm::vec4(0.f, 0.f, -1.f, 0.f);
  glm::vec3 ironsightOffset = glm::vec3(0.f, 0.f, 0.f);

  glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f);
  std::string soundpath;
  std::string modelpath;

  std::string muzzleParticleStr;
  std::string hitParticleStr;
  std::string projectileParticleStr;

  float damage = 0.f;
};

WeaponParams& getWeaponParamsByGunName(std::string gunName);
std::vector<std::string> getWeaponNames();
std::optional<std::string> selectedWeapon();
void setSelectedWeapon(std::string gunName);

void initWeaponsFromConfig();
void saveWeaponJson(std::string gunName);
