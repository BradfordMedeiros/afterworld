#include "./weapon.h"

extern CustomApiBindings* gameapi;

struct WeaponParams {
  float firingRate;
  float recoilLength;
  float recoilPitchRadians;
  bool canHold;
  bool isIronsight;
  bool isRaycast;
};

struct CurrentGun {
  std::string name;
  std::optional<objid> gunId;
  std::optional<objid> soundId;

  float lastShootingTime;
};

struct Weapons {
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;
  WeaponParams weaponParams;

  CurrentGun currentGun;
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
    .stringAttributes = { { "clip", firesound }},
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

  weapons.currentGun.lastShootingTime = -1.f * weapons.weaponParams.firingRate ; // so you can shoot immediately


  auto soundpath = strFromFirstSqlResult(result, 2);
  auto modelpath = strFromFirstSqlResult(result, 0);
  auto gunpos = vec3FromFirstSqlResult(result, 3, 4, 5);
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

bool debugAssert = false;

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Weapons* weapons = new Weapons;

    weapons -> isHoldingLeftMouse = false;
    weapons -> isHoldingRightMouse = false;
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
        tryFireGun(*weapons, gameapi -> listSceneId(id));
      }else if (action == 1){
        weapons -> isHoldingLeftMouse = true;
      }
    }else if (button == 1){
      if (action == 0){
        weapons -> isHoldingRightMouse = false;
        modassert(!debugAssert, "should add ironsight");
      }else if (action == 1){
        weapons -> isHoldingRightMouse = true;
        modassert(!debugAssert, "should add ironsight");
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
    }
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);

    if (weapons -> weaponParams.canHold && weapons -> isHoldingLeftMouse){
      tryFireGun(*weapons, gameapi -> listSceneId(id));
    }
    //std::cout << weaponsToString(*weapons) << std::endl;

    //(if (and can-hold is-holding) (fire-gun))
    //(if gunid (sway-gun (should-zoom)))

  };
  return binding;
}

