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
void spawnGun(Weapons& weapons, objid sceneId, std::string name, std::string modelpath, glm::vec3 gunpos, glm::vec4 rot, glm::vec3 scale){
  if (weapons.currentGun.gunId.has_value()){
    gameapi -> removeObjectById(weapons.currentGun.gunId.value());
    weapons.currentGun.name = "";
    weapons.currentGun.gunId = std::nullopt;
  }

  GameobjAttributes attr {
    .stringAttributes = {
      { "mesh", modelpath },
      { "layer", "no_depth" },
    },
    .numAttributes = {
    },
    .vecAttr = { 
      .vec3 = {
        { "position", gunpos },
        { "scale", scale },
      }, 
      .vec4 = { 
        { "rotation", rot },
      } 
    },
  };

  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto gunId = gameapi -> makeObjectAttr(sceneId, "weapon", attr, submodelAttributes);
  modassert(gunId.has_value(), "gun does not have a value");
  weapons.currentGun.gunId = gunId;
  weapons.currentGun.name = name;
  auto parent = gameapi -> getGameObjectByName(parentName, sceneId, false);
  modassert(parent.has_value(), parentName + " does not exist in scene so cannot create gun");
  gameapi -> makeParent(gunId.value(), parent.value());

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

  auto modelpath = strFromFirstSqlResult(result, 0);
  auto gunpos = vec3FromFirstSqlResult(result, 3, 4, 5);
  auto rot3 = vec3FromFirstSqlResult(result, 6, 7, 8);
  auto rot4 = glm::vec4(rot3.x, rot3.y, rot3.z, 01.f);
  auto scale = vec3FromFirstSqlResult(result, 9, 10, 11);
  spawnGun(weapons, sceneId, gun, modelpath, gunpos, rot4, scale);
  /*
    (let* ((guninfo (car gunstats)) (bloomVec (parse-stringvec (list-ref guninfo 26))))

      (set-animation     (list-ref guninfo 1))

      ; Firing Parameters

      (set! ironsight-offset (list 
        (string->number (list-ref guninfo 16))
        (string->number (list-ref guninfo 17))
        (string->number (list-ref guninfo 18))
      ))
      (set! recoilTranslate (parse-stringvec (list-ref guninfo 23)))
      (set! recoilZoomTranslate (parse-stringvec (list-ref guninfo 24)))
      (set! bloom-radius (list-ref bloomVec 0))

      (format #t "traits: firing-params - rate: ~a, can-hold: ~a, raycast: ~a, ironsight: ~a, ironsight-offset: ~a\n" firing-rate can-hold is-raycast is-ironsight ironsight-offset)
      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

      (set! last-shooting-time initFiringTime)
      (change-sound (list-ref guninfo 2))
      (change-emitter 
        (list-ref guninfo 19)
        (list-ref guninfo 20)
        (list-ref guninfo 25)
      )
      (change-script (list-ref guninfo 27))
 
  )*/
}

std::string weaponsToString(Weapons& weapons){
  std::string str;
  str += std::string("isHoldingLeftMouse: ") + (weapons.isHoldingLeftMouse ? "true" : "false") + "\n";
  str += std::string("isHoldingRightMouseo: ") + (weapons.isHoldingRightMouse ? "true" : "false") + "\n";
  return str;
}

void tryFireGun(Weapons& weapons){
  std::cout << "try fire gun" << std::endl;
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
    };

    changeGun(*weapons, gameapi -> listSceneId(id), "pistol");

  	return weapons;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    delete weapons;
  };
  binding.onMouseCallback = [](objid scriptId, void* data, int button, int action, int mods) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    if (button == 0){
      if (action == 0){
        weapons -> isHoldingLeftMouse = false;
        tryFireGun(*weapons);
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
    //std::cout << weaponsToString(*weapons) << std::endl;
  };
  return binding;
}

