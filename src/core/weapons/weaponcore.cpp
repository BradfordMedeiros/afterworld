#include "./weaponcore.h"

extern CustomApiBindings* gameapi;
extern bool disableAnimation;


void doDamageMessage(objid targetId, float damage);
int ammoForGun(objid inventory, std::string& gun);
void setGunAmmo(objid inventory, std::string gun, int currentAmmo);
bool maybeAddGlassBulletHole(objid id, objid playerId);
void drawDebugRaycast(glm::vec3 fromPosition, glm::vec3 toPos, objid playerId);
void emitBlood(objid sceneId, objid lookAtId, glm::vec3 position);
void emitWaterSplash(objid sceneId, objid lookAtId, glm::vec3 position);
std::optional<objid> getEntityForPlayerIndex(int playerIndex);
int getDefaultPlayerIndex();

std::set<objid> entityIdsToEnableForShooting(objid entityId);


std::optional<objid> findChildObjBySuffix(objid id, const char* objName);
objid createWeaponInstance(WeaponParams& weaponParams, objid sceneId, objid parentId, std::string& weaponName, std::function<objid(objid)> getWeaponParentId){
  std::unordered_map<std::string, AttributeValue> attrAttributes = { 
    { "mesh", weaponParams.modelpath }, 
    { "layer", "no_depth" },
    { "rotation", weaponParams.initialGunRotVec4 },
    { "position", weaponParams.initialGunPos - glm::vec3(0.f, 0.f, 0.f) },
    { "scale", weaponParams.scale },
    //{ "tint", glm::vec4(1.f, 1.f, 1.f, 0.4f) },
  };
 
  GameobjAttributes attr {
    .attr = attrAttributes,
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  submodelAttributes["sight"] = GameobjAttributes{};
  submodelAttributes.at("sight").attr["tint"] = glm::vec4(1.f, 0.f, 0.f, 1.f);

  auto gunId = gameapi -> makeObjectAttr(sceneId, weaponName, attr, submodelAttributes);
  modassert(gunId.has_value(), std::string("weapons could not create gun: ") + weaponParams.name);
  gameapi -> makeParent(gunId.value(), getWeaponParentId(parentId));


  auto screen = findChildObjBySuffix(gunId.value(), "sight");
  if (screen.has_value()){
    gameapi -> setSingleGameObjectAttr(screen.value(), "tint", glm::vec4(0.f, 0.f, 1.f, 0.4f));
    gameapi -> setSingleGameObjectAttr(screen.value(), "layer", "transparency");
  }

  return gunId.value();
}

std::optional<objid> createThirdPersonWeaponInstance(WeaponParams& weaponParams, objid sceneId, objid parentId, ThirdPersonWeapon thirdPersonWeapon, std::string& weaponName){
  auto entityHandId = thirdPersonWeapon.getWeaponParentId(parentId);
  if (!entityHandId.has_value()){
    return std::nullopt;
  }

  std::unordered_map<std::string, AttributeValue> attrAttributes = { 
    { "mesh", weaponParams.modelpath }, 
    //{ "rotation", glm::vec4(-1.f, 0.f, 0.f, 180.f) }, // the default rotation here should probably be set to be relative to the parent but forward...doesnt matter since update hand pos abs elsewhere
    { "position", glm::vec3(0.f, 0.0f, -0.5f) },

    //{ "rotation", glm::vec4(0.f, 0.f, -1.f, 270.f) },
    //{ "position", glm::vec3(0.f, 0.4f, -0.f) },

  };
  GameobjAttributes attr { .attr = attrAttributes };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto gunId = gameapi -> makeObjectAttr(sceneId, weaponName, attr, submodelAttributes);
  modassert(gunId.has_value(), std::string("weapons could not create gun: ") + weaponParams.name);

  gameapi -> makeParent(gunId.value(), entityHandId.value());
  return gunId.value();

}

GunCore createGunCoreInstance(std::string gun, objid sceneId){
  modlog("weapons", std::string("create gun: ") + gun);

  auto& weaponParams = getWeaponParamsByGunName(gun);
  WeaponState newState {
    .lastShootingTime = -1.f * weaponParams.firingRate, // so you can shoot immediately
    .recoilStart = 0.f,
    .gunState = GUN_RAISED,
  };

  GunCore gunCore {
    .weaponParams = &weaponParams,
    .weaponState = newState,
  };
  return gunCore;
}

std::optional<std::string*> getCurrentGunName(GunInstance& weaponValues){
  if (weaponValues.gunCore.weaponParams == NULL){
    return std::nullopt;
  }
  return &weaponValues.gunCore.weaponParams -> name;
}

void ensureGunInstance(GunInstance& _gunInstance, objid parentId, bool createGunModel, bool showThirdPersonGunModel, std::function<objid(objid)> getWeaponParentId, ThirdPersonWeapon thirdPersonWeapon){
  auto elapsedTimeSinceChange = gameapi -> timeSeconds(false) - _gunInstance.changeGunTime; 
  if (elapsedTimeSinceChange  < 0.5f){
    //modlog("ensure gun instance weapons not enough time", std::to_string(elapsedTimeSinceChange));
    return;
  }
  auto currentGun = getCurrentGunName(_gunInstance);
  if (!currentGun.has_value() && _gunInstance.desiredGun == ""){
    return;
  }


  bool sameGun = currentGun.has_value() && (*currentGun.value() == _gunInstance.desiredGun);

  bool needToDeleteFpsGun = (_gunInstance.gunId.has_value() && !sameGun) || (_gunInstance.gunId.has_value() && !createGunModel);
  bool needToCreateFpsGun = (!sameGun && createGunModel) || (sameGun && createGunModel && !_gunInstance.gunId.has_value());

  bool needToDeleteTpsGun =  (_gunInstance.thirdPersonGunId.has_value() && !sameGun) || (_gunInstance.thirdPersonGunId.has_value() && !showThirdPersonGunModel);
  bool needToCreateTpsGun = (!sameGun && showThirdPersonGunModel) || (sameGun && showThirdPersonGunModel && !_gunInstance.thirdPersonGunId.has_value());

  if (needToDeleteFpsGun){
    modlog("weapons ensureGunInstance", "deleting weapon instance");
    gameapi -> removeByGroupId(_gunInstance.gunId.value());
    _gunInstance.gunId = std::nullopt;
  }


  std::optional<objid> muzzlePointId;
  auto sceneId = gameapi -> listSceneId(parentId);

  if (needToCreateFpsGun){
    auto gunCore = createGunCoreInstance(_gunInstance.desiredGun, 0); // would be better to preload all gun cores, also this should just be optional
    auto weaponName = std::string("code-weapon-") + uniqueNameSuffix();
    modlog("weapons ensureGunInstance", "creating weapon instance");
    _gunInstance.gunId = createWeaponInstance(*gunCore.weaponParams, sceneId, parentId, weaponName, getWeaponParentId);

    if (gunCore.weaponParams -> idleAnimation.has_value() && gunCore.weaponParams -> idleAnimation.value() != "" && _gunInstance.gunId.has_value()){
      gameapi -> playAnimation(_gunInstance.gunId.value(), gunCore.weaponParams -> idleAnimation.value(), LOOP, std::nullopt, 0, false, std::nullopt);
    }
    muzzlePointId = gameapi -> getGameObjectByName(weaponName + "/muzzle", sceneId);
    if (!muzzlePointId.has_value()){
      modlog("weapon core", std::string("no muzzle defined for: ") + _gunInstance.desiredGun);
    }
  }

  if (needToDeleteTpsGun){
    modlog("weapons ensureGunInstance third person", "deleting weapon instance");
    gameapi -> removeByGroupId(_gunInstance.thirdPersonGunId.value());
    _gunInstance.thirdPersonGunId = std::nullopt;
  }
  if (needToCreateTpsGun){
    auto gunCore = createGunCoreInstance(_gunInstance.desiredGun, 0); // would be better to preload all gun cores, also this should just be optional
    modlog("weapons ensureGunInstance third person", "create weapon instance");
    auto weaponName = std::string("code-weapon-third-") + uniqueNameSuffix();
    _gunInstance.thirdPersonGunId = createThirdPersonWeaponInstance(*gunCore.weaponParams, sceneId, parentId, thirdPersonWeapon, weaponName);
  }

  if (!sameGun){
    auto gunCore = createGunCoreInstance(_gunInstance.desiredGun, 0); // would be better to preload all gun cores, also this should just be optional
    _gunInstance.gunCore = gunCore;
    _gunInstance.muzzleId = muzzlePointId;
  }

}

void changeGunAnimate(GunInstance& weaponValues, std::string gun){
  if (weaponValues.gunCore.weaponParams != NULL && weaponValues.gunCore.weaponParams -> name == gun){
    modlog("weapons change gun animation - weapon already equipped", gun);
    return;
  }
  weaponValues.gunCore.weaponState.gunState = GUN_LOWERING;
  weaponValues.desiredGun = gun;
  weaponValues.changeGunTime = gameapi -> timeSeconds(false);
}

// probably this shouldn't reset the state, just remove objects 
void removeGun(GunInstance& weaponValues){
  if (weaponValues.gunId.has_value()){
    modlog("weapons core", "remove gun");
    gameapi -> removeByGroupId(weaponValues.gunId.value());
    weaponValues.gunId = std::nullopt;
    weaponValues.muzzleId = std::nullopt; 
    weaponValues.gunCore.weaponParams = NULL;
  }

  if (weaponValues.thirdPersonGunId.has_value()){
    gameapi -> removeByGroupId(weaponValues.thirdPersonGunId.value());
  }
}


void deliverAmmo(objid inventory, std::string gunName, int ammo){
  auto oldAmmo = ammoForGun(inventory, gunName);
  setGunAmmo(inventory, gunName, oldAmmo + ammo);
}

bool canFireGunNow(GunCore& gunCore, float elapsedMilliseconds){
  auto timeSinceLastShot = elapsedMilliseconds - gunCore.weaponState.lastShootingTime;
  bool lessThanFiringRate = timeSinceLastShot >= (0.001f * gunCore.weaponParams -> firingRate);
  return lessThanFiringRate;
}

// fires from point of view of the camera
float maxRaycastDistance = 500.f;
std::vector<HitObject> doRaycast(glm::vec3 orientationOffset, glm::vec3 mainobjPos, glm::quat mainobjRotation, std::optional<int> mask){
  auto orientationOffsetQuat = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), orientationOffset);
  auto rot = mainobjRotation *  orientationOffsetQuat;
  auto hitpoints =  gameapi -> raycast(mainobjPos, rot, maxRaycastDistance, mask);
  return hitpoints;
}

std::vector<HitObject> doRaycastClosest(glm::vec3 cameraPos, glm::quat cameraRotation, glm::vec3 orientationOffset, std::optional<objid> excludeHitpoint, std::optional<int> mask){
  auto hitpoints = doRaycast(orientationOffset, cameraPos, cameraRotation, mask);
  if (hitpoints.size() > 0){
    auto closestIndex = closestHitpoint(hitpoints, cameraPos, excludeHitpoint);
    if (!closestIndex.has_value()){
      return hitpoints;
    }
    return { hitpoints.at(closestIndex.value()) };
  }
  return hitpoints;
}

std::vector<HitObject> doRaycastClosest(objid playerId, glm::vec3 orientationOffset, std::optional<objid> excludeHitpoint, std::optional<int> mask){
  auto mainobjPos = gameapi -> getGameObjectPos(playerId, true, "[gamelogic] doRaycastClosest pos");
  auto mainobjRotation = gameapi -> getGameObjectRotation(playerId, true, "[gamelogic] doRaycastClosest rot"); // tempchecked
  return doRaycastClosest(mainobjPos, mainobjRotation, orientationOffset, excludeHitpoint, mask); 
}

glm::vec3 zFightingForParticle(glm::vec3 pos, glm::quat normal){
  return gameapi -> moveRelative(pos, normal, 0.01);  // 0.01?
}

void fireRaycast(GunCore& gunCore, glm::vec3 orientationOffset, objid playerId, std::vector<MaterialToParticle>& materials, glm::vec3 cameraPos, glm::quat cameraRotation){
  int mask = bonesAndObjects();
  auto hitpoints = doRaycastClosest(cameraPos, cameraRotation, orientationOffset, playerId, mask);

  std::string raycastHits = "";
  for (auto &hitpoint : hitpoints){
    raycastHits += gameapi -> getGameObjNameForId(hitpoint.id).value() + " ";
  }
  modlog("weapons", "fire raycast, total hits = " + std::to_string(hitpoints.size()) + " " + raycastHits);

  for (auto &hitpoint : hitpoints){
    drawDebugRaycast(cameraPos, hitpoint.point, playerId);
    drawDebugHitmark(hitpoint, playerId);

    modlog("weapons", "raycast hit: " + std::to_string(hitpoint.id) + "- point: " + print(hitpoint.point) + ", normal: " + print(hitpoint.normal));
    auto objMaterial = materialTypeForObj(hitpoint.id);
    if (!objMaterial.has_value()){
      objMaterial = "default";
    }

    std::optional<objid> soundEmitterId = std::nullopt;
    std::optional<objid> emitterId = std::nullopt;
    std::optional<objid> splashEmitterId = std::nullopt;

    if (objMaterial.has_value()){
      auto material = getHitMaterial(materials, objMaterial.value());
      std::cout << "hit particle material: (" << (material.has_value() && material.value() -> hitParticle.has_value() ? "has hit particle" : "no hit particle" ) << ") " << std::endl;
      if (material.has_value() && material.value() -> hitParticle.has_value()){
        emitterId = material.value() -> hitParticle.value().particleId;
      }
      if (material.has_value()){
        soundEmitterId = material.value() -> hitParticleClipId;
      }
      if (material.has_value() && material.value() -> splashParticle.has_value()){
        splashEmitterId = material.value() -> splashParticle.value().particleId;
      }
    }
    auto addedGlassDecal = maybeAddGlassBulletHole(hitpoint.id, playerId);
    auto emitParticlePosition = zFightingForParticle(hitpoint.point, hitpoint.normal);

    if (soundEmitterId.has_value()){
      playGameplayClipById(soundEmitterId.value(), std::nullopt, hitpoint.point, false);
    }

    if (splashEmitterId.has_value()){
      gameapi -> emit(splashEmitterId.value(), emitParticlePosition, hitpoint.normal, std::nullopt, std::nullopt, std::nullopt);
    }

    auto activePlayerId = getEntityForPlayerIndex(getDefaultPlayerIndex());
    auto inFront = hitpoint.point + (hitpoint.normal * glm::vec3(0.f, 0.f, -0.1f));
    //emitBlood(rootSceneId(), activePlayerId.value(), inFront);
  
    doDamageMessage(hitpoint.id, gunCore.weaponParams -> damage);
    modlog("weapons", "raycast normal: " + serializeQuat(hitpoint.normal));
  }
}


bool tryFireGun(objid inventory, std::optional<objid> gunId, std::optional<objid> muzzleId, GunCore& gunCore, float bloomAmount, objid playerId, glm::vec3 playerPos, glm::quat playerRotation, std::vector<MaterialToParticle>& materials){  
  float now = gameapi -> timeSeconds(false);
  auto canFireGun = canFireGunNow(gunCore, now);
  modlog("weapons", std::string("try fire gun, can fire = ") + (canFireGun ? "true" : "false") + ", now = " + std::to_string(now) + ", firing rate = " + std::to_string(gunCore.weaponParams -> firingRate));
  if (!canFireGun){
    return false;
  }
  bool hasAmmo = ammoForGun(inventory, gunCore.weaponParams -> name) > 0;
  if (!hasAmmo){
    modlog("weapons", "no ammo, tried to fire, should play sound");
    return false;
  }

  if (gunCore.weaponParams != NULL){
    deliverAmmo(inventory, gunCore.weaponParams -> name, -1);
  }

  playMixedSound(getSymbol(gunCore.weaponParams -> soundpath), playerPos);

  if (gunId.has_value()){
    if (muzzleId.has_value()){
      auto muzzlePosition = gameapi -> getGameObjectPos(muzzleId.value(), true, "[gamelogic] tryFireGun - find muzzle position");
      std::cout << "muzzle emit: " << print(muzzlePosition) << std::endl;
      //gameapi -> emit(gunCore.weaponCore -> weaponParams -> muzzleParticleStr, muzzlePosition, playerRotation /* should this be the muzzle rotation? */, std::nullopt, std::nullopt, playerId);
      emitParticle(getSymbol(gunCore.weaponParams -> muzzleParticleStr), muzzlePosition, playerRotation);
    }else{
      auto gunPosition = gameapi -> getGameObjectPos(gunId.value(), true, "[gamelogic] tryFireGun - get gun position");
      glm::vec3 distanceFromGun = glm::vec3(0.f, 0.f, -1.); // should parameterize particleOffset
      auto slightlyInFrontOfGun = gameapi -> moveRelativeVec(gunPosition, playerRotation, distanceFromGun);
      emitParticle(getSymbol(gunCore.weaponParams -> muzzleParticleStr), slightlyInFrontOfGun, playerRotation);
    }
  }
  gunCore.weaponState.lastShootingTime = now;
  gunCore.weaponState.recoilStart = gameapi -> timeSeconds(false);

  glm::vec3 shootingVecAngle(randomNumber(-bloomAmount, bloomAmount), randomNumber(-bloomAmount, bloomAmount), -1.f);
  if (gunCore.weaponParams -> isRaycast){
    fireRaycast(gunCore, shootingVecAngle, playerId, materials, playerPos, playerRotation);
  }
  if (gunCore.weaponParams -> projectileParticleStr != ""){
    auto fromPos = gameapi -> moveRelative(playerPos, playerRotation, 3);
    glm::vec3 projectileArc(0.f, 0.f, -1.f);
    auto playerForwardAndUp = playerRotation * gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), projectileArc);
    auto initialVelocity = playerForwardAndUp * shootingVecAngle * 10.f;
    emitParticle(getSymbol(gunCore.weaponParams -> projectileParticleStr), fromPos, playerRotation, initialVelocity);
  }

  if (gunId.has_value() && gunCore.weaponParams -> fireAnimation.has_value()){
    modlog("animation fire gun", gunCore.weaponParams -> fireAnimation.value());
    gameapi -> playAnimation(gunId.value(), gunCore.weaponParams -> fireAnimation.value(), ONESHOT, std::nullopt, 0, false, std::nullopt);
  }

  if (!disableAnimation){
    gameapi -> playAnimation(playerId, "rifle-fire", ONESHOT, entityIdsToEnableForShooting(playerId), 1, false, 0.1f);
  }

  return true;
}

float calcRecoilSlerpAmount(GunCore& gunCore, float length,  bool reset){
  float amount = (gameapi -> timeSeconds(false) - gunCore.weaponState.recoilStart) / length;
  return (amount > 1.f) ? (reset ? 0.f : 1.f): amount;
}

float calculateBloomAmount(GunCore& gunCore){
  auto slerpAmount = (1 - calcRecoilSlerpAmount(gunCore, gunCore.weaponParams -> bloomLength, false)); 
  modassert(slerpAmount <= 1, "slerp amount must be less than 1, got: " + std::to_string(slerpAmount));
  return glm::max(gunCore.weaponParams -> minBloom, (gunCore.weaponParams -> totalBloom - gunCore.weaponParams -> minBloom) * slerpAmount + gunCore.weaponParams -> minBloom);
}

GunFireInfo fireGunAndVisualize(GunCore& gunCore, bool holding, bool fireOnce, std::optional<objid> gunId, std::optional<objid> muzzleId, objid id, objid inventory, FiringTransform& transform, bool isInShootingMode){
  if (gunCore.weaponParams == NULL || !isInShootingMode){
    modlog("fire gun", "no weaponCore");
    return GunFireInfo { .didFire = false, .bloomAmount = std::nullopt };
  }
  auto bloomAmount = calculateBloomAmount(gunCore);
  if ((gunCore.weaponParams -> canHold && holding) || fireOnce){
    bool didFire = tryFireGun(inventory, gunId, muzzleId, gunCore, bloomAmount, id, transform.position, transform.rotation, getMaterials());
    return GunFireInfo { .didFire = didFire, .bloomAmount = bloomAmount };
  }
  return GunFireInfo { .didFire = false, .bloomAmount = bloomAmount };
}


glm::vec3 calcLocationWithRecoil(GunInstance& weaponValues, glm::vec3 pos, bool isGunZoomed){
  auto targetPos = isGunZoomed ? weaponValues.gunCore.weaponParams -> ironsightOffset : pos;
  auto recoilAmount = isGunZoomed ? weaponValues.gunCore.weaponParams -> recoilZoomTranslate : weaponValues.gunCore.weaponParams -> recoilTranslate;
  auto targetPosWithRecoil = glm::vec3(targetPos.x + recoilAmount.x, targetPos.y + recoilAmount.y, targetPos.z + recoilAmount.z);
  return glm::lerp(targetPos, targetPosWithRecoil, calcRecoilSlerpAmount(weaponValues.gunCore, weaponValues.gunCore.weaponParams -> recoilLength, true));
}

glm::vec3 smoothVelocity(glm::vec3 lookVelocity){
  return lookVelocity;
  static glm::vec3 smoothedVel2(0.0f);
  float velSmoothFactor = 100.0f; // higher = follows player movement faster
  float dt = gameapi -> timeElapsed();
  smoothedVel2 += (lookVelocity - smoothedVel2) * (dt * velSmoothFactor);

  auto smoothedVel = smoothedVel2;
  //if (smoothedVel.x < 0.1 && smoothedVel.x > -0.1){
  //  smoothedVel.x = 0;
  //}
  //if (smoothedVel.y < 0.1 && smoothedVel.y > -0.1){
  //  smoothedVel.y = 0;
  //}
  //if (smoothedVel.z < 0.1 && smoothedVel.z > -0.1){
  //  smoothedVel.z = 0;
  //}
  return smoothedVel;
}

glm::vec2 smoothVelocity(glm::vec2 lookVelocity){
  static glm::vec2 smoothedVel2(0.0f);
  float velSmoothFactor = 20.2f; // higher = follows player movement faster
  float dt = gameapi -> timeElapsed();
  smoothedVel2 += (lookVelocity - smoothedVel2) * (dt * velSmoothFactor);

  auto smoothedVel = smoothedVel2;
  //if (smoothedVel.x < 0.1 && smoothedVel.x > -0.1){
  //  smoothedVel.x = 0;
  //}
  //if (smoothedVel.y < 0.1 && smoothedVel.y > -0.1){
  //  smoothedVel.y = 0;
  //}
  //if (smoothedVel.z < 0.1 && smoothedVel.z > -0.1){
  //  smoothedVel.z = 0;
  //}
  return smoothedVel;
}


glm::vec3 createNoise(){
  glm::vec3 noiseOffset(0.f, 0.f, 0.f);
  float time = gameapi -> timeSeconds(false); // seconds
  noiseOffset.x = 0.2 * sin(time * 1.3f) * 0.02f;
  noiseOffset.y = 0.2 * cos(time * 0.7f) * 0.015f;
  noiseOffset.z = 0.2 * sin(time * 0.9f + 2.0f) * 0.01f;
  return noiseOffset;
}

glm::vec3 maxMagSway(0.1f, 0.1f, 0.05f);
glm::vec3 maxMagSwayRot(0.2f, 0.2f, 0.2f);

float zoomSpeedMultiplier = 5.f;
float swayVelocity = 1.;

bool swayTranslation = true;
bool swayRotation = true;

glm::vec3 springModel(glm::vec3 basePos, glm::vec3 targetPos, bool isGunZoomed, glm::vec3 currentPos){
  static glm::vec3 velocity(0.0f); // static velocity...okay...maybe should reset this on a gun id or something

  float time = gameapi -> timeElapsed();

  float stiffness = isGunZoomed ? 1400.0f : 1000.0f; // return-to-target speed
  float damping   = isGunZoomed ? 105.0f : 56.0f;  // how much "friction" slows velocity

  // Spring-damper formula
  glm::vec3 displacement = targetPos - currentPos;
  glm::vec3 accel = displacement * stiffness - velocity * damping; // dividing this is effectly mass

  velocity += accel * time;
  glm::vec3 newPos = currentPos + (velocity * time);

  return newPos;
}

glm::vec3 lerpModel(glm::vec3 targetPosWithRecoil, glm::vec3 oldGunPos, bool isGunZoomed){
  float lerpAmount = gameapi -> timeElapsed() * swayVelocity * (isGunZoomed ? zoomSpeedMultiplier : 1.f) * 3.f;
  auto newPos = glm::lerp(oldGunPos, targetPosWithRecoil, lerpAmount);  // probably pick a better function?  how does it feel tho
  return newPos;
}

glm::vec3 debugModel(glm::vec3 basePos, glm::vec3 targetPos, bool isGunZoomed, glm::vec3 currentPos){
  return targetPos;
}

glm::vec3 swayGunTranslation(GunInstance& weaponValues, bool isGunZoomed, objid gunId, glm::vec3 lookVelocity, glm::vec3 movementVec){
  auto normalizedMovement = movementVec;
  if (glm::length(movementVec) < 0.1f && glm::length(movementVec) > -0.1f){
    normalizedMovement = glm::vec3(0.f, 0.f, 0.f);
  }else{
    normalizedMovement = glm::normalize(movementVec);
  }
  auto smoothedVelocity = smoothVelocity(glm::vec3(maxMagSway.x * normalizedMovement.x, maxMagSway.y *  normalizedMovement.y, maxMagSway.z *  normalizedMovement.z));
  modlog("sway gun smooth", print(smoothedVelocity));
  auto oldGunPos = gameapi -> getGameObjectPos(gunId, false, "[gamelogic] swayGunTranslation - find gun position");
  
  glm::vec3 maxSway = maxMagSway * (isGunZoomed ? 0.3f : 1.0f);
  glm::vec3 basePos = weaponValues.gunCore.weaponParams -> initialGunPos;
  glm::vec3 targetOffset = glm::clamp(-smoothedVelocity, -maxSway, maxSway) + basePos;
  auto targetPosWithRecoil = calcLocationWithRecoil(weaponValues, targetOffset, isGunZoomed); // this should use ironsight-offset
  glm::vec3 targetPos = targetPosWithRecoil + createNoise();

  return springModel(basePos, targetPos, isGunZoomed, oldGunPos);
  //return debugModel(basePos, targetPos, isGunZoomed, oldGunPos);
  //return lerpModel(targetPos, oldGunPos, isGunZoomed);
}

glm::quat lerpModel(glm::quat& oldRotation, glm::quat& targetRotation){
  auto newRotation = glm::slerp(oldRotation, targetRotation, 0.1f);
  return newRotation;
}

glm::quat springModel(glm::quat& oldRotation, glm::quat& targetRotation, glm::quat& currentRotation){
  return targetRotation;
}

glm::quat debugModel(glm::quat& oldRotation, glm::quat& targetRotation, glm::quat& currentRotation){
  return targetRotation;
}



glm::quat swayGunRotation(GunInstance& weaponValues, bool isGunZoomed, glm::vec2 lookVelocity, glm::vec3 movementVec, objid gunId){
  auto oldRotation = gameapi -> getGameObjectRotation(gunId, false, "[gamelogic] swayGunRotation");


  auto smoothedLookVelocity = lookVelocity;

  float limitedSwayX = glm::min(maxMagSwayRot.x, glm::max(5 * smoothedLookVelocity.x, -1.f * maxMagSwayRot.x));
  float limitedSwayY = glm::min(maxMagSwayRot.y, glm::max(-5 * smoothedLookVelocity.y, -1.f * maxMagSwayRot.y));
  float recoilAmount = glm::lerp(
    glm::vec3(0.f, 0.f, 0.f), 
    glm::vec3(0.f, weaponValues.gunCore.weaponParams -> recoilPitchRadians, 0.f), 
    calcRecoilSlerpAmount(weaponValues.gunCore, weaponValues.gunCore.weaponParams -> recoilLength, true)
  ).y;

  float totalSwayY = limitedSwayY + recoilAmount * 5.f;

  auto rotation = gameapi -> setFrontDelta(parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)), limitedSwayX, totalSwayY, 0.f, 0.1f);
  auto targetRotation = rotation * (isGunZoomed ? weaponValues.gunCore.weaponParams -> ironSightAngle : weaponValues.gunCore.weaponParams -> initialGunRot);
  
  return lerpModel(oldRotation, targetRotation);
  //return debugModel(oldRotation, targetRotation, oldRotation);
}


void swayGun(GunInstance& weaponValues, bool isGunZoomed, objid playerId, glm::vec2 lookVelocity, glm::vec3 movementVec){
  if (!weaponValues.gunId.has_value()){
    return;
  }

  bool shouldZoomGun = isGunZoomed && weaponValues.gunCore.weaponParams -> isIronsight;
  //modlog("weapon", "movement velocity: " + std::to_string(weapons.movementVelocity));
  //modlog("weapon", "sway velocity: " + print(swayVelocity));

  if (swayTranslation){
    auto relativeMovement = glm::inverse(gameapi -> getGameObjectRotation(playerId, false, "[gamelogic] getSwayVelocity")) * movementVec;
    auto newPos = swayGunTranslation(weaponValues, shouldZoomGun, weaponValues.gunId.value(), glm::vec3(lookVelocity.x, lookVelocity.y, 0.f), relativeMovement);
    gameapi -> setGameObjectPosition(weaponValues.gunId.value(), newPos, false, Hint { .hint = "swayGunTranslation" }); 
  }
  if (swayRotation){
    auto newRotation = swayGunRotation(weaponValues, shouldZoomGun, lookVelocity, movementVec, weaponValues.gunId.value());
    gameapi -> setGameObjectRot(weaponValues.gunId.value(), newRotation, false, Hint { .hint = "swayGunRotation" });
  }
}

