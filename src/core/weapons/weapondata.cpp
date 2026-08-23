#include "./weapondata.h"

std::vector<std::string> listFilesWithExtensionsFromPackage(std::string folder, std::vector<std::string> extensions);
std::string readFileOrPackage(std::string filepath);


std::vector<WeaponParams> weaponData;


WeaponParams parseWeaponJson(std::string filePath){
  auto fileContent = readFileOrPackage(filePath);
  rapidjson::Document doc;
  rapidjson::ParseResult ok = doc.Parse(fileContent.c_str());
  if (doc.HasParseError()){
    std::cout << "error parsing weapon file: " << filePath << "  (" << fileContent << ")" << std::endl;
    exit(1);
  }

  WeaponParams weaponParams {};
  {
    auto it = doc.FindMember("name");
    if (it != doc.MemberEnd() && it -> value.IsString()) {
      weaponParams.name = it -> value.GetString();
    }else{
    	modassert(false, std::string("weapon config must have a name: ") + filePath);
    }   
  }

  {
    auto it = doc.FindMember("firingRate");
    if (it != doc.MemberEnd() && it -> value.IsFloat()) {
      weaponParams.firingRate = it -> value.GetFloat();
    } 
  }

  {
    auto it = doc.FindMember("recoilLength");
    if (it != doc.MemberEnd() && it -> value.IsFloat()) {
      weaponParams.recoilLength = it -> value.GetFloat();
    } 
  }

  {
    auto it = doc.FindMember("recoilPitchRadians");
    if (it != doc.MemberEnd() && it -> value.IsFloat()) {
      weaponParams.recoilPitchRadians = it -> value.GetFloat();
    } 
  }


  {
    weaponParams.recoilTranslate = glm::vec3(0.f, 0.f, 0.f);
    //auto it = doc.FindMember("recoilTranslate");
    //if (it != doc.MemberEnd() && it -> value.IsFloat()) {
    //  weaponParams.recoilPitchRadians = it -> value.GetFloat();
    //} 
  }

  {
  	  weaponParams.recoilZoomTranslate = glm::vec3(0.f, 0.f, 1.f);
  }

  {
    auto it = doc.FindMember("canHold");
    if (it != doc.MemberEnd() && it -> value.IsBool()) {
      weaponParams.canHold = it -> value.GetBool();
    } 
  }

  {
    auto it = doc.FindMember("isIronsight");
    if (it != doc.MemberEnd() && it -> value.IsBool()) {
      weaponParams.isIronsight = it -> value.GetBool();
    } 
  }

  {
    auto it = doc.FindMember("isRaycast");
    if (it != doc.MemberEnd() && it -> value.IsBool()) {
      weaponParams.isRaycast = it -> value.GetBool();
    } 
  }

  {
	  weaponParams.ironsightOffset = glm::vec3(-2.f, 0.f, 0.f);

  }
  //auto fileContent = readFileOrPackage(BALL_CONFIG);

  {
    auto it = doc.FindMember("minBloom");
    if (it != doc.MemberEnd() && it -> value.IsFloat()) {
      weaponParams.minBloom = it -> value.GetFloat();
    } 
  }

  {
    auto it = doc.FindMember("totalBloom");
    if (it != doc.MemberEnd() && it -> value.IsFloat()) {
      weaponParams.totalBloom = it -> value.GetFloat();
    }
  }

  {
    auto it = doc.FindMember("bloomLength");
    if (it != doc.MemberEnd() && it -> value.IsFloat()) {
      weaponParams.bloomLength = it -> value.GetFloat();
    }
  }

  {
    auto it = doc.FindMember("totalAmmo");
    if (it != doc.MemberEnd() && it -> value.IsInt()) {
      weaponParams.totalAmmo = it -> value.GetInt();
    }
  }

  {
    auto it = doc.FindMember("damage");
    if (it != doc.MemberEnd() && it -> value.IsFloat()) {
      weaponParams.damage = it -> value.GetFloat();
    }
  }

  {
    auto it = doc.FindMember("modelpath");
    if (it != doc.MemberEnd() && it -> value.IsString()) {
      weaponParams.modelpath = it -> value.GetString();
    }  
  }

  {
    auto it = doc.FindMember("soundpath");
    if (it != doc.MemberEnd() && it -> value.IsString()) {
      weaponParams.soundpath = it -> value.GetString();
    }  
  }

  {
  	weaponParams.initialGunPos = glm::vec3(0.5f, -0.25f, -0.75);
  }

  {
  	auto rot4 = glm::vec4(0.1f, 0.f, -1.f, 0.f);
  	weaponParams.initialGunRotVec4 = rot4;
  	weaponParams.initialGunRot = parseQuat(rot4);
  }

  {
 	 weaponParams.scale = glm::vec3(1.f, 1.f, 1.f);
  }

/*
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
  */

  return weaponParams;
}

void initWeaponsFromConfig(){
  auto gunFiles = listFilesWithExtensionsFromPackage("../afterworld/data/config/fps/guns", { "json" });
  std::cout << "guns: ";
  for (auto& gunFile : gunFiles){
    std::cout << gunFile << " ";

  }
  std::cout << std::endl;

  for (auto& gunFile : gunFiles){
  	auto fileInfo = decomposePath(gunFile);
  	auto relativeDir = relativePath("../afterworld/data/config/fps/guns", fileInfo.dirPath, ".");
  	auto relativeDirVec = split(relativeDir, '/');
  	std::cout << print(fileInfo) << std::endl;
  	std::cout << relativeDir << ", " << print(relativeDirVec) << std::endl;
  }

  for (auto& gunFile : gunFiles){
  	parseWeaponJson(gunFile);
  }

  //exit(1);
}

WeaponParams getWeaponParamsByGunName(std::string gunName){
	return parseWeaponJson("../afterworld/data/config/fps/guns/shotgun.json");
}