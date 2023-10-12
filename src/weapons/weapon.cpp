#include "./weapon.h"

extern CustomApiBindings* gameapi;

enum GunAnimation { GUN_RAISED, GUN_LOWERING };

struct CurrentGun {
  std::string name;
  std::optional<objid> soundId;
  std::optional<std::string> soundClipObj;
  std::optional<objid> muzzleParticle;  // particle right in front of the muzzle, eg for a smoke effect
  std::optional<objid> hitParticles;    // default hit particle for the gun, used if there is no material particle
  std::optional<objid> projectileParticles;  // eg for a grenade launched from the gun

  float lastShootingTime;
  float recoilStart;
  int currentAmmo;
  GunAnimation gunState;
};


struct Weapons {
  std::vector<MaterialToParticle> materials;

  objid playerId;
  objid sceneId;
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;
  float selectDistance;

  WeaponParams weaponParams;
  std::optional<WeaponInstance> weaponInstance;
  CurrentGun currentGun;

  glm::vec2 lookVelocity;
  float movementVelocity;
  glm::vec3 movementVec;

  std::optional<objid> raycastLine;
  std::optional<objid> heldItem;
};

void saveGunTransform(Weapons& weapons){
  debugAssertForNow(false, "bad code - cannot get raw position / etc since ironsights mean this needs to subtract by initial offset");

  if (weapons.weaponInstance.has_value()){
    auto gunId = weapons.weaponInstance.value().gunId;
    auto gun = weapons.currentGun.name;
    auto attr = gameapi -> getGameObjectAttr(gunId);
    auto position = attr.vecAttr.vec3.at("position");  
    auto scale = attr.vecAttr.vec3.at("scale");
    auto rotation = attr.vecAttr.vec4.at("rotation");

    modlog("weapons", "save gun, name = " + weapons.currentGun.name + ",  pos = " + print(position) + ", scale = " + print(scale) + ", rotation = " + print(rotation));

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

void spawnGun(Weapons& weapons, objid sceneId, std::string name, std::string firesound, std::string particle, std::string hitParticle, std::string projectileParticle, std::string modelpath, std::string script, glm::vec3 gunpos, glm::vec4 rot, glm::vec3 scale){
  modlog("weapons", std::string("spawn gun: ") + name);

  if (weapons.weaponInstance.has_value()){
    weapons.currentGun.name = "";

    removeWeaponInstance(weapons.weaponInstance.value());


    if (weapons.currentGun.soundId.has_value()){
      gameapi -> removeObjectById(weapons.currentGun.soundId.value());
      weapons.currentGun.soundId = std::nullopt;
      weapons.currentGun.soundClipObj = std::nullopt;
    }

    if (weapons.currentGun.muzzleParticle.has_value()){
      gameapi -> removeObjectById(weapons.currentGun.muzzleParticle.value());
      weapons.currentGun.muzzleParticle = std::nullopt;
    }

    if (weapons.currentGun.hitParticles.has_value()){
      gameapi -> removeObjectById(weapons.currentGun.hitParticles.value());
      weapons.currentGun.hitParticles = std::nullopt;
    }
    if (weapons.currentGun.projectileParticles.has_value()){
      gameapi -> removeObjectById(weapons.currentGun.projectileParticles.value());
      weapons.currentGun.projectileParticles = std::nullopt;
    }
  }
  weapons.weaponInstance = std::nullopt;
 

  std::map<std::string, std::string> stringAttributes = { { "mesh", modelpath }, { "layer", "no_depth" } };
  if (script != ""){
    stringAttributes["script"] = script;
  }
  GameobjAttributes attr {
    .stringAttributes = stringAttributes,
    .numAttributes = {},
    .vecAttr = {  .vec3 = {{ "position", gunpos - glm::vec3(0.f, 0.f, 0.f) }, { "scale", scale }},  .vec4 = {{ "rotation", rot }}},
  };

  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto gunId = gameapi -> makeObjectAttr(sceneId, std::string("code-weapon-") + uniqueNameSuffix(), attr, submodelAttributes);
  modassert(gunId.has_value(), std::string("weapons could not spawn gun: ") + name);
  weapons.weaponInstance = WeaponInstance{};
  weapons.weaponInstance.value().gunId = gunId.value();


  if (firesound != ""){
    GameobjAttributes soundAttr {
      .stringAttributes = { { "clip", firesound }, { "physics", "disabled" }},
      .numAttributes = {},
      .vecAttr = {  .vec3 = {},  .vec4 = {} },
    };

    std::string soundClipObj = std::string("&code-weaponsound-") + uniqueNameSuffix();
    auto soundId = gameapi -> makeObjectAttr(sceneId, soundClipObj, soundAttr, submodelAttributes);
    weapons.currentGun.soundId = soundId;  
    weapons.currentGun.soundClipObj = soundClipObj;
  }

  weapons.currentGun.muzzleParticle = createParticleEmitter(sceneId, particle, "+code-muzzleparticles");
  weapons.currentGun.hitParticles = createParticleEmitter(sceneId, hitParticle, "+code-hitparticle");
  weapons.currentGun.projectileParticles = createParticleEmitter(sceneId, projectileParticle, "+code-projectileparticle");

  weapons.currentGun.name = name;

  gameapi -> makeParent(gunId.value(), weapons.playerId);
  if (weapons.currentGun.soundId.has_value()){
    gameapi -> makeParent(weapons.currentGun.soundId.value(), weapons.playerId);
  }
  if (weapons.currentGun.muzzleParticle.has_value()){
    gameapi -> makeParent(weapons.currentGun.muzzleParticle.value(), weapons.weaponInstance.value().gunId);
  }
  if (weapons.currentGun.projectileParticles.has_value()){
    gameapi -> makeParent(weapons.currentGun.projectileParticles.value(), weapons.weaponInstance.value().gunId);
  }
}

void changeGun(Weapons& weapons, objid id, objid sceneId, std::string gun, int ammo){
  weapons.weaponParams = queryWeaponParams(gun);
  weapons.weaponInstance = std::nullopt;
  weapons.currentGun.lastShootingTime = -1.f * weapons.weaponParams.firingRate ; // so you can shoot immediately
  weapons.currentGun.recoilStart = 0.f;
  weapons.currentGun.gunState = GUN_RAISED;
  weapons.currentGun.currentAmmo = ammo;
  gameapi -> sendNotifyMessage("current-gun", CurrentGunMessage {
    .currentAmmo = weapons.currentGun.currentAmmo,
    .totalAmmo = weapons.weaponParams.totalAmmo,
  });
  spawnGun(weapons, sceneId, gun, weapons.weaponParams.soundpath, weapons.weaponParams.muzzleParticleStr, weapons.weaponParams.hitParticleStr, weapons.weaponParams.projectileParticleStr, weapons.weaponParams.modelpath, weapons.weaponParams.script, weapons.weaponParams.initialGunPos, weapons.weaponParams.initialGunRotVec4, weapons.weaponParams.scale);
  if (weapons.weaponParams.idleAnimation.has_value() && weapons.weaponParams.idleAnimation.value() != "" && weapons.weaponInstance.value().gunId){
    gameapi -> playAnimation(weapons.weaponInstance.value().gunId, weapons.weaponParams.idleAnimation.value(), LOOP);
  }
}

std::string weaponsToString(Weapons& weapons){
  std::string str;
  str += std::string("isHoldingLeftMouse: ") + (weapons.isHoldingLeftMouse ? "true" : "false") + "\n";
  str += std::string("isHoldingRightMouseo: ") + (weapons.isHoldingRightMouse ? "true" : "false") + "\n";
  return str;
}

bool canFireGunNow(Weapons& weapons, float elapsedMilliseconds){
  auto timeSinceLastShot = elapsedMilliseconds - weapons.currentGun.lastShootingTime;
  bool lessThanFiringRate = timeSinceLastShot >= (0.001f * weapons.weaponParams.firingRate);
  return lessThanFiringRate;
}
void startRecoil(Weapons& weapons){
  weapons.currentGun.recoilStart = gameapi -> timeSeconds(false);
}

glm::vec3 zFightingForParticle(glm::vec3 pos, glm::quat normal){
  return gameapi -> moveRelative(pos, normal, 0.01);  // 0.01?
}

// fires from point of view of the camera
float maxRaycastDistance = 500.f;
std::vector<HitObject> doRaycast(Weapons& weapons, glm::vec3 orientationOffset){
  auto orientationOffsetQuat = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), orientationOffset);
  auto mainobjPos = gameapi -> getGameObjectPos(weapons.playerId, true);
  auto rot = gameapi -> getGameObjectRotation(weapons.playerId, true) *  orientationOffsetQuat;
  //  (define shotangle (if (should-zoom) rot (with-bloom rot)))
  auto hitpoints =  gameapi -> raycast(mainobjPos, rot, maxRaycastDistance);

  if (weapons.raycastLine.has_value()){
    gameapi -> freeLine(weapons.raycastLine.value());
  }
  
  //auto raycastLineId = gameapi -> drawLine(mainobjPos, gameapi -> moveRelative(mainobjPos, rot, maxRaycastDistance), true, playerId.value(), std::nullopt,  std::nullopt, std::nullopt);
  //weapons.raycastLine = raycastLineId;
  //for (auto &hitpoint : hitpoints){
  //  std::cout << "raycast hit: " << hitpoint.id << "- point: " << print(hitpoint.point) << ", normal: " << print(hitpoint.normal) << std::endl;
  //  showDebugHitmark(hitpoint, weapons.playerId);
  //}
  return hitpoints;
}

std::vector<HitObject> doRaycastClosest(Weapons& weapons, glm::vec3 cameraPos, glm::vec3 orientationOffset){
  auto hitpoints = doRaycast(weapons, orientationOffset);
  if (hitpoints.size() > 0){
    auto closestIndex = closestHitpoint(hitpoints, cameraPos);
    return { hitpoints.at(closestIndex) };
  }
  return hitpoints;
}


void fireRaycast(Weapons& weapons, glm::vec3 orientationOffset){
  auto cameraPos = gameapi -> getGameObjectPos(weapons.playerId, true);
  auto hitpoints = doRaycastClosest(weapons, cameraPos, orientationOffset);
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
      auto material = getHitMaterial(weapons.materials, objMaterial.value());
      std::cout << "hit particle material: (" << (material.has_value() && material.value() -> hitParticle.has_value() ? "has hit particle" : "no hit particle" ) << ") " << std::endl;
      if (material.has_value() && material.value() -> hitParticle.has_value()){
        emitterId = material.value() -> hitParticle.value().particleId;
      }
      if (material.has_value() && material.value() -> splashParticle.has_value()){
        splashEmitterId = material.value() -> splashParticle.value().particleId;
      }
    }

    if (weapons.currentGun.hitParticles.has_value()){
      emitterId = weapons.currentGun.hitParticles.value();
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

void tryFireGun(Weapons& weapons, objid sceneId, float bloomAmount){
  float now = gameapi -> timeSeconds(false);
  auto canFireGun = canFireGunNow(weapons, now);
  modlog("weapons", std::string("try fire gun, can fire = ") + (canFireGun ? "true" : "false") + ", now = " + std::to_string(now) + ", firing rate = " + std::to_string(weapons.weaponParams.firingRate));
  if (!canFireGun){
    return;
  }
  bool hasAmmo = weapons.currentGun.currentAmmo > 0;
  if (!hasAmmo){
    modlog("weapons", "no ammo, tried to fire, should play sound");
    return;
  }
  weapons.currentGun.currentAmmo--;
  gameapi -> sendNotifyMessage("current-gun", CurrentGunMessage {
    .currentAmmo = weapons.currentGun.currentAmmo,
    .totalAmmo = weapons.weaponParams.totalAmmo,
  });

  if (weapons.currentGun.soundId.has_value()){
    gameapi -> playClip(weapons.currentGun.soundClipObj.value(), sceneId, std::nullopt, std::nullopt);
  }
  if (weapons.weaponParams.fireAnimation.has_value()){
    gameapi -> playAnimation(weapons.weaponInstance.value().gunId, weapons.weaponParams.fireAnimation.value(), ONESHOT);
  }

  auto playerRotation = gameapi -> getGameObjectRotation(weapons.playerId, true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
  auto playerPos = gameapi -> getGameObjectPos(weapons.playerId, true);
  if (weapons.currentGun.muzzleParticle.has_value()){
    auto gunPosition = gameapi -> getGameObjectPos(weapons.weaponInstance.value().gunId, true);
    auto playerRotation = gameapi -> getGameObjectRotation(weapons.playerId, true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
    glm::vec3 distanceFromGun = glm::vec3(0.f, 0.f, -1.); // should parameterize particleOffset
    auto slightlyInFrontOfGun = gameapi -> moveRelativeVec(gunPosition, playerRotation, distanceFromGun);
    gameapi -> emit(weapons.currentGun.muzzleParticle.value(), slightlyInFrontOfGun, playerRotation, std::nullopt, std::nullopt, weapons.playerId);
  }
  weapons.currentGun.lastShootingTime = now;
  startRecoil(weapons);

  glm::vec3 shootingVecAngle(randomNumber(-bloomAmount, bloomAmount), randomNumber(-bloomAmount, bloomAmount), -1.f);
  if (weapons.weaponParams.isRaycast){
    fireRaycast(weapons, shootingVecAngle);
  }
  if (weapons.currentGun.projectileParticles.has_value()){
    auto fromPos = gameapi -> moveRelative(playerPos, playerRotation, 3);
    glm::vec3 projectileArc(0.f, 0.f, -1.f);
    auto playerForwardAndUp = playerRotation * gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), projectileArc);
    auto initialVelocity = playerForwardAndUp * shootingVecAngle * 10.f;
    gameapi -> emit(weapons.currentGun.projectileParticles.value(), fromPos, playerRotation, initialVelocity, std::nullopt, std::nullopt);
  }
}


bool swayFromMouse = true;
glm::vec3 getSwayVelocity(Weapons& weapons){
  if (swayFromMouse){
    debugAssertForNow(false, "sway from mouse should take into account sensitivity");
    return glm::vec3(weapons.lookVelocity.x, weapons.lookVelocity.y, 0.f);
  }
  auto parentRot = gameapi -> getGameObjectRotation(weapons.playerId, false);
  //modlog("weapons", "move velocity: " + print(weapons.movementVec));
  auto newPos = glm::inverse(parentRot) * weapons.movementVec;
  //std::cout << "sway velocity: " << print(newPos) << std::endl;
  return newPos;
}


float calcRecoilSlerpAmount(Weapons& weapons, float length,  bool reset){
  float amount = (gameapi -> timeSeconds(false) - weapons.currentGun.recoilStart) / length;
  return (amount > 1.f) ? (reset ? 0.f : 1.f): amount;
}

glm::vec3 calcLocationWithRecoil(Weapons& weapons, glm::vec3 pos, bool isGunZoomed){
  auto targetPos = isGunZoomed ? weapons.weaponParams.ironsightOffset : pos;
  auto recoilAmount = isGunZoomed ? weapons.weaponParams.recoilZoomTranslate : weapons.weaponParams.recoilTranslate;
  auto targetPosWithRecoil = glm::vec3(targetPos.x + recoilAmount.x, targetPos.y + recoilAmount.y, targetPos.z + recoilAmount.z);
  return glm::lerp(targetPos, targetPosWithRecoil, calcRecoilSlerpAmount(weapons, weapons.weaponParams.recoilLength, true));
}


glm::vec3 maxMagSway(0.1f, 0.1f, 0.05f);
float zoomSpeedMultiplier = 5.f;
float swayVelocity = 1.;
void swayGunTranslation(Weapons& weapons, glm::vec3 relVelocity, bool isGunZoomed){
  float swayAmountX =  relVelocity.x;
  float limitedSwayX = glm::min(maxMagSway.x, glm::max(swayAmountX, -1.f * maxMagSway.x));

  float swayAmountY =  relVelocity.y;
  float limitedSwayY = glm::min(maxMagSway.y, glm::max(swayAmountY, -1.f * maxMagSway.y));

  float swayAmountZ =  relVelocity.z;
  float limitedSwayZ = glm::min(maxMagSway.z, glm::max(swayAmountZ, -1.f * maxMagSway.z));  

  glm::vec3 targetPos(-1.f * limitedSwayX + weapons.weaponParams.initialGunPos.x, -1.f * limitedSwayY + weapons.weaponParams.initialGunPos.y, -1.f * limitedSwayZ + weapons.weaponParams.initialGunPos.z); 
  
  auto targetPosWithRecoil = calcLocationWithRecoil(weapons, targetPos, isGunZoomed); // this should use ironsight-offset

  if (weapons.currentGun.gunState == GUN_LOWERING){
    targetPosWithRecoil += glm::vec3(0.f, -1.f, 0.f);
  }

  auto gunId = weapons.weaponInstance.value().gunId;
  auto animationRate = weapons.currentGun.gunState == GUN_LOWERING ? 5.f : 3.f;
  float lerpAmount = gameapi -> timeElapsed() * swayVelocity * (isGunZoomed ? zoomSpeedMultiplier : 1.f) * animationRate;
  auto newPos = glm::lerp(gameapi -> getGameObjectPos(gunId, false), targetPosWithRecoil, lerpAmount);  // probably pick a better function?  how does it feel tho
  //std::cout << "gun: targetpos: " << print(targetPosWithRecoil) << std::endl;
  //std::cout << "gun: newpos: " << print(newPos) << std::endl;
  gameapi -> setGameObjectPosition(gunId, newPos, false);
}

void swayGunRotation(Weapons& weapons, glm::vec3 mouseVelocity, bool isGunZoomed){
  float swayAmountX = weapons.lookVelocity.x;
  float limitedSwayX = glm::min(maxMagSway.x, glm::max(swayAmountX, -1.f * maxMagSway.x));

  float swayAmountY = weapons.lookVelocity.y;
  float limitedSwayY = glm::min(maxMagSway.y, glm::max(swayAmountY, -1.f * maxMagSway.y));
  float recoilAmount =  glm::lerp(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, weapons.weaponParams.recoilPitchRadians, 0.f), calcRecoilSlerpAmount(weapons, weapons.weaponParams.recoilLength, true)).y;

  float totalSwayY = limitedSwayY + recoilAmount * 5.f;
  //std::cout << "limited sway x: " << limitedSwayX << std::endl;
  //std::cout << "limited recoil: " << recoilAmount << std::endl;

  auto oldRotation = gameapi -> getGameObjectRotation(weapons.weaponInstance.value().gunId, false);
  auto rotation = gameapi -> setFrontDelta(
    parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)), 
    limitedSwayX /* delta yaw */, 
    totalSwayY /* should add recoil here */, 
    0.f, 
    0.1f /* why 0.1f? */
  );
  auto targetRotation = rotation * (isGunZoomed ? weapons.weaponParams.ironSightAngle : weapons.weaponParams.initialGunRot);
  gameapi -> setGameObjectRot(
    weapons.weaponInstance.value().gunId, 
    glm::slerp(oldRotation, targetRotation, 0.1f),
    false
  );
}

bool swayRotation = true;
void swayGun(Weapons& weapons, bool isGunZoomed){
  if (!weapons.weaponInstance.has_value()){
    return;
  }

  auto swayVelocity = getSwayVelocity(weapons);
  //modlog("weapon", "movement velocity: " + std::to_string(weapons.movementVelocity));
  //modlog("weapon", "sway velocity: " + print(swayVelocity));
  swayGunTranslation(weapons, swayVelocity, isGunZoomed);
  if (swayRotation){
    swayGunRotation(weapons, glm::vec3(0.f, 0.f, 0.f) /* mouse-velocity  */, isGunZoomed);
  }
}

void reloadTraitsValues(Weapons& weapons){
  auto traitQuery = gameapi -> compileSqlQuery("select select-distance from traits where profile = ?", { "default" });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(traitQuery, &validSql);
  modassert(validSql, "error executing sql query");
  weapons.selectDistance = floatFromFirstSqlResult(result, 0);
}

glm::vec3 calculatePoint(float radians, float radius, glm::quat orientation){
  float x = glm::cos(radians) * radius;
  float y = glm::sin(radians) * radius;
  return orientation * glm::vec3(x, y, 0.f);
}

const int CIRCLE_RESOLUTION = 20;
void drawCircle(objid id, glm::vec3 pos, float radius, glm::quat orientation){
  auto lastPoint = calculatePoint(0, radius, orientation);
  //std::cout << std::endl;
  for (int i = 1; i <= CIRCLE_RESOLUTION; i++){
    auto radians = i * ((2 * MODPI) / static_cast<float>(CIRCLE_RESOLUTION));
    auto newPoint = calculatePoint(radians, radius, orientation);
    //std::cout << "radians = " << radians << ", new point is: " << print(newPoint) << std::endl;
    gameapi -> drawLine(lastPoint + pos,  newPoint + pos, false, id, std::nullopt, std::nullopt, std::nullopt);
    lastPoint = newPoint;
  } 
}

const glm::vec4 reticleColor(1.f, 1.f, 1.f, 0.5f);
const float markerLineLength = 0.01f;

bool shouldDrawMarkers = true;
void drawMarkers(objid id, glm::vec3 pos, float radius, glm::quat orientation){
  float marketInner = radius * 10.f;
  float markerOuter = radius * 10.f + markerLineLength; //
  gameapi -> drawLine2D(glm::vec3(0.f, marketInner, 0.f), glm::vec3(0.f, markerOuter, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawLine2D(glm::vec3(0.f, -1 * marketInner, 0.f), glm::vec3(0.f, -1 * markerOuter, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawLine2D(glm::vec3(marketInner, 0.f, 0.f), glm::vec3(markerOuter, 0.f, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawLine2D(glm::vec3(-1 * marketInner, 0.f, 0.f), glm::vec3(-1 * markerOuter, 0.f, 0.f), false, reticleColor, std::nullopt, true, std::nullopt, std::nullopt);
}


// draw a circle at a distance from the player with a certain radius
// this is independent of fov, and should be
void drawBloom(Weapons& weapons, objid id, float distance, float bloomAmount){

  // i'd rather do this on a screen only texture
  modassert(distance < 0, "distance must be negative (forward -z)");
  auto mainobjPos = gameapi -> getGameObjectPos(weapons.playerId, true);
  auto mainobjRot = gameapi -> getGameObjectRotation(weapons.playerId, true);
  auto toPos = mainobjPos + mainobjRot * glm::vec3(0.f, 0.f, distance);
  gameapi -> drawLine(mainobjPos, toPos, false, id, reticleColor, std::nullopt, std::nullopt);
  
  if (shouldDrawMarkers){
    drawMarkers(id, toPos, bloomAmount, mainobjRot);
  }else{
    float radius = glm::max(0.002f, bloomAmount);
    drawCircle(id, toPos, radius, mainobjRot);
  }
}

float calculateBloomAmount(Weapons& weapons){
  auto slerpAmount = (1 - calcRecoilSlerpAmount(weapons, weapons.weaponParams.bloomLength, false)); 
  modassert(slerpAmount <= 1, "slerp amount must be less than 1, got: " + std::to_string(slerpAmount));
  return glm::max(weapons.weaponParams.minBloom, (weapons.weaponParams.totalBloom - weapons.weaponParams.minBloom) * slerpAmount + weapons.weaponParams.minBloom);
}


// Should interpolate.  Looks better + prevent clipping bugs
// Might be interesting to incorporate things like mass and stuff
void handlePickedUpItem(Weapons& weapons){
  if (!weapons.heldItem.has_value()){
    return;
  }

  auto playerPos = gameapi -> getGameObjectPos(weapons.playerId, true);
  auto playerRotation = gameapi -> getGameObjectRotation(weapons.playerId, true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
  glm::vec3 distanceFromPlayer = glm::vec3(0.f, 0.f, -5.f); 
  auto slightlyInFrontOfPlayer = gameapi -> moveRelativeVec(playerPos, playerRotation, distanceFromPlayer);

  // old object position
  auto oldItemPos = gameapi -> getGameObjectPos(weapons.heldItem.value(), true);
  auto towardPlayerView = slightlyInFrontOfPlayer - oldItemPos;
  auto distance = glm::length(towardPlayerView);
  auto direction = glm::normalize(towardPlayerView);

  glm::vec3 amountToMove = (direction); // * gameapi -> timeSeconds(false);

  auto elapsedTime = gameapi -> timeSeconds(false);
  amountToMove.x *= 10;
  amountToMove.y *= 10;
  amountToMove.z *= 10;
  std::cout << "amount to move: " << print(amountToMove) << std::endl;
  //gameapi -> setGameObjectPos(weapons.heldItem.value(), oldItemPos + amountToMove);
  gameapi -> applyForce(weapons.heldItem.value(), amountToMove);

}


void modifyPhysicsForHeldItem(Weapons& weapons){
  GameobjAttributes newAttr {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = { 
      .vec3 = { 
        { "physics_avelocity", glm::vec3(0.f, 0.f, 0.f) }, 
        { "physics_velocity", glm::vec3(0.f, 0.f, 0.f) },
        { "physics_angle", glm::vec3(0.f, 0.f, 0.f) }, 
        { "physics_linear", glm::vec3(1.f, 1.f, 1.f) }, 
        { "physics_gravity", glm::vec3(0.f, 0.f, 0.f) }, 
      }, 
      .vec4 = { } 
    },
  };
  gameapi -> setGameObjectAttr(weapons.heldItem.value(), newAttr);
}

void changeWeaponTargetId(Weapons& weapons, objid id){
  weapons.playerId = id;
  reloadTraitsValues(weapons);
  //changeGun(weapons, id, gameapi -> listSceneId(id), "pistol", 10);
}

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Weapons* weapons = new Weapons;
    weapons -> materials = loadMaterials(sceneId);

    weapons -> playerId = gameapi -> getGameObjectByName(">maincamera", sceneId, false).value();
    weapons -> sceneId = sceneId;
    weapons -> isHoldingLeftMouse = false;
    weapons -> isHoldingRightMouse = false;

    weapons -> lookVelocity = glm::vec2(0.f, 0.f);
    weapons -> movementVelocity = 0.f;
    weapons -> movementVec = glm::vec3(0.f, 0.f, 0.f);
    weapons -> raycastLine = std::nullopt;

    weapons -> currentGun = CurrentGun {
      .name = "",
      .soundId = std::nullopt,
      .soundClipObj = std::nullopt,
      .muzzleParticle = std::nullopt,
    };

    weapons -> heldItem = std::nullopt;

    changeWeaponTargetId(*weapons, weapons -> playerId);
  	return weapons;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    delete weapons;
  };
  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    if (isPaused()){
      return;
    }
    Weapons* weapons = static_cast<Weapons*>(data);
    if (button == 0){
      if (action == 0){
        weapons -> isHoldingLeftMouse = false;
      }else if (action == 1){
        weapons -> isHoldingLeftMouse = true;
        tryFireGun(*weapons, gameapi -> listSceneId(id), calculateBloomAmount(*weapons));
      }
    }else if (button == 1){
      if (action == 0){
        weapons -> isHoldingRightMouse = false;
      }else if (action == 1){
        // select item
        weapons -> isHoldingRightMouse = true;
        auto hitpoints = doRaycast(*weapons, glm::vec3(0.f, 0.f, -1.f));
        if (hitpoints.size() > 0){
          auto cameraPos = gameapi -> getGameObjectPos(weapons -> playerId, true);
          auto closestIndex = closestHitpoint(hitpoints, cameraPos);
          float distance = glm::length(cameraPos - hitpoints.at(closestIndex).point);
          if (distance <= weapons -> selectDistance){
            gameapi -> sendNotifyMessage("selected", hitpoints.at(closestIndex).id);
          }
        }
      }
    }
  };
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    if (key == 'E') { 
      if (action == 1){
        if (weapons -> heldItem.has_value()){
          modlog("weapons", "pickup released held item: " + std::to_string(weapons -> heldItem.value()));
          weapons -> heldItem = std::nullopt;
        }else{
        auto hitpoints = doRaycast(*weapons, glm::vec3(0.f, 0.f, -1.f));
        if (hitpoints.size() > 0){
            auto cameraPos = gameapi -> getGameObjectPos(weapons -> playerId, true);
            auto closestHitpointIndex = closestHitpoint(hitpoints, cameraPos);
            auto hitpoint = hitpoints.at(closestHitpointIndex);
            float distance = glm::length(cameraPos - hitpoint.point);
            auto attr = gameapi -> getGameObjectAttr(hitpoint.id);
            auto physicsEnabled = getStrAttr(attr, "physics").value() == "enabled";
            auto physicsDynamic = getStrAttr(attr, "physics_type").value() == "dynamic";
            auto physicsCollide = getStrAttr(attr, "physics_collision").value() == "collide";
            auto canPickup = physicsEnabled && physicsDynamic && physicsCollide ;
            modlog("weapons", "pickup item: " + std::to_string(hitpoint.id) + " can pickup: " + print(canPickup) + " distance = " + std::to_string(distance));
            if (canPickup && distance < 5.f){
              weapons -> heldItem = hitpoint.id;
              modifyPhysicsForHeldItem(*weapons);
            }
          }
        }
      }
      return;
    }
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    Weapons* weapons = static_cast<Weapons*>(data);
    if (key == "change-gun"){
      auto changeGunMessage = anycast<ChangeGunMessage>(value); 
      modassert(changeGunMessage != NULL, "change-gun value invalid");
      auto value = changeGunMessage -> gun;
      auto currentAmmo = changeGunMessage -> currentAmmo;
      if (value != weapons -> currentGun.name){
        gameapi -> sendNotifyMessage("set-gun-ammo", SetAmmoMessage {
          .currentAmmo = weapons -> currentGun.currentAmmo,
          .gun = weapons -> currentGun.name,
        });

        weapons -> currentGun.gunState = GUN_LOWERING;
        gameapi -> schedule(id, 1000, weapons, [id, value, currentAmmo](void* weaponData) -> void {
          Weapons* weaponValue = static_cast<Weapons*>(weaponData);
          changeGun(*weaponValue, id, gameapi -> listSceneId(id), value, currentAmmo);
        });
      }
    }else if (key == "ammo"){
      auto intValue = anycast<int>(value);
      modassert(intValue != NULL, "ammo message not an int");
      weapons -> currentGun.currentAmmo += *intValue;
      gameapi -> sendNotifyMessage("current-gun", CurrentGunMessage {
        .currentAmmo = weapons -> currentGun.currentAmmo,
        .totalAmmo = weapons -> weaponParams.totalAmmo,
      });
    }else if (key == "save-gun"){
      saveGunTransform(*weapons);
    }else if (key == "velocity"){
      auto strValue = anycast<std::string>(value);   // would be nice to send the vec3 directly, but notifySend does not support
      modassert(strValue != NULL, "velocity value invalid");  
      weapons -> movementVec = parseVec(*strValue);
      weapons -> movementVelocity = glm::length(weapons -> movementVec);
      //std::cout << "speed is: " << weapons -> movementVelocity << std::endl;
    }else if (key == "reload-config:weapon:traits"){
      Weapons* weapons = static_cast<Weapons*>(data);
      reloadTraitsValues(*weapons);
    }else if (key == "request:change-control"){
      Weapons* weapons = static_cast<Weapons*>(data);
      auto objIdValue = anycast<objid>(value); 
      modassert(objIdValue != NULL, "weapons - request change control value invalid");
      std::cout << "weapons want to change value: " << objIdValue << std::endl;
      changeWeaponTargetId(*weapons, *objIdValue);
    }
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    if (isPaused()){
      return;
    }
    //std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    Weapons* weapons = static_cast<Weapons*>(data);
    weapons -> lookVelocity = glm::vec2(xPos, yPos);
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    if (isPaused()){
      return;
    }
    Weapons* weapons = static_cast<Weapons*>(data);
    auto bloomAmount = calculateBloomAmount(*weapons);
    drawBloom(*weapons, id, -1.f, bloomAmount); // 0.002f is just a min amount for visualization, not actual bloom
    if (weapons -> weaponParams.canHold && weapons -> isHoldingLeftMouse){
      tryFireGun(*weapons, gameapi -> listSceneId(id), bloomAmount);
    }
    swayGun(*weapons, weapons -> isHoldingRightMouse);
    handlePickedUpItem(*weapons);
    //std::cout << weaponsToString(*weapons) << std::endl;
  };
  return binding;
}

