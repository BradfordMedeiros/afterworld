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
      { "layer", "no_depth" },    ///////
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
  auto query = gameapi -> compileSqlQuery("select material, hit-particle, particle from materials", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  std::map<std::string, GameobjAttributes> submodelAttributes;
  for (auto &row : result){
    auto particleAttr = particleAttributes(row.at(1));
    auto materialEmitterId = gameapi -> makeObjectAttr(sceneId, "+code-hitparticle-" + row.at(0), particleAttr, submodelAttributes);

    auto splashParticleAttr = particleAttributes(row.at(2));
    auto splashEmitterId = gameapi -> makeObjectAttr(sceneId, "+code-splashparticle-" + row.at(0), splashParticleAttr, submodelAttributes);

    materialToParticles.push_back(MaterialToParticle {
      .material = row.at(0),
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
	std::map<std::string, GameobjAttributes> submodelAttributes;
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

void emitBloodTest(objid sceneId){
  gameapi -> schedule(sceneId, 1000, NULL, [sceneId](void*) -> void {
    emitBlood(sceneId, glm::vec3(0.f, 0.f, 0.f));
    emitBloodTest(sceneId);
  });
}



std::vector<ParticleAndEmitter> particleEmitters;
void loadParticleEmitters(objid rootSceneId){
  /// temp code hackey hook here

  emitBloodTest(rootSceneId);
  ///////////////////////


  auto query = gameapi -> compileSqlQuery("select name, projectile from particles", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  modlog("load particle emitters", "start");

  std::map<std::string, GameobjAttributes> submodelAttributes;
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


std::optional<objid> bloodEmitter = std::nullopt;
void emitBlood(objid sceneId, glm::vec3 position){
  modlog("blood", "blood emitter");
  //modassert(false, "emit blood not yet implemented");
  static bool callOnce = true;
  if (callOnce){
    callOnce = false;
    std::string particleStr("+scale:0.1 0.1 0.1;+mesh:./res/models/box/box.obj;+physics:enabled;+physics_type:dynamic;+Cube/tint:1 0 0 1;+physics_gravity:0 -10 0;+physics_collision:nocollide");
    bloodEmitter = createParticleEmitter(sceneId, particleStr, "+blood-emitter");
  }
  if (bloodEmitter.has_value()){
    gameapi -> emit(bloodEmitter.value(), position, std::nullopt /* normal */, std::nullopt, std::nullopt, std::nullopt);
  }
}