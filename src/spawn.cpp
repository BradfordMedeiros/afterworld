#include "./spawn.h"

extern CustomApiBindings* gameapi;

objid createSpawnManagedPrefab(objid sceneId, objid spawnOwnerId, const char* prefab, glm::vec3 pos, glm::quat rotation){
  GameobjAttributes attr = {
    .stringAttributes = {
      { "scene", prefab },
      { "+item", "pickup-remove:scene" },
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
  return gameapi -> makeObjectAttr(
    sceneId, 
    std::string("[spawned-instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  ).value();
}

objid createEnemyInstance(objid sceneId, objid spawnOwnerId, glm::vec3 pos, glm::quat rotation, std::string team){
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
  return gameapi -> makeObjectAttr(
    sceneId, 
    std::string("spawned-instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  ).value();
}


const int basicEnemyInstance = getSymbol("enemy");
const int ammoInstance = getSymbol("ammo");

objid spawnEntity(int spawnTypeSymbol, objid spawnOwnerId, objid sceneId, glm::vec3 pos, glm::quat rotation){
  if (spawnTypeSymbol == basicEnemyInstance){
    return createEnemyInstance(sceneId, spawnOwnerId, pos, rotation, "red");
  }else if (spawnTypeSymbol == ammoInstance){
    return createSpawnManagedPrefab(sceneId, spawnOwnerId, "../afterworld/scenes/prefabs/ammo.rawscene", pos, rotation);
  }
  modassert(false, "invalid type: " + nameForSymbol(spawnTypeSymbol));
  return -1;
}

struct Spawnpoint {
  int type;
  std::optional<float> respawnRate;
  std::optional<int> itemLimit;

  std::optional<float> lastSpawnTime;
  std::set<objid> managedIds;
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
    .itemLimit = getIntFromAttr(attr, "spawnlimit"),

    .lastSpawnTime = 0.f,
    .managedIds = {},
  };
}
void spawnRemoveId(objid id){
  managedSpawnpoints.erase(id);
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    spawnpoint.managedIds.erase(id);
  }
}

void spawnEntity(objid id, Spawnpoint& spawnpoint, float currentTime){
  auto spawnPosition = gameapi -> getGameObjectPos(id, true);
  auto spawnRotation = gameapi -> getGameObjectRotation(id, true);  // maybe don't want the actual rotn but rather only on xz plane?  maybe?
  spawnpoint.lastSpawnTime = currentTime;
  auto spawnedEntityId = spawnEntity(ammoInstance, id, gameapi -> listSceneId(id), spawnPosition, spawnRotation);
  spawnpoint.managedIds.insert(spawnedEntityId);
}

void onSpawnTick(){
  float currentTime = gameapi -> timeSeconds(false);
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    if (spawnpoint.respawnRate.has_value()){
      bool underSpawnRate = !spawnpoint.lastSpawnTime.has_value() || ((currentTime - spawnpoint.lastSpawnTime.value()) > spawnpoint.respawnRate.value());
      bool underSpawnLimit = spawnpoint.managedIds.size() < spawnpoint.itemLimit;
      //std::cout << "respawnRate = " << spawnpoint.respawnRate.value() << ", currentTime = " << currentTime << ", lastSpawnTime = " << spawnpoint.lastSpawnTime.value() << std::endl;
      if (underSpawnRate && underSpawnLimit){
        //std::cout << "spawning:  " << id << std::endl;
        spawnEntity(id, spawnpoint, currentTime);
      }
    }
  }
}

void spawnFromAllSpawnpoints(const char* team){
  if (managedSpawnpoints.size() == 0){
    return;
  }
  float currentTime = gameapi -> timeSeconds(false);
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    spawnEntity(id, spawnpoint, currentTime);
  }
}
void spawnFromRandomSpawnpoint(const char* team){
  if (managedSpawnpoints.size() == 0){
    return;
  }
  float currentTime = gameapi -> timeSeconds(false);
  auto spawnpointIndex = randomNumber(0, managedSpawnpoints.size() - 1);
  int currentIndex = 0;
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    if (spawnpointIndex == currentIndex){
      spawnEntity(id, spawnpoint, currentTime);
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


