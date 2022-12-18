#include "./weapon.h"

extern CustomApiBindings* gameapi;

struct WeaponParams {
  float firingRate;
  float recoilLength;
  float recoilPitchRadians;
  bool canHold;
  bool isIronsight;
  bool isRaycast;
  glm::vec3 ironsightOffset;
};

struct CurrentGun {
  std::string name;
  std::optional<objid> gunId;
  std::optional<objid> soundId;

  float lastShootingTime;
  glm::vec3 initialGunPos;
};

struct Weapons {
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;

  WeaponParams weaponParams;
  CurrentGun currentGun;

  glm::vec2 lookVelocity;
  float movementVelocity;
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

void spawnGun(Weapons& weapons, objid sceneId, std::string name, std::string firesound, std::string modelpath, glm::vec3 gunpos, glm::vec4 rot, glm::vec3 scale){
  if (weapons.currentGun.gunId.has_value()){
    weapons.currentGun.name = "";

    gameapi -> removeObjectById(weapons.currentGun.gunId.value());
    weapons.currentGun.gunId = std::nullopt;

    if (weapons.currentGun.soundId.has_value()){
      gameapi -> removeObjectById(weapons.currentGun.soundId.value());
      weapons.currentGun.soundId = std::nullopt;
    }
  }

  GameobjAttributes attr {
    .stringAttributes = { { "mesh", modelpath }, { "layer", "no_depth" }},
    .numAttributes = {},
    .vecAttr = {  .vec3 = {{ "position", gunpos }, { "scale", scale }},  .vec4 = {{ "rotation", rot }}},
  };

  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto gunId = gameapi -> makeObjectAttr(sceneId, "code-weapon", attr, submodelAttributes);
  modassert(gunId.has_value(), "gun does not have a value");
  weapons.currentGun.gunId = gunId;

  GameobjAttributes soundAttr {
    .stringAttributes = { { "clip", firesound } },
    .numAttributes = {},
    .vecAttr = {  .vec3 = {},  .vec4 = {} },
  };

  if (firesound != ""){
    auto soundId = gameapi -> makeObjectAttr(sceneId, "&code-weaponsound", soundAttr, submodelAttributes);
    weapons.currentGun.soundId = soundId;  
  }


  weapons.currentGun.name = name;
  auto parent = gameapi -> getGameObjectByName(parentName, sceneId, false);
  modassert(parent.has_value(), parentName + " does not exist in scene so cannot create gun");
  gameapi -> makeParent(gunId.value(), parent.value());
  if (weapons.currentGun.soundId.has_value()){
    gameapi -> makeParent(weapons.currentGun.soundId.value(), parent.value());
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
  weapons.weaponParams.canHold = boolFromFirstSqlResult(result, 13);
  weapons.weaponParams.isIronsight = boolFromFirstSqlResult(result, 15);
  weapons.weaponParams.isRaycast = boolFromFirstSqlResult(result, 14);
  weapons.weaponParams.ironsightOffset = vec3FromFirstSqlResult(result, 16, 17, 18);

  weapons.currentGun.lastShootingTime = -1.f * weapons.weaponParams.firingRate ; // so you can shoot immediately

  auto gunpos = vec3FromFirstSqlResult(result, 3, 4, 5);
  weapons.currentGun.initialGunPos = gunpos;

  auto soundpath = strFromFirstSqlResult(result, 2);
  auto modelpath = strFromFirstSqlResult(result, 0);
  auto rot3 = vec3FromFirstSqlResult(result, 6, 7, 8);
  auto rot4 = glm::vec4(rot3.x, rot3.y, rot3.z, 01.f);
  auto scale = vec3FromFirstSqlResult(result, 9, 10, 11);

  spawnGun(weapons, sceneId, gun, soundpath, modelpath, gunpos, rot4, scale);
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
  weapons.currentGun.lastShootingTime = now;
  

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
  std::cout << "sway velocity: " << print(newPos) << std::endl;
  return newPos;
}

glm::vec3 calcLocationWithRecoil(Weapons& weapons, glm::vec3 targetPos, bool isGunZoomed){
  if (isGunZoomed){
    return weapons.weaponParams.ironsightOffset;
  }

/*
  (define recoilAmount (if zoomgun recoilZoomTranslate recoilTranslate))
  (define targetposWithRecoil (list 
    (+ (car targetpos)   (car recoilAmount))
    (+ (cadr targetpos)  (cadr recoilAmount)) 
    (+ (caddr targetpos) (caddr recoilAmount))
  ))
  ;(format #t "targetpos: ~a\n" targetposWithRecoil)
  (lerp targetpos targetposWithRecoil (calc-recoil-slerpamount))*/
  debugAssertForNow(false, "recoil location not yet implemented");
  return targetPos;
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

    weapons -> currentGun = CurrentGun {
      .name = "",
      .gunId = std::nullopt,
      .soundId = std::nullopt,
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
        debugAssertForNow(false, "should add ironsight");
      }else if (action == 1){
        weapons -> isHoldingRightMouse = true;
        debugAssertForNow(false, "should add ironsight");
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
    std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
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

