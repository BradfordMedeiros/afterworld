#include "./materials.h"

extern CustomApiBindings* gameapi;

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

std::vector<MaterialToParticle> loadMaterials(objid sceneId){
  std::vector<MaterialToParticle> materialToParticles;
  auto query = gameapi -> compileSqlQuery("select material, hit-particle, particle, hit-sound from materials", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  for (auto &row : result){
    auto particleAttr = particleAttributes(row.at(1));
    auto materialEmitterId = gameapi -> makeObjectAttr(sceneId, "+code-hitparticle-" + row.at(0), particleAttr, submodelAttributes);

    auto splashParticleAttr = particleAttributes(row.at(2));
    auto splashEmitterId = gameapi -> makeObjectAttr(sceneId, "+code-splashparticle-" + row.at(0), splashParticleAttr, submodelAttributes);

    GameobjAttributes attr {
      .attr = {},
    };

    auto materialHitSound = row.at(3);
    std::optional<objid> hitParticleClipId;
    if (materialHitSound != ""){
      attr.attr["clip"] = row.at(3);
      hitParticleClipId = gameapi -> makeObjectAttr(sceneId, std::string("&material-hitsound") + row.at(0), attr, submodelAttributes);
    }

    materialToParticles.push_back(MaterialToParticle {
      .material = row.at(0),
      .hitParticleClipId = hitParticleClipId,
      .hitParticle = ParticleAndEmitter {
        .particle = row.at(1),
        .particleId = materialEmitterId.value(),
      },
      .splashParticle = ParticleAndEmitter {
        .particle = row.at(2),
        .particleId = splashEmitterId.value(),
      },
    });
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

std::vector<ParticleAndEmitter> particleEmitters;
void loadParticleEmitters(objid rootSceneId){
  auto query = gameapi -> compileSqlQuery("select name, projectile from particles", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  modlog("load particle emitters", "start");

  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  for (auto &row : result){
    auto particleAttr = particleAttributes(row.at(1));
    auto materialEmitterId = gameapi -> makeObjectAttr(rootSceneId, "+code-particle-" + row.at(0), particleAttr, submodelAttributes);
    particleEmitters.push_back(ParticleAndEmitter {
      .particle = row.at(0),
      .particleId = materialEmitterId.value(),
    });
    modlog("load particle emitters - loading", row.at(0));
  }
  modlog("load particle emitters", "finish");

}
std::optional<objid> getParticleEmitter(std::string& emitterName){
  for (auto &emitter : particleEmitters){
    if (emitter.particle == emitterName){
      return emitter.particleId;
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

  auto mainObjPos = gameapi -> getGameObjectPos(lookAtId, true);

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

void emitExplosion(objid sceneId, objid lookAtId, glm::vec3 position, glm::vec3 size){
  emitBlood(sceneId, lookAtId, position);
}

std::optional<objid> waterEmitter = std::nullopt;
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