#include "./weaponcore.h"

extern CustomApiBindings* gameapi;

void doDamageMessage(objid targetId, float damage);
int ammoForGun(objid inventory, std::string& gun);
void setGunAmmo(objid inventory, std::string gun, int currentAmmo);
bool maybeAddGlassBulletHole(objid id, objid playerId);

std::vector<WeaponCore> weaponCores = {};

WeaponCore* findWeaponCore(std::string& name){
  for (auto &weaponCore : weaponCores){
    if (weaponCore.name == name){
      return &weaponCore;
    }
  }
  return NULL;
}

void loadWeaponCore(std::string& coreName, objid sceneId, WeaponParams& weaponParams){
  modlog("weapons", std::string("load weapon core: ") + coreName);
  if (findWeaponCore(coreName)){
    return;
  }

  WeaponCore weaponCore { };
  weaponCore.weaponParams = weaponParams;
  if (weaponParams.soundpath != ""){
    GameobjAttributes soundAttr {
      .attr = { { "clip", weaponParams.soundpath }, { "physics", "disabled" }},
    };
    std::string soundClipObj = std::string("&code-weaponsound-") + uniqueNameSuffix();
    std::map<std::string, GameobjAttributes> submodelAttributes;
    auto clipObjectId = gameapi -> makeObjectAttr(sceneId, soundClipObj, soundAttr, submodelAttributes);
    modassert(clipObjectId.has_value(), "load weapon core sound, could not create clip obj");
    SoundResource soundResource {
      .clipObjectId = clipObjectId.value(),
    };
    weaponCore.soundResource = soundResource;
  }
  
  weaponCore.name = coreName;

  weaponCore.muzzleParticle = createParticleEmitter(sceneId, weaponParams.muzzleParticleStr, (std::string("+code-muzzleparticle") + uniqueNameSuffix()));
  weaponCore.hitParticles = createParticleEmitter(sceneId, weaponParams.hitParticleStr, (std::string("+code-hitparticle") + uniqueNameSuffix()));

  if (weaponParams.projectileParticleStr.size() > 0 && weaponParams.projectileParticleStr.at(0) == '?'){
    auto particle = weaponParams.projectileParticleStr.substr(1, weaponParams.projectileParticleStr.size());
    auto particleId = getParticleEmitter(particle);
    modassert(particleId.has_value(), "projectile particle: no particle for: " + particle);
    weaponCore.projectileParticles = particleId.value();
    weaponCore.removeProjectileOnExit = false;
  }else{
    weaponCore.projectileParticles = createParticleEmitter(sceneId, weaponParams.projectileParticleStr, (std::string("+code-projectileparticle") + uniqueNameSuffix()));
    weaponCore.removeProjectileOnExit = true;
  }

  weaponCores.push_back(weaponCore);
}

void unloadWeaponCore(WeaponCore& weaponCore){
  if (weaponCore.soundResource.has_value()){
    gameapi -> removeByGroupId(weaponCore.soundResource.value().clipObjectId);
  }
  if (weaponCore.muzzleParticle.has_value()){
    gameapi -> removeByGroupId(weaponCore.muzzleParticle.value());
  }
  if (weaponCore.hitParticles.has_value()){
    gameapi -> removeByGroupId(weaponCore.hitParticles.value());
  }
  if (weaponCore.projectileParticles.has_value() && weaponCore.removeProjectileOnExit){
    gameapi -> removeByGroupId(weaponCore.projectileParticles.value());
  }
}

void removeAllWeaponCores(){
  for (auto &weaponCore : weaponCores){
    unloadWeaponCore(weaponCore);
  }
  weaponCores = {};
}


WeaponParams queryWeaponParams(std::string gunName){
  auto gunQuery = gameapi -> compileSqlQuery(
    std::string("select modelpath, fireanimation, fire-sound, xoffset-pos, ") +
    "yoffset-pos, zoffset-pos, xrot, yrot, zrot, xscale, yscale, zscale, " + 
    "firing-rate, hold, raycast, ironsight, iron-xoffset-pos, iron-yoffset-pos, " + 
    "iron-zoffset-pos, particle, hit-particle, recoil-length, recoil-angle, " + 
    "recoil, recoil-zoom, projectile, bloom, script, fireanimation, idleanimation, bloom-length, minbloom, ironsight-rot, ammo, damage " + 
    "from guns where name = ?",
    { gunName }
  );

  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(gunQuery, &validSql);
  modassert(validSql, "error executing sql query");

  modassert(result.size() > 0, "no gun named: " + gunName);
  modassert(result.size() == 1, "more than one gun named: " + gunName);
  modlog("weapons", "gun: result: " + print(result.at(0)));

	WeaponParams weaponParams {};
  weaponParams.name = gunName;
  weaponParams.firingRate = floatFromFirstSqlResult(result, 12);
  weaponParams.recoilLength = floatFromFirstSqlResult(result, 21);

  weaponParams.recoilPitchRadians = floatFromFirstSqlResult(result, 22);
  weaponParams.recoilTranslate = vec3FromFirstSqlResult(result, 23);
  weaponParams.recoilZoomTranslate = vec3FromFirstSqlResult(result, 24);
  weaponParams.canHold = boolFromFirstSqlResult(result, 13);
  weaponParams.isIronsight = boolFromFirstSqlResult(result, 15);
  weaponParams.isRaycast = boolFromFirstSqlResult(result, 14);
  weaponParams.ironsightOffset = vec3FromFirstSqlResult(result, 16, 17, 18);
  weaponParams.minBloom = floatFromFirstSqlResult(result, 31);
  weaponParams.totalBloom = floatFromFirstSqlResult(result, 26);
  weaponParams.bloomLength = floatFromFirstSqlResult(result, 30);
  weaponParams.totalAmmo = intFromFirstSqlResult(result, 33);
  weaponParams.damage = floatFromFirstSqlResult(result, 34);

  auto fireAnimation = strFromFirstSqlResult(result, 28);
  weaponParams.fireAnimation = std::nullopt;
  if(fireAnimation != ""){
    weaponParams.fireAnimation = fireAnimation;
  }

  auto idleAnimation = strFromFirstSqlResult(result, 29);
  weaponParams.idleAnimation = std::nullopt;
  if (idleAnimation != ""){
    weaponParams.idleAnimation = idleAnimation;;
  }
  

  auto gunpos = vec3FromFirstSqlResult(result, 3, 4, 5);
  weaponParams.initialGunPos = gunpos;

  auto rot3 = vec3FromFirstSqlResult(result, 6, 7, 8);
  auto rot4 = glm::vec4(rot3.x, rot3.y, rot3.z, 0.f);
  weaponParams.initialGunRotVec4 = rot4;
  weaponParams.initialGunRot = parseQuat(rot4);
  weaponParams.scale = vec3FromFirstSqlResult(result, 9, 10, 11);

  weaponParams.ironSightAngle = result.at(0).at(32) == "" ? weaponParams.initialGunRot : quatFromFirstSqlResult(result, 32);

  weaponParams.soundpath = strFromFirstSqlResult(result, 2);
  weaponParams.modelpath = strFromFirstSqlResult(result, 0);
  weaponParams.script = strFromFirstSqlResult(result, 27);

  weaponParams.muzzleParticleStr = strFromFirstSqlResult(result, 19);
  weaponParams.hitParticleStr = strFromFirstSqlResult(result, 20);
  weaponParams.projectileParticleStr = strFromFirstSqlResult(result, 25);


  return weaponParams;
}

objid createWeaponInstance(WeaponParams& weaponParams, objid sceneId, objid parentId, std::string& weaponName, std::function<objid(objid)> getWeaponParentId){
  std::map<std::string, AttributeValue> attrAttributes = { 
    { "mesh", weaponParams.modelpath }, 
    { "layer", "no_depth" },
    { "rotation", weaponParams.initialGunRotVec4 },
    { "position", weaponParams.initialGunPos - glm::vec3(0.f, 0.f, 0.f) },
    { "scale", weaponParams.scale },
    { "tint", glm::vec4(1.f, 1.f, 1.f, 0.4f) },
  };
  if (weaponParams.script != ""){
    attrAttributes["script"] = weaponParams.script;
  }
  GameobjAttributes attr {
    .attr = attrAttributes,
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto gunId = gameapi -> makeObjectAttr(sceneId, weaponName, attr, submodelAttributes);
  modassert(gunId.has_value(), std::string("weapons could not spawn gun: ") + weaponParams.name);
  gameapi -> makeParent(gunId.value(), getWeaponParentId(parentId));
  return gunId.value();
}

void saveGunTransform(GunInstance& weaponValues){
  debugAssertForNow(false, "bad code - cannot get raw position / etc since ironsights mean this needs to subtract by initial offset");

  if (weaponValues.gunId.has_value()){
    auto gunId = weaponValues.gunId.value();
    auto gun = weaponValues.gunCore.weaponCore -> weaponParams.name;

    auto attrHandle = getAttrHandle(gunId);
    auto position = getVec3Attr(attrHandle, "position").value();  
    auto scale = getVec3Attr(attrHandle, "scale").value();
    auto rotation = getVec4Attr(attrHandle, "rotation").value();

    modlog("weapons", "save gun, name = " + weaponValues.gunCore.weaponCore -> weaponParams.name + ",  pos = " + print(position) + ", scale = " + print(scale) + ", rotation = " + print(rotation));

    auto updateQuery = gameapi -> compileSqlQuery(
      std::string("update guns set ") +
        "xoffset-pos = " + serializeFloat(position.x) + ", " +
        "yoffset-pos = " + serializeFloat(position.y) + ", " +
        "zoffset-pos = " + serializeFloat(position.z) + ", " + 
        "xrot = " + serializeFloat(rotation.x) + ", " +
        "yrot = " + serializeFloat(rotation.y) + ", " +
        "zrot = " + serializeFloat(rotation.z) + 
        " where name = " + gun,
        {}
    );
    bool validSql = false;
    auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
    modassert(validSql, "error executing sql query");
  }
}

GunCore createGunCoreInstance(std::string gun, objid sceneId){
  modlog("weapons", std::string("spawn gun: ") + gun);
  auto weaponParams = queryWeaponParams(gun);

  loadWeaponCore(gun, sceneId, weaponParams);

  auto weaponCore = findWeaponCore(gun);
  modassert(weaponCore, std::string("could not find gun core for: ") + gun);
  WeaponState newState {
    .lastShootingTime = -1.f * weaponCore -> weaponParams.firingRate, // so you can shoot immediately
    .recoilStart = 0.f,
    .gunState = GUN_RAISED,
  };

  GunCore gunCore {
    .weaponCore = weaponCore,
    .weaponState = newState,
  };
  return gunCore;
}

std::optional<std::string*> getCurrentGunName(GunInstance& weaponValues){
  if (!weaponValues.gunCore.weaponCore){
    return std::nullopt;
  }
  return &weaponValues.gunCore.weaponCore -> weaponParams.name;
}

void ensureGunInstance(GunInstance& _gunInstance, objid parentId, bool createGunModel, std::function<objid(objid)> getWeaponParentId){
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
  if (sameGun && _gunInstance.gunId.has_value() && createGunModel){
    return;
  }
  modlog("weapons ensureGunInstance", std::string("change gun instance: ") + _gunInstance.desiredGun);

  if (_gunInstance.gunId.has_value()){
    gameapi -> removeByGroupId(_gunInstance.gunId.value());
  }
  _gunInstance.gunId = std::nullopt;
  _gunInstance.muzzleId = std::nullopt;

  if (sameGun && !createGunModel){
    return;
  }

  auto gunCore = createGunCoreInstance(_gunInstance.desiredGun, 0); // would be better to preload all gun cores
  std::optional<objid> gunId;
  std::optional<objid> muzzlePointId;

  if (createGunModel){
    auto sceneId = gameapi -> listSceneId(parentId);
    auto weaponName = std::string("code-weapon-") + uniqueNameSuffix();
    gunId = createWeaponInstance(gunCore.weaponCore -> weaponParams, sceneId, parentId, weaponName, getWeaponParentId);
    if (gunCore.weaponCore -> weaponParams.idleAnimation.has_value() && gunCore.weaponCore -> weaponParams.idleAnimation.value() != "" && gunId){
      gameapi -> playAnimation(gunId.value(), gunCore.weaponCore -> weaponParams.idleAnimation.value(), LOOP);
    }
    muzzlePointId = gameapi -> getGameObjectByName(weaponName + "/muzzle", sceneId, true);
    if (!muzzlePointId.has_value()){
      modlog("weapon core", std::string("no muzzle defined for: ") + _gunInstance.desiredGun);
    }    
  }

  _gunInstance.gunCore = gunCore;
  _gunInstance.gunId = gunId;
  _gunInstance.muzzleId = muzzlePointId;
}

void changeGunAnimate(GunInstance& weaponValues, std::string gun, objid sceneId, objid playerId, bool createGunModel){
  if (weaponValues.gunCore.weaponCore != NULL && weaponValues.gunCore.weaponCore -> weaponParams.name == gun){
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
    weaponValues.gunCore.weaponCore = NULL;
  }
}

void deliverAmmo(objid inventory, std::string gunName, int ammo){
  auto oldAmmo = ammoForGun(inventory, gunName);
  setGunAmmo(inventory, gunName, oldAmmo + ammo);
}

bool canFireGunNow(GunCore& gunCore, float elapsedMilliseconds){
  auto timeSinceLastShot = elapsedMilliseconds - gunCore.weaponState.lastShootingTime;
  bool lessThanFiringRate = timeSinceLastShot >= (0.001f * gunCore.weaponCore -> weaponParams.firingRate);
  return lessThanFiringRate;
}

// fires from point of view of the camera
float maxRaycastDistance = 500.f;
std::vector<HitObject> doRaycast(glm::vec3 orientationOffset, glm::vec3 mainobjPos, glm::quat mainobjRotation){
  auto orientationOffsetQuat = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), orientationOffset);
  auto rot = mainobjRotation *  orientationOffsetQuat;
  auto hitpoints =  gameapi -> raycast(mainobjPos, rot, maxRaycastDistance);
  return hitpoints;
}

std::vector<HitObject> doRaycastClosest(glm::vec3 cameraPos, glm::vec3 orientationOffset, glm::quat mainobjRotation){
  auto hitpoints = doRaycast(orientationOffset, cameraPos, mainobjRotation);
  if (hitpoints.size() > 0){
    auto closestIndex = closestHitpoint(hitpoints, cameraPos);
    return { hitpoints.at(closestIndex) };
  }
  return hitpoints;
}

std::vector<HitObject> doRaycastClosest(glm::vec3 orientationOffset, objid playerId){
  auto mainobjPos = gameapi -> getGameObjectPos(playerId, true);
  auto mainobjRotation = gameapi -> getGameObjectRotation(playerId, true);

  return doRaycastClosest(mainobjPos, orientationOffset, mainobjRotation); 
}

glm::vec3 zFightingForParticle(glm::vec3 pos, glm::quat normal){
  return gameapi -> moveRelative(pos, normal, 0.01);  // 0.01?
}

void fireRaycast(GunCore& gunCore, glm::vec3 orientationOffset, objid playerId, std::vector<MaterialToParticle>& materials, glm::vec3 cameraPos){
  auto hitpoints = doRaycastClosest(orientationOffset, playerId);
  modlog("weapons", "fire raycast, total hits = " + std::to_string(hitpoints.size()));

  for (auto &hitpoint : hitpoints){
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

    if (gunCore.weaponCore -> hitParticles.has_value()){
      emitterId = gunCore.weaponCore -> hitParticles.value();
    }

    auto addedGlassDecal = maybeAddGlassBulletHole(hitpoint.id, playerId);
    auto emitParticlePosition = zFightingForParticle(hitpoint.point, hitpoint.normal);
    if (!addedGlassDecal){
      if (emitterId.has_value()){
        std::cout << "hit particle, hitpoint.id = " << hitpoint.id << std::endl;
        gameapi -> emit(emitterId.value(), emitParticlePosition, hitpoint.normal, std::nullopt, std::nullopt, hitpoint.id);
      }      
    }

    if (soundEmitterId.has_value()){
      playGameplayClipById(soundEmitterId.value(), std::nullopt, hitpoint.point);
    }


    if (splashEmitterId.has_value()){
      gameapi -> emit(splashEmitterId.value(), emitParticlePosition, hitpoint.normal, std::nullopt, std::nullopt, std::nullopt);
    }

    doDamageMessage(hitpoint.id, gunCore.weaponCore -> weaponParams.damage);
    modlog("weapons", "raycast normal: " + serializeQuat(hitpoint.normal));
  }
}

bool tryFireGun(objid inventory, std::optional<objid> gunId, std::optional<objid> muzzleId, GunCore& gunCore, float bloomAmount, objid playerId, glm::vec3 playerPos, glm::quat playerRotation, std::vector<MaterialToParticle>& materials){  
  float now = gameapi -> timeSeconds(false);
  auto canFireGun = canFireGunNow(gunCore, now);
  modlog("weapons", std::string("try fire gun, can fire = ") + (canFireGun ? "true" : "false") + ", now = " + std::to_string(now) + ", firing rate = " + std::to_string(gunCore.weaponCore -> weaponParams.firingRate));
  if (!canFireGun){
    return false;
  }
  bool hasAmmo = ammoForGun(inventory, gunCore.weaponCore -> name) > 0;
  if (!hasAmmo){
    modlog("weapons", "no ammo, tried to fire, should play sound");
    return false;
  }

  if (gunCore.weaponCore != NULL){
    deliverAmmo(inventory, gunCore.weaponCore -> weaponParams.name, -1);
  }

  if (gunCore.weaponCore -> soundResource.has_value()){
    playGameplayClipById(gunCore.weaponCore -> soundResource.value().clipObjectId, std::nullopt, playerPos);
  }

  if (gunCore.weaponCore -> muzzleParticle.has_value() && gunId.has_value()){
    if (muzzleId.has_value()){
      auto muzzlePosition = gameapi -> getGameObjectPos(muzzleId.value(), true);
      std::cout << "muzzle emit: " << print(muzzlePosition) << std::endl;
      gameapi -> emit(gunCore.weaponCore -> muzzleParticle.value(), muzzlePosition, playerRotation /* should this be the muzzle rotation? */, std::nullopt, std::nullopt, playerId);
    }else{
      auto gunPosition = gameapi -> getGameObjectPos(gunId.value(), true);
      glm::vec3 distanceFromGun = glm::vec3(0.f, 0.f, -1.); // should parameterize particleOffset
      auto slightlyInFrontOfGun = gameapi -> moveRelativeVec(gunPosition, playerRotation, distanceFromGun);
      gameapi -> emit(gunCore.weaponCore -> muzzleParticle.value(), slightlyInFrontOfGun, playerRotation /* should this be the gun rotation? */, std::nullopt, std::nullopt, playerId);
    }
  }
  gunCore.weaponState.lastShootingTime = now;
  gunCore.weaponState.recoilStart = gameapi -> timeSeconds(false);

  glm::vec3 shootingVecAngle(randomNumber(-bloomAmount, bloomAmount), randomNumber(-bloomAmount, bloomAmount), -1.f);
  if (gunCore.weaponCore -> weaponParams.isRaycast){
    fireRaycast(gunCore, shootingVecAngle, playerId, materials, gameapi -> getGameObjectPos(playerId, true));
  }
  if (gunCore.weaponCore -> projectileParticles.has_value()){
    auto fromPos = gameapi -> moveRelative(playerPos, playerRotation, 3);
    glm::vec3 projectileArc(0.f, 0.f, -1.f);
    auto playerForwardAndUp = playerRotation * gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), projectileArc);
    auto initialVelocity = playerForwardAndUp * shootingVecAngle * 10.f;
    gameapi -> emit(gunCore.weaponCore -> projectileParticles.value(), fromPos, playerRotation, initialVelocity, std::nullopt, std::nullopt);
  }

  if (gunId.has_value() && gunCore.weaponCore -> weaponParams.fireAnimation.has_value()){
     gameapi -> playAnimation(gunId.value(), gunCore.weaponCore -> weaponParams.fireAnimation.value(), ONESHOT);
  }
  ////

  return true;
}

float calcRecoilSlerpAmount(GunCore& gunCore, float length,  bool reset){
  float amount = (gameapi -> timeSeconds(false) - gunCore.weaponState.recoilStart) / length;
  return (amount > 1.f) ? (reset ? 0.f : 1.f): amount;
}

float calculateBloomAmount(GunCore& gunCore){
  auto slerpAmount = (1 - calcRecoilSlerpAmount(gunCore, gunCore.weaponCore -> weaponParams.bloomLength, false)); 
  modassert(slerpAmount <= 1, "slerp amount must be less than 1, got: " + std::to_string(slerpAmount));
  return glm::max(gunCore.weaponCore -> weaponParams.minBloom, (gunCore.weaponCore -> weaponParams.totalBloom - gunCore.weaponCore -> weaponParams.minBloom) * slerpAmount + gunCore.weaponCore -> weaponParams.minBloom);
}

GunFireInfo fireGunAndVisualize(GunCore& gunCore, bool holding, bool fireOnce, std::optional<objid> gunId, std::optional<objid> muzzleId, objid id, objid inventory, FiringTransform& transform){
  if (!gunCore.weaponCore){
    modlog("fire gun", "no weaponCore");
    return GunFireInfo { .didFire = false, .bloomAmount = std::nullopt };
  }
  auto bloomAmount = calculateBloomAmount(gunCore);
  if ((gunCore.weaponCore -> weaponParams.canHold && holding) || fireOnce){
    bool didFire = tryFireGun(inventory, gunId, muzzleId, gunCore, bloomAmount, id, transform.position, transform.rotation, getMaterials());
    return GunFireInfo { .didFire = didFire, .bloomAmount = bloomAmount };
  }
  return GunFireInfo { .didFire = false, .bloomAmount = bloomAmount };
}

bool swayFromMouse = true;
glm::vec3 getSwayVelocity(objid playerId, glm::vec2 lookVelocity, glm::vec3 movementVec){
  if (swayFromMouse){
    debugAssertForNow(false, "sway from mouse should take into account sensitivity");
    return glm::vec3(lookVelocity.x, lookVelocity.y, 0.f);
  }
  auto parentRot = gameapi -> getGameObjectRotation(playerId, false);
  auto newPos = glm::inverse(parentRot) * movementVec;
  return newPos;
}


glm::vec3 calcLocationWithRecoil(GunInstance& weaponValues, glm::vec3 pos, bool isGunZoomed){
  auto targetPos = isGunZoomed ? weaponValues.gunCore.weaponCore -> weaponParams.ironsightOffset : pos;
  auto recoilAmount = isGunZoomed ? weaponValues.gunCore.weaponCore -> weaponParams.recoilZoomTranslate : weaponValues.gunCore.weaponCore -> weaponParams.recoilTranslate;
  auto targetPosWithRecoil = glm::vec3(targetPos.x + recoilAmount.x, targetPos.y + recoilAmount.y, targetPos.z + recoilAmount.z);
  return glm::lerp(targetPos, targetPosWithRecoil, calcRecoilSlerpAmount(weaponValues.gunCore, weaponValues.gunCore.weaponCore -> weaponParams.recoilLength, true));
}

glm::vec3 maxMagSway(0.1f, 0.1f, 0.05f);
float zoomSpeedMultiplier = 5.f;
float swayVelocity = 1.;
void swayGunTranslation(GunInstance& weaponValues, glm::vec3 relVelocity, bool isGunZoomed){
  float swayAmountX =  relVelocity.x;
  float limitedSwayX = glm::min(maxMagSway.x, glm::max(swayAmountX, -1.f * maxMagSway.x));

  float swayAmountY =  relVelocity.y;
  float limitedSwayY = glm::min(maxMagSway.y, glm::max(swayAmountY, -1.f * maxMagSway.y));

  float swayAmountZ =  relVelocity.z;
  float limitedSwayZ = glm::min(maxMagSway.z, glm::max(swayAmountZ, -1.f * maxMagSway.z));  

  glm::vec3 targetPos(-1.f * limitedSwayX + weaponValues.gunCore.weaponCore -> weaponParams.initialGunPos.x, -1.f * limitedSwayY + weaponValues.gunCore.weaponCore -> weaponParams.initialGunPos.y, -1.f * limitedSwayZ + weaponValues.gunCore.weaponCore -> weaponParams.initialGunPos.z); 
  
  auto targetPosWithRecoil = calcLocationWithRecoil(weaponValues, targetPos, isGunZoomed); // this should use ironsight-offset

  if (weaponValues.gunCore.weaponState.gunState == GUN_LOWERING){
    targetPosWithRecoil += glm::vec3(0.f, -1.f, 0.f);
  }

  auto gunId = weaponValues.gunId.value();
  auto animationRate = weaponValues.gunCore.weaponState.gunState == GUN_LOWERING ? 5.f : 3.f;
  float lerpAmount = gameapi -> timeElapsed() * swayVelocity * (isGunZoomed ? zoomSpeedMultiplier : 1.f) * animationRate;
  auto newPos = glm::lerp(gameapi -> getGameObjectPos(gunId, false), targetPosWithRecoil, lerpAmount);  // probably pick a better function?  how does it feel tho
  //std::cout << "gun: targetpos: " << print(targetPosWithRecoil) << std::endl;
  //std::cout << "gun: newpos: " << print(newPos) << std::endl;
  gameapi -> setGameObjectPosition(gunId, newPos, false);
}

void swayGunRotation(GunInstance& weaponValues, bool isGunZoomed, glm::vec2 lookVelocity, glm::vec3 movementVec){
  float swayAmountX = lookVelocity.x;
  float limitedSwayX = glm::min(maxMagSway.x, glm::max(swayAmountX, -1.f * maxMagSway.x));

  float swayAmountY = lookVelocity.y;
  float limitedSwayY = glm::min(maxMagSway.y, glm::max(swayAmountY, -1.f * maxMagSway.y));
  float recoilAmount =  glm::lerp(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, weaponValues.gunCore.weaponCore -> weaponParams.recoilPitchRadians, 0.f), calcRecoilSlerpAmount(weaponValues.gunCore, weaponValues.gunCore.weaponCore -> weaponParams.recoilLength, true)).y;

  float totalSwayY = limitedSwayY + recoilAmount * 5.f;
  //std::cout << "limited sway x: " << limitedSwayX << std::endl;
  //std::cout << "limited recoil: " << recoilAmount << std::endl;

  auto oldRotation = gameapi -> getGameObjectRotation(weaponValues.gunId.value(), false);
  auto rotation = gameapi -> setFrontDelta(
    parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)), 
    limitedSwayX /* delta yaw */, 
    totalSwayY /* should add recoil here */, 
    0.f, 
    0.1f /* why 0.1f? */
  );
  auto targetRotation = rotation * (isGunZoomed ? weaponValues.gunCore.weaponCore -> weaponParams.ironSightAngle : weaponValues.gunCore.weaponCore -> weaponParams.initialGunRot);
  gameapi -> setGameObjectRot(
    weaponValues.gunId.value(), 
    glm::slerp(oldRotation, targetRotation, 0.1f),
    false
  );
}

bool swayRotation = true;
void swayGun(GunInstance& weaponValues, bool isGunZoomed, objid playerId, glm::vec2 lookVelocity, glm::vec3 movementVec){
  if (!weaponValues.gunId.has_value()){
    return;
  }

  bool shouldZoomGun = isGunZoomed && weaponValues.gunCore.weaponCore -> weaponParams.isIronsight;
  //modlog("weapon", "movement velocity: " + std::to_string(weapons.movementVelocity));
  //modlog("weapon", "sway velocity: " + print(swayVelocity));
  auto swayVelocity = getSwayVelocity(playerId, lookVelocity, movementVec);
  swayGunTranslation(weaponValues, swayVelocity, shouldZoomGun);
  if (swayRotation){
    swayGunRotation(weaponValues, shouldZoomGun, lookVelocity, movementVec);
  }
}

