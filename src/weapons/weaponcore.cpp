#include "./weaponcore.h"

extern CustomApiBindings* gameapi;

struct WeaponCore {

};

std::vector<WeaponCore> weaponCores = {

};

WeaponParams queryWeaponParams(std::string gunName){
  auto gunQuery = gameapi -> compileSqlQuery(
    std::string("select modelpath, fireanimation, fire-sound, xoffset-pos, ") +
    "yoffset-pos, zoffset-pos, xrot, yrot, zrot, xscale, yscale, zscale, " + 
    "firing-rate, hold, raycast, ironsight, iron-xoffset-pos, iron-yoffset-pos, " + 
    "iron-zoffset-pos, particle, hit-particle, recoil-length, recoil-angle, " + 
    "recoil, recoil-zoom, projectile, bloom, script, fireanimation, idleanimation, bloom-length, minbloom, ironsight-rot, ammo " + 
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

GunCore getGunCoreType(std::string gun, int ammo){
  modassert(false, "not yet implemented");
  return GunCore { };
}


WeaponInstance createWeaponInstance(WeaponParams& weaponParams, objid sceneId, objid playerId){
  WeaponInstance weaponInstance {};
  {
    std::map<std::string, std::string> stringAttributes = { { "mesh", weaponParams.modelpath }, { "layer", "no_depth" } };
    if (weaponParams.script != ""){
      stringAttributes["script"] = weaponParams.script;
    }
    GameobjAttributes attr {
      .stringAttributes = stringAttributes,
      .numAttributes = {},
      .vecAttr = {  .vec3 = {{ "position", weaponParams.initialGunPos - glm::vec3(0.f, 0.f, 0.f) }, { "scale", weaponParams.scale }},  .vec4 = {{ "rotation", weaponParams.initialGunRotVec4 }}},
    };
    std::map<std::string, GameobjAttributes> submodelAttributes;
    auto gunId = gameapi -> makeObjectAttr(sceneId, std::string("code-weapon-") + uniqueNameSuffix(), attr, submodelAttributes);
    modassert(gunId.has_value(), std::string("weapons could not spawn gun: ") + weaponParams.name);
    weaponInstance.gunId = gunId.value();
  }

  {
    if (weaponParams.soundpath != ""){
      GameobjAttributes soundAttr {
        .stringAttributes = { { "clip", weaponParams.soundpath }, { "physics", "disabled" }},
        .numAttributes = {},
        .vecAttr = {  .vec3 = {},  .vec4 = {} },
      };
      std::string soundClipObj = std::string("&code-weaponsound-") + uniqueNameSuffix();
      std::map<std::string, GameobjAttributes> submodelAttributes;
      auto soundId = gameapi -> makeObjectAttr(sceneId, soundClipObj, soundAttr, submodelAttributes);
      weaponInstance.soundId = soundId;  
      weaponInstance.soundClipObj = soundClipObj;
    }
  }

  {
    weaponInstance.muzzleParticle = createParticleEmitter(sceneId, weaponParams.muzzleParticleStr, "+code-muzzleparticles");
    weaponInstance.hitParticles = createParticleEmitter(sceneId, weaponParams.hitParticleStr, "+code-hitparticle");
    weaponInstance.projectileParticles = createParticleEmitter(sceneId, weaponParams.projectileParticleStr, "+code-projectileparticle");
  }
  
  {
    gameapi -> makeParent(weaponInstance.gunId, playerId);
    if (weaponInstance.soundId.has_value()){
      gameapi -> makeParent(weaponInstance.soundId.value(), playerId);
    }
    if (weaponInstance.muzzleParticle.has_value()){
      gameapi -> makeParent(weaponInstance.muzzleParticle.value(), weaponInstance.gunId);
    }
    if (weaponInstance.projectileParticles.has_value()){
      gameapi -> makeParent(weaponInstance.projectileParticles.value(), weaponInstance.gunId);
    }
  }

  return weaponInstance;
}

void removeWeaponInstance(WeaponInstance& weaponInstance){
  gameapi -> removeObjectById(weaponInstance.gunId);

  gameapi -> removeObjectById(weaponInstance.soundId.value());
  weaponInstance.soundId = std::nullopt;
  weaponInstance.soundClipObj = std::nullopt;

  if (weaponInstance.muzzleParticle.has_value()){
     gameapi -> removeObjectById(weaponInstance.muzzleParticle.value());
     weaponInstance.muzzleParticle = std::nullopt;
  }

  if (weaponInstance.hitParticles.has_value()){
    gameapi -> removeObjectById(weaponInstance.hitParticles.value());
    weaponInstance.hitParticles = std::nullopt;
  }

  if (weaponInstance.projectileParticles.has_value()){
    gameapi -> removeObjectById(weaponInstance.projectileParticles.value());
    weaponInstance.projectileParticles = std::nullopt;
  }
}

void saveGunTransform(WeaponValues& weaponValues){
  debugAssertForNow(false, "bad code - cannot get raw position / etc since ironsights mean this needs to subtract by initial offset");

  if (weaponValues.weaponInstance.has_value()){
    auto gunId = weaponValues.weaponInstance.value().gunId;
    auto gun = weaponValues.gunCore.weaponParams.name;
    auto attr = gameapi -> getGameObjectAttr(gunId);
    auto position = attr.vecAttr.vec3.at("position");  
    auto scale = attr.vecAttr.vec3.at("scale");
    auto rotation = attr.vecAttr.vec4.at("rotation");

    modlog("weapons", "save gun, name = " + weaponValues.gunCore.weaponParams.name + ",  pos = " + print(position) + ", scale = " + print(scale) + ", rotation = " + print(rotation));

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

void changeGun(WeaponValues& weaponValues, std::string gun, int ammo, objid sceneId, objid playerId){
  weaponValues.gunCore.weaponParams = queryWeaponParams(gun);
  modlog("weapons", std::string("spawn gun: ") + weaponValues.gunCore.weaponParams.name);

  WeaponState newState {
    .lastShootingTime = -1.f * weaponValues.gunCore.weaponParams.firingRate, // so you can shoot immediately
    .recoilStart = 0.f,
    .currentAmmo = ammo,
    .gunState = GUN_RAISED,
  };

  weaponValues.gunCore.weaponState = newState;

  if (weaponValues.weaponInstance.has_value()){
    removeWeaponInstance(weaponValues.weaponInstance.value());
  }
  weaponValues.weaponInstance = createWeaponInstance(weaponValues.gunCore.weaponParams, sceneId, playerId);
  if (weaponValues.gunCore.weaponParams.idleAnimation.has_value() && weaponValues.gunCore.weaponParams.idleAnimation.value() != "" && weaponValues.weaponInstance.value().gunId){
    gameapi -> playAnimation(weaponValues.weaponInstance.value().gunId, weaponValues.gunCore.weaponParams.idleAnimation.value(), LOOP);
  }
  gameapi -> sendNotifyMessage("current-gun", CurrentGunMessage {
    .currentAmmo = weaponValues.gunCore.weaponState.currentAmmo,
    .totalAmmo = weaponValues.gunCore.weaponParams.totalAmmo,
  });
}
void changeGunAnimate(WeaponValues& weaponValues, std::string gun, int ammo, objid sceneId, objid playerId){
  if (gun == weaponValues.gunCore.weaponParams.name){
    return;
  }
  weaponValues.gunCore.weaponState.gunState = GUN_LOWERING;
  gameapi -> schedule(playerId, 500, NULL, [&weaponValues, gun, ammo, sceneId, playerId](void* weaponData) -> void {
    gameapi -> sendNotifyMessage("set-gun-ammo", SetAmmoMessage {
      .currentAmmo = ammo,
      .gun = weaponValues.gunCore.weaponParams.name,
    });
    changeGun(weaponValues, gun, ammo, sceneId, playerId);
  });
}

void deliverAmmo(GunCore& _gunCore, int ammo){
  _gunCore.weaponState.currentAmmo += ammo;
  gameapi -> sendNotifyMessage("current-gun", CurrentGunMessage {
    .currentAmmo = _gunCore.weaponState.currentAmmo,
    .totalAmmo = _gunCore.weaponParams.totalAmmo,
  });
}

bool canFireGunNow(GunCore& gunCore, float elapsedMilliseconds){
  auto timeSinceLastShot = elapsedMilliseconds - gunCore.weaponState.lastShootingTime;
  bool lessThanFiringRate = timeSinceLastShot >= (0.001f * gunCore.weaponParams.firingRate);
  return lessThanFiringRate;
}

// fires from point of view of the camera
float maxRaycastDistance = 500.f;
std::vector<HitObject> doRaycast(glm::vec3 orientationOffset, objid playerId){
  auto orientationOffsetQuat = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), orientationOffset);
  auto mainobjPos = gameapi -> getGameObjectPos(playerId, true);
  auto rot = gameapi -> getGameObjectRotation(playerId, true) *  orientationOffsetQuat;
  //  (define shotangle (if (should-zoom) rot (with-bloom rot)))
  auto hitpoints =  gameapi -> raycast(mainobjPos, rot, maxRaycastDistance);

  //if (weapons.raycastLine.has_value()){
  //  gameapi -> freeLine(weapons.raycastLine.value());
  //}
  
  //auto raycastLineId = gameapi -> drawLine(mainobjPos, gameapi -> moveRelative(mainobjPos, rot, maxRaycastDistance), true, playerId.value(), std::nullopt,  std::nullopt, std::nullopt);
  //weapons.raycastLine = raycastLineId;
  //for (auto &hitpoint : hitpoints){
  //  std::cout << "raycast hit: " << hitpoint.id << "- point: " << print(hitpoint.point) << ", normal: " << print(hitpoint.normal) << std::endl;
  //  showDebugHitmark(hitpoint, weapons.playerId);
  //}
  return hitpoints;
}

std::vector<HitObject> doRaycastClosest(glm::vec3 cameraPos, glm::vec3 orientationOffset, objid playerId){
  auto hitpoints = doRaycast(orientationOffset, playerId);
  if (hitpoints.size() > 0){
    auto closestIndex = closestHitpoint(hitpoints, cameraPos);
    return { hitpoints.at(closestIndex) };
  }
  return hitpoints;
}

glm::vec3 zFightingForParticle(glm::vec3 pos, glm::quat normal){
  return gameapi -> moveRelative(pos, normal, 0.01);  // 0.01?
}

void fireRaycast(WeaponValues& weaponValues, glm::vec3 orientationOffset, objid playerId, std::vector<MaterialToParticle>& materials){
  auto cameraPos = gameapi -> getGameObjectPos(playerId, true);
  auto hitpoints = doRaycastClosest(cameraPos, orientationOffset, playerId);
  modlog("weapons", "fire raycast, total hits = " + std::to_string(hitpoints.size()));

  for (auto &hitpoint : hitpoints){
    modlog("weapons", "raycast hit: " + std::to_string(hitpoint.id) + "- point: " + print(hitpoint.point) + ", normal: " + print(hitpoint.normal));
    auto objMaterial = materialTypeForObj(hitpoint.id);
    if (!objMaterial.has_value()){
      objMaterial = "default";
    }

    std::optional<objid> emitterId = std::nullopt;
    std::optional<objid> splashEmitterId = std::nullopt;

    if (objMaterial.has_value()){
      auto material = getHitMaterial(materials, objMaterial.value());
      std::cout << "hit particle material: (" << (material.has_value() && material.value() -> hitParticle.has_value() ? "has hit particle" : "no hit particle" ) << ") " << std::endl;
      if (material.has_value() && material.value() -> hitParticle.has_value()){
        emitterId = material.value() -> hitParticle.value().particleId;
      }
      if (material.has_value() && material.value() -> splashParticle.has_value()){
        splashEmitterId = material.value() -> splashParticle.value().particleId;
      }
    }

    if (weaponValues.weaponInstance.value().hitParticles.has_value()){
      emitterId = weaponValues.weaponInstance.value().hitParticles.value();
    }

    auto emitParticlePosition = zFightingForParticle(hitpoint.point, hitpoint.normal);
    if (emitterId.has_value()){
      std::cout << "hit particle, hitpoint.id = " << hitpoint.id << std::endl;
      gameapi -> emit(emitterId.value(), emitParticlePosition, hitpoint.normal, std::nullopt, std::nullopt, hitpoint.id);
    }
    if (splashEmitterId.has_value()){
      gameapi -> emit(splashEmitterId.value(), emitParticlePosition, hitpoint.normal, std::nullopt, std::nullopt, std::nullopt);
    }

    DamageMessage damageMessage {
      .id = hitpoint.id,
      .amount = 50.f,
    };
    gameapi -> sendNotifyMessage("damage", damageMessage);
    modlog("weapons", "raycast normal: " + serializeQuat(hitpoint.normal));
  }
}

bool tryFireGun(WeaponValues& weaponValues, objid sceneId, float bloomAmount, objid playerId, std::vector<MaterialToParticle>& materials){
  //// should come from outside this since gun core can't know this 
  auto playerRotation = gameapi -> getGameObjectRotation(playerId, true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
  auto playerPos = gameapi -> getGameObjectPos(playerId, true);
  //////////////////////
  
  float now = gameapi -> timeSeconds(false);
  auto canFireGun = canFireGunNow(weaponValues.gunCore, now);
  modlog("weapons", std::string("try fire gun, can fire = ") + (canFireGun ? "true" : "false") + ", now = " + std::to_string(now) + ", firing rate = " + std::to_string(weaponValues.gunCore.weaponParams.firingRate));
  if (!canFireGun){
    return false;
  }
  bool hasAmmo = weaponValues.gunCore.weaponState.currentAmmo > 0;
  if (!hasAmmo){
    modlog("weapons", "no ammo, tried to fire, should play sound");
    return false;
  }
  deliverAmmo(weaponValues.gunCore, -1);
  gameapi -> sendNotifyMessage("current-gun", CurrentGunMessage {
    .currentAmmo = weaponValues.gunCore.weaponState.currentAmmo,
    .totalAmmo = weaponValues.gunCore.weaponParams.totalAmmo,
  });

  if (weaponValues.weaponInstance.value().soundId.has_value()){
    gameapi -> playClip(weaponValues.weaponInstance.value().soundClipObj.value(), sceneId, std::nullopt, std::nullopt);
  }

  if (weaponValues.weaponInstance.value().muzzleParticle.has_value()){
    auto gunPosition = gameapi -> getGameObjectPos(weaponValues.weaponInstance.value().gunId, true);
    auto playerRotation = gameapi -> getGameObjectRotation(playerId, true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
    glm::vec3 distanceFromGun = glm::vec3(0.f, 0.f, -1.); // should parameterize particleOffset
    auto slightlyInFrontOfGun = gameapi -> moveRelativeVec(gunPosition, playerRotation, distanceFromGun);
    gameapi -> emit(weaponValues.weaponInstance.value().muzzleParticle.value(), slightlyInFrontOfGun, playerRotation, std::nullopt, std::nullopt, playerId);
  }
  weaponValues.gunCore.weaponState.lastShootingTime = now;
  weaponValues.gunCore.weaponState.recoilStart = gameapi -> timeSeconds(false);

  glm::vec3 shootingVecAngle(randomNumber(-bloomAmount, bloomAmount), randomNumber(-bloomAmount, bloomAmount), -1.f);
  if (weaponValues.gunCore.weaponParams.isRaycast){
    fireRaycast(weaponValues, shootingVecAngle, playerId, materials);
  }
  if (weaponValues.weaponInstance.value().projectileParticles.has_value()){
    auto fromPos = gameapi -> moveRelative(playerPos, playerRotation, 3);
    glm::vec3 projectileArc(0.f, 0.f, -1.f);
    auto playerForwardAndUp = playerRotation * gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), projectileArc);
    auto initialVelocity = playerForwardAndUp * shootingVecAngle * 10.f;
    gameapi -> emit(weaponValues.weaponInstance.value().projectileParticles.value(), fromPos, playerRotation, initialVelocity, std::nullopt, std::nullopt);
  }

  /////////////// not gun core
  if (weaponValues.gunCore.weaponParams.fireAnimation.has_value()){
    gameapi -> playAnimation(weaponValues.weaponInstance.value().gunId, weaponValues.gunCore.weaponParams.fireAnimation.value(), ONESHOT);
  }
  ////

  return true;
}

float calcRecoilSlerpAmount(GunCore& gunCore, float length,  bool reset){
  float amount = (gameapi -> timeSeconds(false) - gunCore.weaponState.recoilStart) / length;
  return (amount > 1.f) ? (reset ? 0.f : 1.f): amount;
}

float calculateBloomAmount(GunCore& gunCore){
  auto slerpAmount = (1 - calcRecoilSlerpAmount(gunCore, gunCore.weaponParams.bloomLength, false)); 
  modassert(slerpAmount <= 1, "slerp amount must be less than 1, got: " + std::to_string(slerpAmount));
  return glm::max(gunCore.weaponParams.minBloom, (gunCore.weaponParams.totalBloom - gunCore.weaponParams.minBloom) * slerpAmount + gunCore.weaponParams.minBloom);
}
bool fireGunAndVisualize(WeaponValues& weaponValues, objid id, objid playerId, std::vector<MaterialToParticle>& materials, bool holding, bool fireOnce){
  auto bloomAmount = calculateBloomAmount(weaponValues.gunCore);
  drawBloom(playerId, id, -1.f, bloomAmount); // 0.002f is just a min amount for visualization, not actual bloom
  if ((weaponValues.gunCore.weaponParams.canHold && holding) || fireOnce){
    return tryFireGun(weaponValues, gameapi -> listSceneId(id), bloomAmount, playerId, materials);
  }
  return false;
}

bool swayFromMouse = false;
glm::vec3 getSwayVelocity(objid playerId, glm::vec2 lookVelocity, glm::vec3 movementVec){
  if (swayFromMouse){
    debugAssertForNow(false, "sway from mouse should take into account sensitivity");
    return glm::vec3(lookVelocity.x, lookVelocity.y, 0.f);
  }
  auto parentRot = gameapi -> getGameObjectRotation(playerId, false);
  auto newPos = glm::inverse(parentRot) * movementVec;
  return newPos;
}


glm::vec3 calcLocationWithRecoil(WeaponValues& weaponValues, glm::vec3 pos, bool isGunZoomed){
  auto targetPos = isGunZoomed ? weaponValues.gunCore.weaponParams.ironsightOffset : pos;
  auto recoilAmount = isGunZoomed ? weaponValues.gunCore.weaponParams.recoilZoomTranslate : weaponValues.gunCore.weaponParams.recoilTranslate;
  auto targetPosWithRecoil = glm::vec3(targetPos.x + recoilAmount.x, targetPos.y + recoilAmount.y, targetPos.z + recoilAmount.z);
  return glm::lerp(targetPos, targetPosWithRecoil, calcRecoilSlerpAmount(weaponValues.gunCore, weaponValues.gunCore.weaponParams.recoilLength, true));
}

glm::vec3 maxMagSway(0.1f, 0.1f, 0.05f);
float zoomSpeedMultiplier = 5.f;
float swayVelocity = 1.;
void swayGunTranslation(WeaponValues& weaponValues, glm::vec3 relVelocity, bool isGunZoomed){
  float swayAmountX =  relVelocity.x;
  float limitedSwayX = glm::min(maxMagSway.x, glm::max(swayAmountX, -1.f * maxMagSway.x));

  float swayAmountY =  relVelocity.y;
  float limitedSwayY = glm::min(maxMagSway.y, glm::max(swayAmountY, -1.f * maxMagSway.y));

  float swayAmountZ =  relVelocity.z;
  float limitedSwayZ = glm::min(maxMagSway.z, glm::max(swayAmountZ, -1.f * maxMagSway.z));  

  glm::vec3 targetPos(-1.f * limitedSwayX + weaponValues.gunCore.weaponParams.initialGunPos.x, -1.f * limitedSwayY + weaponValues.gunCore.weaponParams.initialGunPos.y, -1.f * limitedSwayZ + weaponValues.gunCore.weaponParams.initialGunPos.z); 
  
  auto targetPosWithRecoil = calcLocationWithRecoil(weaponValues, targetPos, isGunZoomed); // this should use ironsight-offset

  if (weaponValues.gunCore.weaponState.gunState == GUN_LOWERING){
    targetPosWithRecoil += glm::vec3(0.f, -1.f, 0.f);
  }

  auto gunId = weaponValues.weaponInstance.value().gunId;
  auto animationRate = weaponValues.gunCore.weaponState.gunState == GUN_LOWERING ? 5.f : 3.f;
  float lerpAmount = gameapi -> timeElapsed() * swayVelocity * (isGunZoomed ? zoomSpeedMultiplier : 1.f) * animationRate;
  auto newPos = glm::lerp(gameapi -> getGameObjectPos(gunId, false), targetPosWithRecoil, lerpAmount);  // probably pick a better function?  how does it feel tho
  //std::cout << "gun: targetpos: " << print(targetPosWithRecoil) << std::endl;
  //std::cout << "gun: newpos: " << print(newPos) << std::endl;
  gameapi -> setGameObjectPosition(gunId, newPos, false);
}

void swayGunRotation(WeaponValues& weaponValues, bool isGunZoomed, glm::vec2 lookVelocity, glm::vec3 movementVec){
  float swayAmountX = lookVelocity.x;
  float limitedSwayX = glm::min(maxMagSway.x, glm::max(swayAmountX, -1.f * maxMagSway.x));

  float swayAmountY = lookVelocity.y;
  float limitedSwayY = glm::min(maxMagSway.y, glm::max(swayAmountY, -1.f * maxMagSway.y));
  float recoilAmount =  glm::lerp(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, weaponValues.gunCore.weaponParams.recoilPitchRadians, 0.f), calcRecoilSlerpAmount(weaponValues.gunCore, weaponValues.gunCore.weaponParams.recoilLength, true)).y;

  float totalSwayY = limitedSwayY + recoilAmount * 5.f;
  //std::cout << "limited sway x: " << limitedSwayX << std::endl;
  //std::cout << "limited recoil: " << recoilAmount << std::endl;

  auto oldRotation = gameapi -> getGameObjectRotation(weaponValues.weaponInstance.value().gunId, false);
  auto rotation = gameapi -> setFrontDelta(
    parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)), 
    limitedSwayX /* delta yaw */, 
    totalSwayY /* should add recoil here */, 
    0.f, 
    0.1f /* why 0.1f? */
  );
  auto targetRotation = rotation * (isGunZoomed ? weaponValues.gunCore.weaponParams.ironSightAngle : weaponValues.gunCore.weaponParams.initialGunRot);
  gameapi -> setGameObjectRot(
    weaponValues.weaponInstance.value().gunId, 
    glm::slerp(oldRotation, targetRotation, 0.1f),
    false
  );
}

bool swayRotation = true;
void swayGun(WeaponValues& weaponValues, bool isGunZoomed, objid playerId, glm::vec2 lookVelocity, glm::vec3 movementVec){
  if (!weaponValues.weaponInstance.has_value()){
    return;
  }
  //modlog("weapon", "movement velocity: " + std::to_string(weapons.movementVelocity));
  //modlog("weapon", "sway velocity: " + print(swayVelocity));
  auto swayVelocity = getSwayVelocity(playerId, lookVelocity, movementVec);
  swayGunTranslation(weaponValues, swayVelocity, isGunZoomed);
  if (swayRotation){
    swayGunRotation(weaponValues, isGunZoomed, lookVelocity, movementVec);
  }
}

