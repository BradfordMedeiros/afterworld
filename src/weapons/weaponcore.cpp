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

void registerGunType(std::string gunName){
	// query from sql
	// parse it into a  raw form
	// load the resources
	// then add it to the cores




  /*
  WeaponParams weaponParams {
  	float firingRate;
  	float recoilLength;
  	float recoilPitchRadians;
  	glm::vec3 recoilTranslate;
  	glm::vec3 recoilZoomTranslate;
  	bool canHold;
  	bool isIronsight;
  	bool isRaycast;
  	glm::vec3 ironsightOffset;
  	glm::quat ironSightAngle;
	};
	*/
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

WeaponState changeGun(WeaponParams& weaponParams, WeaponState& weaponState, int ammo){
  WeaponState state {
    .lastShootingTime = -1.f * weaponParams.firingRate, // so you can shoot immediately
    .recoilStart = 0.f,
    .currentAmmo = ammo,
    .gunState = GUN_RAISED,
  };
  return state;
}

void changeGun(WeaponValues& weaponValues, std::string gun, int ammo, objid sceneId, objid playerId){
  weaponValues.weaponParams = queryWeaponParams(gun);
  modlog("weapons", std::string("spawn gun: ") + weaponValues.weaponParams.name);

  weaponValues.weaponState = changeGun(weaponValues.weaponParams, weaponValues.weaponState, ammo);
  if (weaponValues.weaponInstance.has_value()){
    removeWeaponInstance(weaponValues.weaponInstance.value());
  }
  weaponValues.weaponInstance = createWeaponInstance(weaponValues.weaponParams, sceneId, playerId);
  if (weaponValues.weaponParams.idleAnimation.has_value() && weaponValues.weaponParams.idleAnimation.value() != "" && weaponValues.weaponInstance.value().gunId){
    gameapi -> playAnimation(weaponValues.weaponInstance.value().gunId, weaponValues.weaponParams.idleAnimation.value(), LOOP);
  }
  gameapi -> sendNotifyMessage("current-gun", CurrentGunMessage {
    .currentAmmo = weaponValues.weaponState.currentAmmo,
    .totalAmmo = weaponValues.weaponParams.totalAmmo,
  });
}

void deliverAmmo(WeaponValues& weaponValues, int ammo){
  weaponValues.weaponState.currentAmmo += ammo;
  gameapi -> sendNotifyMessage("current-gun", CurrentGunMessage {
    .currentAmmo = weaponValues.weaponState.currentAmmo,
    .totalAmmo = weaponValues.weaponParams.totalAmmo,
  });
}

bool canFireGunNow(WeaponValues& weaponValues, float elapsedMilliseconds){
  auto timeSinceLastShot = elapsedMilliseconds - weaponValues.weaponState.lastShootingTime;
  bool lessThanFiringRate = timeSinceLastShot >= (0.001f * weaponValues.weaponParams.firingRate);
  return lessThanFiringRate;
}