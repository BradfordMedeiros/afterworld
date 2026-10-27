#include "./materials.h"

extern CustomApiBindings* gameapi;

std::optional<glm::vec2> parseVec2(rapidjson::Value& object, const char* field);
std::optional<glm::vec4> parseVec4(rapidjson::Value& object, const char* field);

std::optional<std::string> materialTypeForObj(objid id){
  auto attr = getAttrHandle(id);
  return getStrAttr(attr, "material");
}

std::optional<MaterialToParticle*> getHitMaterial(std::vector<MaterialToParticle>& materials, std::string& materialName){
  for (auto &material : materials){
    if (material.material == materialName){
      return &material;
    }
  }
  return std::nullopt;
}

GameobjAttributes particleAttributes(std::string& particle){
  auto templateEmitLine = split(particle, ';');

  GameobjAttributes particleAttr {
    .attr = { 
      { "state", "disabled" },    // default should keep
      { "physics", "disabled" },  
      //{ "layer", "no_depth" },    ///////
      { "duration", 10.f }
    },
  };

  for (auto &line : templateEmitLine){
    auto keyValuePair = split(line, ':');
    modassert(keyValuePair.size() == 2, "invalid emitter particle attr, line: " + line + ", size = " + std::to_string(keyValuePair.size()));
    addFieldDynamic(particleAttr, keyValuePair.at(0), keyValuePair.at(1));  // should probably use only explicitly allowed api methods
  }
  return particleAttr;
}


MaterialToParticle parseMaterial(objid sceneId, std::string filepath){
  auto fileInfo = decomposePath(filepath);
  auto relativeDir = relativePath("../afterworld/data/config/materials/ ", fileInfo.dirPath, ".");
  auto relativeDirVec = split(relativeDir, '/');
  std::string name = fileInfo.filename;

  std::cout << "material: parseMaterial: " << filepath << ", name = " << name << std::endl;


  MaterialToParticle materialToParticle {
    .material = name,

    // .hitParticleClipId;
    // .hitParticle;  // wherever gun hits, gets parented to the surface
    // .splashParticle;  // same as hit particle, but does not get parented.
  };

  auto fileContent = readFileOrPackage(filepath);
  rapidjson::Document doc;
  rapidjson::ParseResult ok = doc.Parse(fileContent.c_str());
  if (doc.HasParseError()){
    std::cout << "error parsing game file: " << filepath << "  (" << fileContent << ")" << std::endl;
    modassert(false, "parseMaterial failed");
  }

  {
    auto collisionParticleClip = doc.FindMember("collision-clip");
    if (collisionParticleClip != doc.MemberEnd() && collisionParticleClip -> value.IsString()) {
      materialToParticle.collisionSound = collisionParticleClip -> value.GetString();
      std::cout << "material: parseMaterial: particle material set hit-clip = " << print(materialToParticle.collisionSound) << std::endl;
    }       
  }

  {
    // hitDecal
    auto hitDecal = doc.FindMember("hit-decal");
    if (hitDecal != doc.MemberEnd() && hitDecal -> value.IsObject()) {
      auto textureValue = hitDecal -> value.FindMember("texture");
      auto size = parseVec2(hitDecal -> value, "size");
      auto tint = parseVec4(hitDecal -> value, "tint");
      std::cout << "hit decal size: " << print(size) << std::endl;
      materialToParticle.hitDecal = HitDecal {
        .texture = textureValue -> value.IsString() ? textureValue -> value.GetString() : "./res/textures/wood.jpg",
        .size = size.has_value() ? size.value() : glm::vec2(1.f, 1.f),
        .tint = tint.has_value() ? tint.value() : glm::vec4(1.f, 1.f, 1.f, 1.f),
      };
    }    
  }
  {
    auto hitParticleClip = doc.FindMember("hit-clip");
    if (hitParticleClip != doc.MemberEnd() && hitParticleClip -> value.IsString()) {
      materialToParticle.hitParticleClip = hitParticleClip -> value.GetString();
      std::cout << "material: parseMaterial: particle material set hit-clip = " << print(materialToParticle.hitParticleClip) << std::endl;
    }    
  }


  {
    auto splashParticle = doc.FindMember("splash-particle");
    if (splashParticle != doc.MemberEnd() && splashParticle -> value.IsString()) {
      materialToParticle.splashParticle = splashParticle -> value.GetString();
      std::cout << "material: parseMaterial: particle material set materialToParticle.materialToParticle.splashParticle = " << print(materialToParticle.splashParticle) << std::endl;
    }    
  }

  return materialToParticle;
}

std::vector<MaterialToParticle> loadMaterials(objid sceneId){
  std::vector<MaterialToParticle> materialToParticles;
  auto materialFiles = listFilesWithExtensionsFromPackage("../afterworld/data/config/materials", { "json" });
  for (auto &materialFile : materialFiles){
    auto material = parseMaterial(sceneId, materialFile);
    materialToParticles.push_back(material);
  }
  return materialToParticles;
}

std::optional<objid> createParticleEmitter(objid sceneId, std::string& particleStr, std::string emitterName){
	if (particleStr == ""){
		return std::nullopt;
	}

  modlog("particle", particleStr);
	std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto particleAttr = particleAttributes(particleStr);
  auto particleId = gameapi -> makeObjectAttr(sceneId, emitterName, particleAttr, submodelAttributes);
  modassert(particleId.has_value(), "create particle emitter - could not create emitter (probably duplicate name)");
  return particleId;
}

std::vector<MaterialToParticle> materials;
void loadAllMaterials(objid rootSceneId){
  materials = loadMaterials(rootSceneId);
}

std::vector<MaterialToParticle>& getMaterials(){
  return materials;
}


void onCollisionEnterSound(int32_t sceneId, int32_t obj1, int32_t obj2, glm::vec3 pos){
  for (auto& material : materials){
    auto materialForObj1 = materialTypeForObj(obj1);
    auto materialForObj2 = materialTypeForObj(obj2);

    std::string materialName = "default";
    if (materialForObj1.has_value()){
      materialName = materialForObj1.value();
    }else if (materialForObj2.has_value()){
      materialName = materialForObj2.value();
    }

    if (material.material == materialName){ // obvously this shouldnt be hardcoded to this
      if (material.collisionSound.has_value()){
        playMixedSound(getSymbol(material.collisionSound.value()), pos);
      }
      return;
    }
  }
}

objid createEffect(std::string path, std::optional<glm::vec3> scale, std::optional<glm::vec4> tint){
    auto sceneId = gameapi -> rootSceneId();
    GameobjAttributes emitterAttr { 
      .attr = {
        { "effekseer", path },
        //{ "state", "enabled" },
      } 
    };
    if (scale.has_value()){
      emitterAttr.attr["scale"] = scale.value();
    }
    if (tint.has_value()){
      emitterAttr.attr["effect-tint"] = tint.value();
    }
    std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
    std::string emitterName = std::string("+emitter-") + std::to_string(getUniqueObjId());
    auto emitter = gameapi -> makeObjectAttr(sceneId, emitterName, emitterAttr, submodelAttributes);
    return emitter.value();
}


struct EffekData {
  std::string name;
  std::string path;
  std::optional<objid> id;
  std::optional<glm::vec3> scale;
  std::optional<glm::vec4> tint;
};
std::unordered_map<std::string, EffekData> effects {
    { "electric", EffekData { .name = "electric", .path  = "./res/particles/electric.efkefc", .scale = glm::vec3(0.2f, 0.2f, 0.2f)  }},
    { "killeffect", EffekData { .name = "killeffect", .path  = "./res/particles/backgrounds.efkefc", .scale = glm::vec3(0.2f, 0.2f, 0.2f)  }},
    { "warp", EffekData { .name = "warp",  .path  = "./res/particles/warp.efkefc", .scale = glm::vec3(0.4f, 0.4f, 0.4f)  }},
   
    { "rain", EffekData { .name = "rain", .path  = "./res/particles/rain2.efkefc", .scale = glm::vec3(0.4f, 0.4f, 0.4f) }},
    { "splash", EffekData { .name = "splash", .path  = "./res/particles/splash.efkefc" }},


    { "fire", EffekData { .name = "fire", .path  = "./res/particles/fire.efkefc", .scale = glm::vec3(0.4f, 0.4f, 0.4f) }},
    { "fog", EffekData { .name = "fog",  .path  = "./res/particles/fog.efkefc", .scale = glm::vec3(6.f, 6.f, 6.f) }},

    { "sparks", EffekData { .name = "sparks", .path  = "./res/particles/sparks.efkefc" , .scale = glm::vec3(0.1f, 0.1f, 0.1f) }},
    { "firework", EffekData { .name = "firework", .path  = "./res/particles/firework.efkefc"  }},
};


struct ParticleEmitters {
  std::vector<ParticleAndEmitter> coreEmitters;
  std::vector<EffekData> effekseerEmitters;
};

ParticleEmitters particleEmitters;

typedef std::variant<ParticleAndEmitter, EffekData> ParsedEmitterData;

ParsedEmitterData parseParticleEmitter(objid rootSceneId, std::string filepath){
  auto fileContent = readFileOrPackage(filepath);
  rapidjson::Document doc;
  rapidjson::ParseResult ok = doc.Parse(fileContent.c_str());
  if (doc.HasParseError()){
    std::cout << "error parsing game file: " << filepath << "  (" << fileContent << ")" << std::endl;
    modassert(false, "parseParticleEmitter failed");
  }

  auto fileInfo = decomposePath(filepath);
  auto relativeDir = relativePath("../afterworld/data/config/particles/ ", fileInfo.dirPath, ".");
  auto relativeDirVec = split(relativeDir, '/');
  std::string name = fileInfo.filename;

  std::cout << "particle: parseParticleEmitter: " << filepath << ", name = " << name << std::endl;

  ParticleAndEmitter particle {
    .particle = name,
  };



  {
    auto typeStr = doc.FindMember("type");
    if (typeStr != doc.MemberEnd() && typeStr -> value.IsString()){
      std::string type = typeStr -> value.GetString();
      if (type == "default"){
        // do nothing
      }else if (type == "effekseer"){
        auto effekseerFilePath = doc.FindMember("particle");
        modassert(effekseerFilePath -> value.IsString(), "particle field must be defined for partseParticleEmitter");
        std::string filePath = effekseerFilePath -> value.GetString();
        EffekData effekData { .name = name,  .path  = filePath  };
        return effekData;
      }else{
        modassert(false, std::string("parseParticleEmitter invalid type: ") + typeStr -> value.GetString());
      }
    }

  }
  
  {
    bool hasParticleStr = false;
    {
      auto particleStr = doc.FindMember("particle");
      if (particleStr != doc.MemberEnd() && particleStr -> value.IsString()) {
        std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
        std::string particleStrValue = particleStr -> value.GetString();
        auto particleAttr = particleAttributes(particleStrValue);
        auto particleEmitterId = gameapi -> makeObjectAttr(rootSceneId, "+code-particle-" + std::to_string(getUniqueObjId()), particleAttr, submodelAttributes);
        particle.particleId = particleEmitterId.value();
        hasParticleStr = true;
      }       
    }
    modassert(hasParticleStr, std::string("particle does not have a particlestr defined in config: ") + particle.particle);    
  }

  return particle;
}
ParticleEmitters loadParticleEmittersFromConfig(objid sceneId){
  ParticleEmitters particleEmitters;

  auto particleEmitterFiles = listFilesWithExtensionsFromPackage("../afterworld/data/config/particles/", { "json" });
  for (auto &particleEmitterFile : particleEmitterFiles){
    auto particleEmitterVariant = parseParticleEmitter(sceneId, particleEmitterFile);
    auto particleEmitterPtr = std::get_if<ParticleAndEmitter>(&particleEmitterVariant);
    auto effekDataPtr = std::get_if<EffekData>(&particleEmitterVariant);

    if (particleEmitterPtr){
      particleEmitters.coreEmitters.push_back(*particleEmitterPtr);
    }else if (effekDataPtr){
      particleEmitters.effekseerEmitters.push_back(*effekDataPtr);
    }else{
      modassert(false, "load particle emitter invalid type");
    }
  } 

  return particleEmitters;
}

void loadParticleEmitters(objid rootSceneId){
  auto particles = loadParticleEmittersFromConfig(rootSceneId);
  particleEmitters = particles;
}




std::optional<objid> getParticleEmitterCore(std::string& emitterName){
  for (auto &emitter : particleEmitters.coreEmitters){
    if (emitter.particle == emitterName){
      return emitter.particleId;
    }
  }
  return std::nullopt;
}

std::optional<EffekData*> getParticleEmitterEffek(std::string& emitterName){
  for (auto &emitter : particleEmitters.effekseerEmitters){
    if (emitter.name == emitterName){
      return &emitter;
    }
  }
  return std::nullopt;
}

int PARTICLE_MASK = 4;

std::optional<objid> bloodEmitter = std::nullopt;
void emitBlood(objid sceneId, objid lookAtId, glm::vec3 position){
  modlog("blood", "blood emitter");
  //modassert(false, "emit blood not yet implemented");
  static bool callOnce = true;
  if (callOnce){
    callOnce = false;
    std::string particleStr("+lookat:enabled;limit:100;+scale:0.3 0.3 0.3;+mesh:../gameresources/build/primitives/plane_xy_1x1.gltf;+physics_layer:4;+physics:enabled;+physics_type:dynamic;+tint:1 0 0 1;+physics_gravity:0 -9.10 0;+physics_collision:nocollide;+texture:../gameresources/textures/particles/blood.png;+normal-texture:../gameresources/textures/particles/blood.normal.png");
    particleStr += std::to_string(lookAtId);
    modlog("blood", particleStr);

    auto playerName = gameapi -> getGameObjNameForId(lookAtId);
    modassert(playerName.has_value(), "player name does not have a value...");
    bloodEmitter = createParticleEmitter(sceneId, particleStr, "+blood-emitter");
  }

  auto mainObjPos = gameapi -> getGameObjectPos(lookAtId, true, "[gamelogic] emitBlood - get lookat pos");

  auto orientation1 = gameapi -> orientationFromPos(position, mainObjPos);
  gameapi -> emit(bloodEmitter.value(), position, orientation1, std::nullopt, std::nullopt, std::nullopt);

  auto position2 = position + glm::vec3(0.2f, 0.f, 0.f);
  auto orientation2 = gameapi -> orientationFromPos(position2, mainObjPos);
  gameapi -> emit(bloodEmitter.value(), position2, orientation2, std::nullopt, std::nullopt, std::nullopt);
  
  auto position3 =  position + glm::vec3(-0.2f, 0.f, 0.f);
  auto orientation3 = gameapi -> orientationFromPos(position3, mainObjPos);

  auto velocity = position3 - position;
  velocity.x *= 100;
  velocity.y *= 100;
  velocity.z *= 100;
  gameapi -> emit(bloodEmitter.value(), position3, orientation3, glm::vec3(0.f, 2.f, 0.f), std::nullopt, std::nullopt);
}

void emitGibs(objid sceneId, objid lookAtId, glm::vec3 position){
  emitBlood(sceneId, lookAtId, position);  // just emit blood for now
}


std::optional<objid> waterEmitter;
void emitWaterSplash(objid sceneId, objid lookAtId, glm::vec3 position){
  modlog("water", "water emitter");
  //modassert(false, "emit blood not yet implemented");
  static bool callOnce = true;
  if (callOnce){
    callOnce = false;
    std::string particleStr("+lookat:enabled;limit:100;+scale:0.3 0.3 0.3;+mesh:../gameresources/build/primitives/plane_xy_1x1.gltf;+physics_layer:4;+physics:enabled;+physics_type:dynamic;+tint:0 0 1 1;+physics_gravity:0 -9.10 0;+physics_collision:nocollide;+texture:../gameresources/textures/particles/blood.png;+normal-texture:../gameresources/textures/particles/blood.normal.png");
    particleStr += std::to_string(lookAtId);
    modlog("water", particleStr);

    auto playerName = gameapi -> getGameObjNameForId(lookAtId);
    modassert(playerName.has_value(), "player name does not have a value...");
    waterEmitter = createParticleEmitter(sceneId, particleStr, "+water-emitter");
  }

  gameapi -> emit(waterEmitter.value(), position, std::nullopt, std::nullopt, std::nullopt, std::nullopt);

  auto position2 = position + glm::vec3(0.2f, 0.f, 0.f);
  gameapi -> emit(waterEmitter.value(), position2, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  
  auto position3 =  position + glm::vec3(-0.2f, 0.f, 0.f);
  auto velocity = position3 - position;
  velocity.x *= 100;
  velocity.y *= 100;
  velocity.z *= 100;
  gameapi -> emit(waterEmitter.value(), position3, std::nullopt, glm::vec3(0.f, 2.f, 0.f), std::nullopt, std::nullopt); 
}

void emitEffectRaw(EffekData& effect, glm::vec3 position){
  if (!effect.id.has_value()){
    effect.id = createEffect(effect.path, effect.scale, effect.tint);
  }
  gameapi -> emit(effect.id.value(), position, std::nullopt, std::nullopt, std::nullopt, std::nullopt); 
}

void emitEffect(std::string effectName, glm::vec3 position){
  auto& effect = effects.at(effectName);
  emitEffectRaw(effect, position);
}

void emitParticle(int symbol, glm::vec3 position, glm::quat rotation, glm::vec3 initialVelocity){
  auto name = nameForSymbol(symbol);
  if (name == ""){
    return;
  }
  if (name.size() > 0 &&  name.at(0) == '?'){
    auto particleName = name.substr(1, name.size());
    auto particleId = getParticleEmitterCore(particleName);
    if (particleId.has_value()){
      gameapi -> emit(particleId.value(), position, rotation, initialVelocity, std::nullopt, std::nullopt);
    }else{

      auto emitterEffect = getParticleEmitterEffek(particleName);
      if (emitterEffect.has_value()){
         emitEffectRaw(*emitterEffect.value(), position);
      }else{
        std::cout << "missing particle for: " << name << std::endl;
      }
     
    }

  }else if (name == "explosion"){
    emitBlood(gameapi -> rootSceneId(), 0, position);
  }else{
    if (effects.find(name) != effects.end()){
      emitEffect(name, position);
    }
  }
}

void emitKillEffect(glm::vec3 position){
   emitEffect("killeffect", position);
}

void emitWarp(glm::vec3 position){
  emitEffect("warp", position);
}

void emitSplash(glm::vec3 position){
  emitEffect("splash", position);
}

void emitSparks(glm::vec3 position){
  emitEffect("sparks", position);
}

void emitFirework(glm::vec3 position){
  emitEffect("firework", position);
}

void emitFog(glm::vec3 position){
  emitEffect("fog", position);
}

void makeDecal(MaterialToParticle& material, glm::vec3 position, glm::quat rotation, objid parent){
  std::unordered_map<std::string, AttributeValue> attrAttributes = { 
    { "mesh", "../gameresources/build/primitives/plane_xy_1x1.gltf" }, 
    { "texture", material.hitDecal.value().texture },
    { "scale", glm::vec3(0.01f * material.hitDecal.value().size.x, 0.01f * material.hitDecal.value().size.y, 0.01f) },
    { "tint", material.hitDecal.value().tint },
  };
  GameobjAttributes attr { .attr = attrAttributes };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto decal = gameapi -> makeObjectAttr(gameapi -> listSceneId(parent), std::to_string(getUniqueObjId()), attr, submodelAttributes);
  modassert(decal.has_value(), "could not create decal");
  gameapi -> makeParent(decal.value(), parent);

  auto zFightingAdjustment = rotation * glm::vec3(0.f, 0.f, -0.001f);

  gameapi -> setGameObjectPosition(decal.value(), position + zFightingAdjustment, true, Hint { .hint = "make decal pos" }); 
  gameapi -> setGameObjectRot(decal.value(), rotation, true, Hint { .hint = "make decal rot" }); 

}