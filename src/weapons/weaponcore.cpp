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

  weaponParams.ironSightAngle = result.at(0).at(32) == "" ? weaponParams.initialGunRot : quatFromFirstSqlResult(result, 32);

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

