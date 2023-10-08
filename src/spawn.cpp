#include "./spawn.h"

extern CustomApiBindings* gameapi;

void createSpawnManagedPrefab(objid sceneId, objid spawnOwnerId, const char* prefab, glm::vec3 pos, glm::quat rotation){
  GameobjAttributes attr = {
    .stringAttributes = {
      { "scene", prefab },
    },
    .numAttributes = {
      { "spawn-managed", spawnOwnerId},
    },
    .vecAttr = {
      .vec3 = {
        { "position", pos },
      },
      .vec4 = {},
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
  gameapi -> makeObjectAttr(
    sceneId, 
    std::string("[spawned-instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  );
}

void createEnemyInstance(objid sceneId, objid spawnOwnerId, glm::vec3 pos, glm::quat rotation, std::string team){
  // replace with   createPrefab(sceneId, "../afterworld/scenes/prefabs/enemy.rawscene", pos, rotation);
  // when fix physics bug, add data attributes
  GameobjAttributes attr = {
    .stringAttributes = {
      { "mesh", "../gameresources/build/characters/plaguerobot.gltf" },
      { "physics", "enabled" },
      { "physics_type", "dynamic" },
      { "agent", "basic" },
      { "agent-target", team == "red" ? "blue" : "red" },
      { "team", team  },
    },
    .numAttributes = {
      { "health", 130.f },
      { "spawn-managed", spawnOwnerId },
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

void spawnEntity(int spawnTypeSymbol, objid spawnOwnerId, objid sceneId, glm::vec3 pos, glm::quat rotation){
  if (spawnTypeSymbol == basicEnemyInstance){
    createEnemyInstance(sceneId, spawnOwnerId, pos, rotation, "red");
    return;
  }else if (spawnTypeSymbol == ammoInstance){
    createSpawnManagedPrefab(sceneId, spawnOwnerId, "../afterworld/scenes/prefabs/ammo.rawscene", pos, rotation);
    return;
  }
  modassert(false, "invalid type: " + nameForSymbol(spawnTypeSymbol));
}

struct Spawnpoint {
  int type;
  std::optional<float> respawnRate;
  std::optional<int> itemLimit;
};

std::unordered_map<objid, Spawnpoint> managedSpawnpoints;

int spawnTypeFromAttr(std::optional<std::string>&& value){
  if (value == "enemy"){
    return basicEnemyInstance;
  }
  if (value == "ammo"){
    return ammoInstance;
  }
  modassert(false, "invalid spawn type");
  return -1;
}

void spawnAddId(objid id){
  auto attr = gameapi -> getGameObjectAttr(id);
  managedSpawnpoints[id] = Spawnpoint {
    .type = spawnTypeFromAttr(getStrAttr(attr, "spawn")),
    .respawnRate = getFloatAttr(attr, "spawnrate"),
  };
}
void spawnRemoveId(objid id){
  managedSpawnpoints.erase(id);
}

void onSpawnTick(){
  std::cout << "spawn on frame" << std::endl;

}


void spawnEntity(objid id){
  auto spawnPosition = gameapi -> getGameObjectPos(id, true);
  auto spawnRotation = gameapi -> getGameObjectRotation(id, true);  // maybe don't want the actual rotn but rather only on xz plane?  maybe?
  spawnEntity(ammoInstance, id, gameapi -> listSceneId(id), spawnPosition, spawnRotation);
}


void spawnFromAllSpawnpoints(const char* team){
  if (managedSpawnpoints.size() == 0){
    return;
  }
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    spawnEntity(id);
  }
}
void spawnFromRandomSpawnpoint(const char* team){
  if (managedSpawnpoints.size() == 0){
    return;
  }

  auto spawnpointIndex = randomNumber(0, managedSpawnpoints.size() - 1);
  int currentIndex = 0;
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    if (spawnpointIndex == currentIndex){
      spawnEntity(id);
      break;
    }
    currentIndex++;
  }
}

void removeAllSpawnedEntities(){
  auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn-managed", std::nullopt, std::nullopt);
  for (auto spawnpointId : spawnpointIds){
    gameapi -> removeObjectById(spawnpointId);
  }  
}


