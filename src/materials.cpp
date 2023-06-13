#include "./materials.h"

extern CustomApiBindings* gameapi;

std::optional<std::string> materialTypeForObj(objid id){
  auto attr = gameapi -> getGameObjectAttr(id);
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
    .stringAttributes = { 
      { "state", "disabled" },    // default should keep
      { "physics", "disabled" },  
      { "layer", "no_depth" },    ///////
    },
    .numAttributes = { { "duration", 10.f } },
    .vecAttr = {  
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

std::optional<objid> createParticleEmitter(objid sceneId, std::string& particleStr, const char* emitterName){
	if (particleStr == ""){
		return std::nullopt;
	}
	std::map<std::string, GameobjAttributes> submodelAttributes;
  auto particleAttr = particleAttributes(particleStr);
  return gameapi -> makeObjectAttr(sceneId, emitterName, particleAttr, submodelAttributes);
}