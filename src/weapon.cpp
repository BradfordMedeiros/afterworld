#include "./weapon.h"

extern CustomApiBindings* gameapi;

struct WeaponParams {
  float firingRate;
  float recoilLength;
  float recoilPitchRadians;
  glm::vec3 recoilTranslate;
  glm::vec3 recoilZoomTranslate;
  bool canHold;
  bool isIronsight;
  bool isRaycast;
  glm::vec3 ironsightOffset;
};

struct CurrentGun {
  std::string name;
  std::optional<objid> gunId;
  std::optional<objid> soundId;
  std::optional<objid> muzzleParticle;
  std::optional<objid> hitParticles;    // probably this isn't actually particle to the gun 
  std::optional<objid> projectileParticles;

  std::optional<std::string> fireAnimation;

  float lastShootingTime;
  float recoilStart;
  glm::vec3 initialGunPos;
};

struct Weapons {
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;

  WeaponParams weaponParams;
  float selectDistance;
  CurrentGun currentGun;

  glm::vec2 lookVelocity;
  float movementVelocity;

  std::optional<objid> raycastLine;
};

void saveGunTransform(Weapons& weapons){
  debugAssertForNow(false, "bad code - cannot get raw position / etc since ironsights mean this needs to subtract by initial offset");

  if (weapons.currentGun.gunId.has_value()){
    auto gunId = weapons.currentGun.gunId.value();
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

std::string parentName = ">maincamera";


std::string templateValues(std::string& particleStr, std::map<std::string, std::string> templateValues){
  auto particleStrTemplated = particleStr;
  for (auto &[valueToReplace, replacedValue] : templateValues){
    particleStrTemplated = std::regex_replace(particleStrTemplated, std::regex(valueToReplace), replacedValue);
  }
  return particleStr;
}
GameobjAttributes particleAttributes(std::string& particle){
  auto templateEmitLine = split(particle, ';');

  GameobjAttributes particleAttr {
    .stringAttributes = { 
      { "state", "disabled" },    // default should keep
      { "physics", "disabled" },  
      { "layer", "no_depth" },    ///////
    },
    .numAttributes = { { "duration", 10.f } },
    .vecAttr = {  
    },
  };

  for (auto &line : templateEmitLine){
    auto keyValuePair = split(line, ':');
    modassert(keyValuePair.size() == 2, "invalid emitter particle attr, line: " + line + ", size = " + std::to_string(keyValuePair.size()));
    addFieldDynamic(particleAttr, keyValuePair.at(0), keyValuePair.at(1));  // should probably use only explicitly allowed api methods
  }
  return particleAttr;
}

void spawnGun(Weapons& weapons, objid sceneId, std::string name, std::string firesound, std::string particle, std::string hitParticle, std::string projectileParticle, std::string modelpath, std::string script, glm::vec3 gunpos, glm::vec4 rot, glm::vec3 scale){
  if (weapons.currentGun.gunId.has_value()){
    weapons.currentGun.name = "";

    gameapi -> removeObjectById(weapons.currentGun.gunId.value());
    weapons.currentGun.gunId = std::nullopt;

    if (weapons.currentGun.soundId.has_value()){
      gameapi -> removeObjectById(weapons.currentGun.soundId.value());
      weapons.currentGun.soundId = std::nullopt;
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

  std::map<std::string, std::string> stringAttributes = { { "mesh", modelpath }, { "layer", "no_depth" } };
  if (script != ""){
    stringAttributes["script"] = script;
  }
  GameobjAttributes attr {
    .stringAttributes = stringAttributes,
    .numAttributes = {},
    .vecAttr = {  .vec3 = {{ "position", gunpos }, { "scale", scale }},  .vec4 = {{ "rotation", rot }}},
  };

  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto gunId = gameapi -> makeObjectAttr(sceneId, "code-weapon", attr, submodelAttributes);
  modassert(gunId.has_value(), "gun does not have a value");
  weapons.currentGun.gunId = gunId;

  if (firesound != ""){
    GameobjAttributes soundAttr {
      .stringAttributes = { { "clip", firesound }, { "physics", "disabled" }},
      .numAttributes = {},
      .vecAttr = {  .vec3 = {},  .vec4 = {} },
    };
    auto soundId = gameapi -> makeObjectAttr(sceneId, "&code-weaponsound", soundAttr, submodelAttributes);
    weapons.currentGun.soundId = soundId;  
  }

  if (particle != ""){
    auto particleAttr = particleAttributes(particle);
    weapons.currentGun.muzzleParticle = gameapi -> makeObjectAttr(sceneId, "+code-muzzleparticles", particleAttr, submodelAttributes);
  }
  if (hitParticle != ""){
    auto particleAttr = particleAttributes(hitParticle);
    weapons.currentGun.hitParticles = gameapi -> makeObjectAttr(sceneId, "+code-hitparticle", particleAttr, submodelAttributes);
  }

  if (projectileParticle != ""){
    //projectileParticles
    auto projectileAttr = particleAttributes(projectileParticle);
    weapons.currentGun.projectileParticles = gameapi -> makeObjectAttr(sceneId, "+code-projectileparticle", projectileAttr, submodelAttributes);
  }

  weapons.currentGun.name = name;
  auto parent = gameapi -> getGameObjectByName(parentName, sceneId, false);
  modassert(parent.has_value(), parentName + " does not exist in scene so cannot create gun");
  gameapi -> makeParent(gunId.value(), parent.value());
  if (weapons.currentGun.soundId.has_value()){
    gameapi -> makeParent(weapons.currentGun.soundId.value(), parent.value());
  }
  if (weapons.currentGun.muzzleParticle.has_value()){
    gameapi -> makeParent(weapons.currentGun.muzzleParticle.value(), weapons.currentGun.gunId.value());
  }
  if (weapons.currentGun.projectileParticles.has_value()){
    gameapi -> makeParent(weapons.currentGun.projectileParticles.value(), weapons.currentGun.gunId.value());
  }
}

void changeGun(Weapons& weapons, objid sceneId, std::string gun){
  auto gunQuery = gameapi -> compileSqlQuery(
   std::string("select modelpath, fire-animation, fire-sound, xoffset-pos, ") +
   "yoffset-pos, zoffset-pos, xrot, yrot, zrot, xscale, yscale, zscale, " + 
   "firing-rate, hold, raycast, ironsight, iron-xoffset-pos, iron-yoffset-pos, " + 
   "iron-zoffset-pos, particle, hit-particle, recoil-length, recoil-angle, " + 
   "recoil, recoil-zoom, projectile, bloom, script, fireanimation " + 
   "from guns where name = " + gun,
   {}
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(gunQuery, &validSql);
  modassert(validSql, "error executing sql query");

  modassert(result.size() > 0, "no gun named: " + gun);
  modassert(result.size() == 1, "more than one gun named: " + gun);
  modlog("weapons", "gun: result: " + print(result.at(0)));

  weapons.weaponParams.firingRate = floatFromFirstSqlResult(result, 12);
  weapons.weaponParams.recoilLength = floatFromFirstSqlResult(result, 21);
  weapons.weaponParams.recoilPitchRadians = floatFromFirstSqlResult(result, 22);
  weapons.weaponParams.recoilTranslate = vec3FromFirstSqlResult(result, 23);
  weapons.weaponParams.recoilZoomTranslate = vec3FromFirstSqlResult(result, 24);
  weapons.weaponParams.canHold = boolFromFirstSqlResult(result, 13);
  weapons.weaponParams.isIronsight = boolFromFirstSqlResult(result, 15);
  weapons.weaponParams.isRaycast = boolFromFirstSqlResult(result, 14);
  weapons.weaponParams.ironsightOffset = vec3FromFirstSqlResult(result, 16, 17, 18);

  weapons.currentGun.lastShootingTime = -1.f * weapons.weaponParams.firingRate ; // so you can shoot immediately
  weapons.currentGun.recoilStart = 0.f;

  auto fireAnimation = strFromFirstSqlResult(result, 28);
  weapons.currentGun.fireAnimation = std::nullopt;
  if(fireAnimation != ""){
    weapons.currentGun.fireAnimation = fireAnimation;
  }

  auto gunpos = vec3FromFirstSqlResult(result, 3, 4, 5);
  weapons.currentGun.initialGunPos = gunpos;

  auto soundpath = strFromFirstSqlResult(result, 2);
  auto modelpath = strFromFirstSqlResult(result, 0);
  auto script = strFromFirstSqlResult(result, 27);

  auto rot3 = vec3FromFirstSqlResult(result, 6, 7, 8);
  auto rot4 = glm::vec4(rot3.x, rot3.y, rot3.z, 01.f);
  auto scale = vec3FromFirstSqlResult(result, 9, 10, 11);

  auto muzzleParticleStr = strFromFirstSqlResult(result, 19);
  auto hitParticleStr = strFromFirstSqlResult(result, 20);
  auto projectileParticleStr = strFromFirstSqlResult(result, 25);

  spawnGun(weapons, sceneId, gun, soundpath, muzzleParticleStr, hitParticleStr, projectileParticleStr, modelpath, script, gunpos, rot4, scale);
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


float hitlength = 2;
void showDebugHitmark(HitObject& hitpoint, objid playerId){
  gameapi -> drawLine(
    hitpoint.point + glm::vec3(0.f, -1.f * hitlength,  0.f),
    hitpoint.point + glm::vec3(0.f, 1.f * hitlength,  0.f),
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );
  gameapi -> drawLine(
    hitpoint.point + glm::vec3(-1.f * hitlength, 0.f, 0.f),
    hitpoint.point + glm::vec3(1.f * hitlength, 0.f, 0.f),
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );
  gameapi -> drawLine(
    hitpoint.point + glm::vec3(0.f, 0.f, -1.f * hitlength),
    hitpoint.point + glm::vec3(0.f, 0.f, 1.f * hitlength),
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );

  gameapi -> drawLine(
    hitpoint.point,
    hitpoint.point + (10.f * (hitpoint.normal * glm::vec3(0.f, 0.f, -1.f))),
    true, playerId, glm::vec4(1.f, 0.f, 0.f, 1.f),  std::nullopt, std::nullopt
  );
}

glm::vec3 zFightingForParticle(glm::vec3 pos, glm::quat normal){
  return gameapi -> moveRelative(pos, normal, 0.01);  // 0.01?
}

// fires from point of view of the camera
float maxRaycastDistance = 500.f;
std::vector<HitObject> doRaycast(Weapons& weapons, objid sceneId){
  auto playerId = gameapi -> getGameObjectByName(parentName, sceneId, false);
  auto mainobjPos = gameapi -> getGameObjectPos(playerId.value(), true);
  auto rot = gameapi -> getGameObjectRotation(playerId.value(), true);
  //  (define shotangle (if (should-zoom) rot (with-bloom rot)))
  auto hitpoints =  gameapi -> raycast(mainobjPos, rot, maxRaycastDistance);

  if (weapons.raycastLine.has_value()){
    gameapi -> freeLine(weapons.raycastLine.value());
  }
  
  /*
  auto raycastLineId = gameapi -> drawLine(mainobjPos, gameapi -> moveRelative(mainobjPos, rot, maxRaycastDistance), true, playerId.value(), std::nullopt,  std::nullopt, std::nullopt);
  weapons.raycastLine = raycastLineId;
  for (auto &hitpoint : hitpoints){
    std::cout << "raycast hit: " << hitpoint.id << "- point: " << print(hitpoint.point) << ", normal: " << print(hitpoint.normal) << std::endl;
    showDebugHitmark(hitpoint, playerId.value());
  }*/
  return hitpoints;
}


void fireRaycast(Weapons& weapons, objid sceneId){
  auto hitpoints = doRaycast(weapons, sceneId);
  modlog("weapons", "fire raycast, total hits = " + std::to_string(hitpoints.size()));
  for (auto &hitpoint : hitpoints){
    modlog("weapons", "raycast hit: " + std::to_string(hitpoint.id) + "- point: " + print(hitpoint.point) + ", normal: " + print(hitpoint.normal));
    if (weapons.currentGun.hitParticles.has_value()){
      gameapi -> emit(weapons.currentGun.hitParticles.value(), zFightingForParticle(hitpoint.point, hitpoint.normal), hitpoint.normal, std::nullopt, std::nullopt);
    }
    gameapi -> sendNotifyMessage("damage." + std::to_string(hitpoint.id), 50);
    modlog("weapons", "raycast normal: " + serializeQuat(hitpoint.normal));
  }
}

void tryFireGun(Weapons& weapons, objid sceneId){
  float now = gameapi -> timeSeconds(false);
  auto canFireGun = canFireGunNow(weapons, now);

  modlog("weapons", std::string("try fire gun, can fire = ") + (canFireGun ? "true" : "false") + ", now = " + std::to_string(now) + ", firing rate = " + std::to_string(weapons.weaponParams.firingRate));
  if (!canFireGun){
    return;
  }
  if (weapons.currentGun.soundId.has_value()){
    gameapi -> playClip("&code-weaponsound", sceneId);
  }
  if (weapons.currentGun.fireAnimation.has_value()){
    gameapi -> playAnimation(weapons.currentGun.gunId.value(), weapons.currentGun.fireAnimation.value(), false);
  }

  auto playerId = gameapi -> getGameObjectByName(parentName, gameapi -> listSceneId(weapons.currentGun.gunId.value()), false);
  auto playerRotation = gameapi -> getGameObjectRotation(playerId.value(), true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
  auto playerPos = gameapi -> getGameObjectPos(playerId.value(), true);
  if (weapons.currentGun.muzzleParticle.has_value()){
    auto gunPosition = gameapi -> getGameObjectPos(weapons.currentGun.gunId.value(), true);
    auto playerId = gameapi -> getGameObjectByName(parentName, gameapi -> listSceneId(weapons.currentGun.gunId.value()), false);
    auto playerRotation = gameapi -> getGameObjectRotation(playerId.value(), true);  // maybe this should be the gun rotation instead, problem is the offsets on the gun
    glm::vec3 distanceFromGun = glm::vec3(0.f, 0.f, -0.1f); // should parameterize particleOffset
    auto slightlyInFrontOfGun = gameapi -> moveRelativeVec(gunPosition, playerRotation, distanceFromGun);
    gameapi -> emit(weapons.currentGun.muzzleParticle.value(), slightlyInFrontOfGun, playerRotation, std::nullopt, std::nullopt);
  }
  weapons.currentGun.lastShootingTime = now;
  startRecoil(weapons);
  if (weapons.weaponParams.isRaycast){
    fireRaycast(weapons, sceneId);
  }
  if (weapons.currentGun.projectileParticles.has_value()){
    auto fromPos = gameapi -> moveRelative(playerPos, playerRotation, 3);
    glm::vec3 projectileArc(0.f, 0.f, -1.f);
    auto playerForwardAndUp = playerRotation * gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), projectileArc);
    auto initialVelocity = playerForwardAndUp * glm::vec3(0.f, 0.f, -1.f) * 10.f;
    gameapi -> emit(weapons.currentGun.projectileParticles.value(), fromPos, playerRotation, initialVelocity, std::nullopt);
  }
}


bool swayFromMouse = false;
glm::vec3 getSwayVelocity(Weapons& weapons){
  if (swayFromMouse){
    debugAssertForNow(false, "sway from mouse should take into account sensitivity");
    return glm::vec3(weapons.lookVelocity.x, weapons.lookVelocity.y, 0.f);
  }

 
  auto parent = gameapi -> getGameObjectByName(parentName, gameapi -> listSceneId(weapons.currentGun.gunId.value()), false);
  auto parentRot = gameapi -> getGameObjectRotation(parent.value(), false);
  auto newPos = gameapi -> moveRelative(glm::vec3(0.f, 0.f, 0.f), glm::inverse(parentRot), weapons.movementVelocity);
  //std::cout << "sway velocity: " << print(newPos) << std::endl;
  return newPos;
}


float calcRecoilSlerpAmount(Weapons& weapons){
  float amount = (gameapi -> timeSeconds(false) - weapons.currentGun.recoilStart) / weapons.weaponParams.recoilLength;
  return (amount > 1.f) ? 0 : amount;
}

glm::vec3 calcLocationWithRecoil(Weapons& weapons, glm::vec3 pos, bool isGunZoomed){
  auto targetPos = isGunZoomed ? weapons.weaponParams.ironsightOffset : pos;
  auto recoilAmount = isGunZoomed ? weapons.weaponParams.recoilZoomTranslate : weapons.weaponParams.recoilTranslate;
  auto targetPosWithRecoil = glm::vec3(targetPos.x + recoilAmount.x, targetPos.y + recoilAmount.y, targetPos.z + recoilAmount.z);
  return glm::lerp(targetPos, targetPosWithRecoil, calcRecoilSlerpAmount(weapons));
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

  glm::vec3 targetPos(-1.f * limitedSwayX + weapons.currentGun.initialGunPos.x, -1.f * limitedSwayY + weapons.currentGun.initialGunPos.y, -1.f * limitedSwayZ + weapons.currentGun.initialGunPos.z); 
  
  auto targetPosWithRecoil = calcLocationWithRecoil(weapons, targetPos, isGunZoomed); // this should use ironsight-offset
  auto gunId = weapons.currentGun.gunId.value();
  float lerpAmount = gameapi -> timeElapsed() * swayVelocity * (isGunZoomed ? zoomSpeedMultiplier : 1.f);
  auto newPos = glm::lerp(gameapi -> getGameObjectPos(gunId, false), targetPosWithRecoil, lerpAmount);
  gameapi -> setGameObjectPosRelative(gunId, newPos);
  /*


  (if zoomgun
    (gameobj-setpos-rel! 
      (gameobj-by-id gunid) 
      (lerp (gameobj-pos (gameobj-by-id gunid)) (locationWithRecoil ironsight-offset #t) (* (time-elapsed) 10))
    ) 

  )
  )*/
}

void swayGunRotation(Weapons& weapons, glm::vec3 mouseVelocity, bool isGunZoomed){
  //(define (sway-gun-rotation relvelocity zoomgun)
  //  (define sway-amount-x (list-ref relvelocity 0))
  //  (define limited-sway-x (min max-mag-sway-x-rot (max sway-amount-x (* -1 max-mag-sway-x-rot))))
  //  (define sway-amount-y (list-ref relvelocity 1))
  //  (define limited-sway-y (min max-mag-sway-y-rot (max sway-amount-y (* -1 max-mag-sway-y-rot))))
  //  (define recoilAmount (cadr (lerp (list 0 0 0) (list 0 recoilPitchRadians 0) (calc-recoil-slerpamount))))
  //  (define targetrot 
  //    (quatmult 
  //      (setfrontdelta forwardvec limited-sway-x (+ recoilAmount (* -1 limited-sway-y)) 0) ; yaw, pitch, roll 
  //      initial-gun-rot 
  //    )  
  //  )
  //  ;(format #t "sway x: ~a, sway y: ~a\n" limited-sway-x limited-sway-y)
  //  (gameobj-setrot! 
  //    (gameobj-by-id gunid) 
  //    (slerp (gameobj-rot (gameobj-by-id gunid)) targetrot 0.01)
  //  )  
  //)
}

bool swayRotation = false;
void swayGun(Weapons& weapons, bool isGunZoomed){
  if (!weapons.currentGun.gunId.has_value()){
    return;
  }
  swayGunTranslation(weapons, getSwayVelocity(weapons), isGunZoomed);
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

int closestHitpoint(std::vector<HitObject>& hitpoints, glm::vec3 playerPos){
  modassert(hitpoints.size() > 0, "hitpoints object is size 0");
  int closestIndex = 0;
  auto minDistance = glm::distance(playerPos, hitpoints.at(0).point);
  for (int i = 1; i < hitpoints.size(); i++){
    if (glm::distance(playerPos, hitpoints.at(i).point) < minDistance){
      closestIndex = i;
    }
  }
  return closestIndex;
}

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Weapons* weapons = new Weapons;

    weapons -> isHoldingLeftMouse = false;
    weapons -> isHoldingRightMouse = false;

    weapons -> lookVelocity = glm::vec2(0.f, 0.f);
    weapons -> movementVelocity = 0.f;

    weapons -> raycastLine = std::nullopt;

    weapons -> currentGun = CurrentGun {
      .name = "",
      .gunId = std::nullopt,
      .soundId = std::nullopt,
      .muzzleParticle = std::nullopt,
    };

    reloadTraitsValues(*weapons);
    changeGun(*weapons, gameapi -> listSceneId(id), "pistol");
  	return weapons;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    delete weapons;
  };
  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    if (button == 0){
      if (action == 0){
        weapons -> isHoldingLeftMouse = false;
      }else if (action == 1){
        weapons -> isHoldingLeftMouse = true;
        tryFireGun(*weapons, gameapi -> listSceneId(id));
      }
    }else if (button == 1){
      if (action == 0){
        weapons -> isHoldingRightMouse = false;
      }else if (action == 1){
        // select item
        weapons -> isHoldingRightMouse = true;
        auto hitpoints = doRaycast(*weapons, gameapi -> listSceneId(id));
        if (hitpoints.size() > 0){
          auto cameraObj = gameapi -> getGameObjectByName(parentName, gameapi -> listSceneId(id), false);
          auto cameraPos = gameapi -> getGameObjectPos(cameraObj.value(), true);
          auto closestIndex = closestHitpoint(hitpoints, cameraPos);
          float distance = glm::length(cameraPos - hitpoints.at(closestIndex).point);
          if (distance <= weapons -> selectDistance){
            gameapi -> sendNotifyMessage("selected", std::to_string(hitpoints.at(closestIndex).id));
          }
        }
      }
    }
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
    Weapons* weapons = static_cast<Weapons*>(data);
    if (key == "change-gun"){
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "change-gun value invalid");
      changeGun(*weapons, gameapi -> listSceneId(id), *strValue);
    }else if (key == "save-gun"){
      saveGunTransform(*weapons);
    }else if (key == "velocity"){
      auto strValue = std::get_if<std::string>(&value);   // would be nice to send the vec3 directly, but notifySend does not support
      modassert(strValue != NULL, "velocity value invalid");  
      weapons -> movementVelocity = glm::length(parseVec(*strValue));
      //std::cout << "speed is: " << weapons -> movementVelocity << std::endl;
    }else if (key == "reload-config:weapon:traits"){
      Weapons* weapons = static_cast<Weapons*>(data);
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "reload-traits:weapon reload value invalid");
      reloadTraitsValues(*weapons);
    }
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    //std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    Weapons* weapons = static_cast<Weapons*>(data);
    weapons -> lookVelocity = glm::vec2(xPos, yPos);
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);

    if (weapons -> weaponParams.canHold && weapons -> isHoldingLeftMouse){
      tryFireGun(*weapons, gameapi -> listSceneId(id));
    }

    swayGun(*weapons, weapons -> isHoldingRightMouse);

    //std::cout << weaponsToString(*weapons) << std::endl;


  };
  return binding;
}

