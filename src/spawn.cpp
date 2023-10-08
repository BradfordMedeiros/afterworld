#include "./spawn.h"

extern CustomApiBindings* gameapi;

void createPrefab(objid sceneId, const char* prefab, glm::vec3 pos, glm::quat rotation){
  GameobjAttributes attr = {
    .stringAttributes = {
      { "scene", prefab },
    },
    .numAttributes = {},
    .vecAttr = {
      .vec3 = {
        { "position", pos },
      },
      .vec4 = {},
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  gameapi -> makeObjectAttr(
    sceneId, 
    std::string("[spawned-instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  );
}

void createEnemyInstance(objid sceneId, glm::vec3 pos, glm::quat rotation, std::string team){
  // replace with   createPrefab(sceneId, "../afterworld/scenes/prefabs/enemy.rawscene", pos, rotation);
  // when fix physics bug, add data attributes
  GameobjAttributes attr = {
    .stringAttributes = {
      { "mesh", "../gameresources/build/characters/plaguerobot.gltf" },
      { "physics", "enabled" },
      { "physics_type", "dynamic" },
      { "agent", "basic" },
      { "agent-target", team == "red" ? "blue" : "red" },
      { "spawn-managed", "" },
      { "team", team  },
    },
    .numAttributes = {
      { "health", 130.f },
    },
    .vecAttr = {
      .vec3 = {
        { "position", pos },
        { "physics_angle", glm::vec3(0.f, 0.f, 0.f) },
      },
      .vec4 = {
      },
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  gameapi -> makeObjectAttr(
    sceneId, 
    std::string("spawned-instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  );
}


const int basicEnemyInstance = getSymbol("enemy");
const int ammoInstance = getSymbol("ammo");

void spawnEntity(int spawnTypeSymbol, objid sceneId, glm::vec3 pos, glm::quat rotation){
  if (spawnTypeSymbol == basicEnemyInstance){
    createEnemyInstance(sceneId, pos, rotation, "red");
    return;
  }else if (spawnTypeSymbol == ammoInstance){
    createPrefab(sceneId, "../afterworld/scenes/prefabs/ammo.rawscene", pos, rotation);
    return;
  }
  modassert(false, "invalid type: " + nameForSymbol(spawnTypeSymbol));
}


void spawnEntity(objid id){
  auto spawnPosition = gameapi -> getGameObjectPos(id, true);
  auto spawnRotation = gameapi -> getGameObjectRotation(id, true);  // maybe don't want the actual rotn but rather only on xz plane?  maybe?
  spawnEntity(ammoInstance, gameapi -> listSceneId(id), spawnPosition, spawnRotation);
}

void spawnFromAllSpawnpoints(const char* team){
  auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn", std::nullopt, std::nullopt);
  modassert(spawnpointIds.size() > 0, "no spawnpoints");
  for (auto spawnpointId : spawnpointIds){
    spawnEntity(spawnpointId);
  }
}

void spawnFromRandomSpawnpoint(const char* team){
  auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn", std::nullopt, std::nullopt);
  if (spawnpointIds.size() == 0){
    return;
  }
  auto spawnpointIndex = randomNumber(0, spawnpointIds.size() - 1);
  auto spawnpointId = spawnpointIds.at(spawnpointIndex);
  spawnEntity(spawnpointId);
}

void removeAllSpawnedEntities(){
  auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn-managed", std::nullopt, std::nullopt);
  for (auto spawnpointId : spawnpointIds){
    gameapi -> removeObjectById(spawnpointId);
  }  
}
