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

  float lastShootingTime;
  float recoilStart;
  glm::vec3 initialGunPos;
};

struct Weapons {
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;

  WeaponParams weaponParams;
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

    std::cout << "save gun, name = " << weapons.currentGun.name << ",  pos = " << print(position) << ", scale = " << print(scale) << ", rotation = " << print(rotation) << std::endl;

    auto updateQuery = gameapi -> compileSqlQuery(
      std::string("update guns set ") +
        "xoffset-pos = " + serializeFloat(position.x) + ", " +
        "yoffset-pos = " + serializeFloat(position.y) + ", " +
        "zoffset-pos = " + serializeFloat(position.z) + ", " + 
        "xrot = " + serializeFloat(rotation.x) + ", " +
        "yrot = " + serializeFloat(rotation.y) + ", " +
        "zrot = " + serializeFloat(rotation.z) + 
        " where name = " + gun
    );
    bool validSql = false;
    auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
    modassert(validSql, "error executing sql query");
  }
}

std::string parentName = ">maincamera";

GameobjAttributes particleAttributes(std::string& particle){

/*(define (template sourceString template templateValue)
  (define index (string-contains sourceString template))
  (if index
    (string-replace sourceString templateValue index (+ index (string-length template)))
    sourceString
  )
)

(define reserved-emitter-chars (list "%" "!" "?"))
(define (reserved-emitter-name value) (member (substring value 0 1) reserved-emitter-chars))
(define (template-emit-line line) 
  (define keyname (car line))
  (define value (template (cadr line) "$MAINOBJ" parent-name))
  (define reservedKeyName (reserved-emitter-name keyname))
  (if reservedKeyName
    (list keyname value)
    (list keyname (parse-attr value))
  )
)
(define (split-emit-line emitLine) (template-emit-line (string-split emitLine #\:)))
(define (is-not-whitespace val) (not (equal? (string-trim val) "")))
(define (emitterOptsToList emitterOptions) (map split-emit-line (filter is-not-whitespace (string-split emitterOptions #\;))))
*/

  GameobjAttributes particleAttr {
    .stringAttributes = { 
      { "state", "disabled" },    // default should keep
      { "physics", "disabled" },  
      { "layer", "no_depth" },    ///////
     
      { "+mesh", "../gameresources/build/primitives/plane_xy_1x1.gltf" },
      { "+texture", "../gameresources/textures/evilpattern.png" },
      { "+shader", "../afterworld/shaders/discard/fragment.glsl" },
    },
    .numAttributes = { { "duration", 10.f } },
    .vecAttr = {  
      .vec3 = {{ "+scale", glm::vec3(0.2f, 0.2f, 0.2f) }},  
      .vec4 = {{ "+tint", glm::vec4(1.f, 1.f, 0.f, 0.8f) }}  // this shouldn't be needed for the default plane since 0 0 -1 0 should make the plane face the player
    },
  };
  return particleAttr;
}

void spawnGun(Weapons& weapons, objid sceneId, std::string name, std::string firesound, std::string particle, std::string hitParticle, std::string modelpath, std::string script, glm::vec3 gunpos, glm::vec4 rot, glm::vec3 scale){
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
    particleAttr.stringAttributes["+lookat"] = ">maincamera";
    weapons.currentGun.muzzleParticle = gameapi -> makeObjectAttr(sceneId, "+code-muzzleparticles", particleAttr, submodelAttributes);
  }
  if (hitParticle != ""){
    auto particleAttr = particleAttributes(hitParticle);
    particleAttr.stringAttributes["+physics"] = "enabled";
    particleAttr.stringAttributes["+physics_type"] = "dynamic";
    particleAttr.vecAttr.vec3["+scale"] = glm::vec3(0.2, 0.2, 0.2f);
    particleAttr.vecAttr.vec4["+tint"] = glm::vec4(1.f, 0.f, 0.f, 0.6f);
    weapons.currentGun.hitParticles = gameapi -> makeObjectAttr(sceneId, "+code-hitparticle", particleAttr, submodelAttributes);
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
}

void changeGun(Weapons& weapons, objid sceneId, std::string gun){
  auto gunQuery = gameapi -> compileSqlQuery(
   std::string("select modelpath, fire-animation, fire-sound, xoffset-pos, ") +
   "yoffset-pos, zoffset-pos, xrot, yrot, zrot, xscale, yscale, zscale, " + 
   "firing-rate, hold, raycast, ironsight, iron-xoffset-pos, iron-yoffset-pos, " + 
   "iron-zoffset-pos, particle, hit-particle, recoil-length, recoil-angle, " + 
   "recoil, recoil-zoom, projectile, bloom, script " + 
   "from guns where name = " + gun
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(gunQuery, &validSql);
  modassert(validSql, "error executing sql query");

  modassert(result.size() > 0, "no gun named: " + gun);
  modassert(result.size() == 1, "more than one gun named: " + gun);
  std::cout << "gun: result: " << print(result.at(0)) << std::endl;

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

  auto gunpos = vec3FromFirstSqlResult(result, 3, 4, 5);
  weapons.currentGun.initialGunPos = gunpos;

  auto soundpath = strFromFirstSqlResult(result, 2);
  auto modelpath = strFromFirstSqlResult(result, 0);
  auto script = strFromFirstSqlResult(result, 27);

  auto rot3 = vec3FromFirstSqlResult(result, 6, 7, 8);
  auto rot4 = glm::vec4(rot3.x, rot3.y, rot3.z, 01.f);
  auto scale = vec3FromFirstSqlResult(result, 9, 10, 11);

  auto muzzleParticleStr = "+mesh:../gameresources/build/primitives/plane_xy_1x1.gltf;+texture:./res/textures/wood.jpg";
  auto hitParticleStr = "+mesh:../gameresources/build/primitives/plane_xy_1x1.gltf;+texture:./res/textures/wood.jpg";
  spawnGun(weapons, sceneId, gun, soundpath, muzzleParticleStr, hitParticleStr, modelpath, script, gunpos, rot4, scale);
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

  std::cout << "fire raycast, total hits = " << hitpoints.size() << std::endl;
  for (auto &hitpoint : hitpoints){
    std::cout << "raycast hit: " << hitpoint.id << "- point: " << print(hitpoint.point) << ", normal: " << print(hitpoint.normal) << std::endl;
    if (weapons.currentGun.hitParticles.has_value()){
      gameapi -> emit(weapons.currentGun.hitParticles.value(), zFightingForParticle(hitpoint.point, hitpoint.normal), hitpoint.normal, std::nullopt, std::nullopt);
    }
    std::cout << "raycast normal: " << serializeQuat(hitpoint.normal) << std::endl;
  }
}

void tryFireGun(Weapons& weapons, objid sceneId){
  float now = gameapi -> timeSeconds(false);
  auto canFireGun = canFireGunNow(weapons, now);

  std::cout << "try fire gun, can fire = " << canFireGun << ", now = " << now << ", firing rate = " << weapons.weaponParams.firingRate << std::endl;
  if (!canFireGun){
    return;
  }
  if (weapons.currentGun.soundId.has_value()){
    gameapi -> playClip("&code-weaponsound", sceneId);
  }
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


  /*
  (define elapsedMilliseconds (* 1000 (time-seconds)))
  (define timeSinceLastShot (- elapsedMilliseconds last-shooting-time))
  (define lessThanFiringRate (> timeSinceLastShot firing-rate))
  (if lessThanFiringRate 
    (begin
      (fire-gun)
      (set! last-shooting-time elapsedMilliseconds)
    )
  )
  (format #f "fire gun limited placeholder\n")*/

  /*(define (fire-gun)
  (define objpos (gameobj-pos (lsobj-name parent-name)))
  (define objrot (gameobj-rot (lsobj-name parent-name)))
  (if (not (equal? fire-animation #f))
    (gameobj-playanimation (gameobj-by-id gunid) fire-animation)
  )
  (if soundid (playclip "&weapon-sound"))
  (if emitterid (emit emitterid))
  (if projectile-emitterid 
    (emit projectile-emitterid 
      (move-relative objpos objrot 3)
      objrot
      (addvecs 
        (move-relative (list 0 0 0) (quatmult objrot (orientation-from-pos (list 0 0 0) (list 0 0.2 -1))) 25)
        velocity
      )
    )
  )
  (if is-raycast (fire-raycast))
  (start-recoil)
  )*/

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
          gameapi -> sendNotifyMessage("selected", std::to_string(hitpoints.at(0).id));
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

