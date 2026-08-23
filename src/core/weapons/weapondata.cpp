#include "./weapondata.h"

std::vector<std::string> listFilesWithExtensionsFromPackage(std::string folder, std::vector<std::string> extensions);
std::string readFileOrPackage(std::string filepath);

std::unordered_map<std::string, WeaponParams> weapons;

std::optional<glm::vec3> parseVec3(rapidjson::Value& object, const char* field){
  if (!object.IsObject()){
    return std::nullopt;
  }
  if (!object.HasMember(field)){
    return std::nullopt;
  }
  auto& value = object[field];
  if (!value.IsArray() || value.Size() != 3){
    return std::nullopt;
  }
  if (!value[0].IsNumber() || !value[1].IsNumber() || !value[2].IsNumber()){
    return std::nullopt;
  }
  return glm::vec3(value[0].GetFloat(), value[1].GetFloat(), value[2].GetFloat());
}

std::optional<glm::vec4> parseVec4(rapidjson::Value& object, const char* field){
  if (!object.IsObject()){
    return std::nullopt;
  }
  if (!object.HasMember(field)){
    return std::nullopt;
  }
  auto& value = object[field];
  if (!value.IsArray() || value.Size() != 4){
    return std::nullopt;
  }
  if (!value[0].IsNumber() || !value[1].IsNumber() || !value[2].IsNumber() || !value[3].IsNumber()){
    return std::nullopt;
  }
  return glm::vec4(value[0].GetFloat(), value[1].GetFloat(), value[2].GetFloat(), value[3].GetFloat());
}

WeaponParams parseWeaponJson(std::string filePath, std::string gunName){
  auto fileContent = readFileOrPackage(filePath);
  rapidjson::Document doc;
  rapidjson::ParseResult ok = doc.Parse(fileContent.c_str());
  if (doc.HasParseError()){
    std::cout << "error parsing weapon file: " << filePath << "  (" << fileContent << ")" << std::endl;
    exit(1);
  }

  WeaponParams weaponParams {};
  weaponParams.name = gunName;
  
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
    auto recoilTranslate = parseVec3(doc, "recoilTranslate");
    if (recoilTranslate.has_value()){
      weaponParams.recoilTranslate = recoilTranslate.value();
    }
  }

  {
    auto recoilZoomTranslate = parseVec3(doc, "recoilZoomTranslate");
    if (recoilZoomTranslate.has_value()){
      weaponParams.recoilZoomTranslate = recoilZoomTranslate.value();
    }
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
    auto ironsightOffset = parseVec3(doc, "ironsightOffset");
    if (ironsightOffset.has_value()){
      weaponParams.ironsightOffset = ironsightOffset.value();
    }
  }



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
    auto scale = parseVec3(doc, "scale");
    if (scale.has_value()){
      weaponParams.scale = scale.value();
    }
  }

  {
    auto initialGunPos = parseVec3(doc, "initialGunPos");
    if (initialGunPos.has_value()){
      weaponParams.initialGunPos = initialGunPos.value();
    }else {
      weaponParams.initialGunPos = glm::vec3(0.5f, -0.25f, -0.75f);
    }
  }

  {
    auto rot = parseVec4(doc, "gunrotation");
    if (rot.has_value()){
      weaponParams.initialGunRotVec4 = rot.value();
      weaponParams.initialGunRot = parseQuat(rot.value());      
    }else{
      auto rot4 = glm::vec4(0.1f, 0.f, -1.f, 0.f);
      weaponParams.initialGunRotVec4 = rot4;
      weaponParams.initialGunRot = parseQuat(rot4); 
    }

  }

  {
    auto rot = parseVec4(doc, "ironSightAngle");
    if (rot.has_value()){
      weaponParams.ironSightAngle = parseQuat(rot.value());      
    }
  }

  

  {
    auto it = doc.FindMember("fireAnimation");
    if (it != doc.MemberEnd() && it -> value.IsString()) {
      weaponParams.fireAnimation = it -> value.GetString();
    }    
  }

  {
    auto it = doc.FindMember("idleAnimation");
    if (it != doc.MemberEnd() && it -> value.IsString()) {
      weaponParams.idleAnimation = it -> value.GetString();
    }    
  }

  {
    auto it = doc.FindMember("muzzleParticle");
    if (it != doc.MemberEnd() && it->value.IsString()) {
      weaponParams.muzzleParticleStr = it->value.GetString();
    }
  }
  {
    auto it = doc.FindMember("hitParticle");
    if (it != doc.MemberEnd() && it->value.IsString()) {
      weaponParams.hitParticleStr = it->value.GetString();
    }
  }
  {
    auto it = doc.FindMember("projectileParticle");
    if (it != doc.MemberEnd() && it->value.IsString()) {
      weaponParams.projectileParticleStr = it->value.GetString();
    }
  }

  std::cout << "projectile: " << weaponParams.projectileParticleStr << std::endl;

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
   	auto fileInfo = decomposePath(gunFile);
  	auto weapon = parseWeaponJson(gunFile, fileInfo.filename);
  	weapons[fileInfo.filename] = weapon;
  }

}

WeaponParams getWeaponParamsByGunName(std::string gunName){
	return weapons.at(gunName);
}