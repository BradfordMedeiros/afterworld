#include "./spawn.h"

extern CustomApiBindings* gameapi;

struct Spawnpoint {
  int type;
  std::set<std::string> tags;
  std::optional<float> respawnRate;
  std::optional<int> itemLimit;
  std::optional<float> lastSlotFreeTime;
  std::set<objid> managedIds;
};


objid createSpawnManagedPrefab(objid sceneId, objid spawnOwnerId, const char* prefab, glm::vec3 pos, glm::quat rotation){
  GameobjAttributes attr = {
    .stringAttributes = {
      { "scene", prefab },
      { "+item", "pickup-remove:prefab" },
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

objid createEnemyInstance(objid sceneId, objid spawnOwnerId, glm::vec3 pos, glm::quat rotation){
  // replace with   createPrefab(sceneId, "../afterworld/scenes/prefabs/enemy.rawscene", pos, rotation);
  // when fix physics bug, add data attributes
  GameobjAttributes attr = {
    .stringAttributes = {
      { "mesh", "../gameresources/build/characters/plaguerobot.gltf" },
      { "physics", "enabled" },
      { "physics_type", "dynamic" },
      { "agent", "basic" },
      { "goal-info", "target" },
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
  std::cout << "do spawn entity: " << nameForSymbol(spawnTypeSymbol) << std::endl;
  if (spawnTypeSymbol == basicEnemyInstance){
    //return createEnemyInstance(sceneId, spawnOwnerId, pos, rotation);
    return createSpawnManagedPrefab(sceneId, spawnOwnerId, "../afterworld/scenes/prefabs/enemy/sentinel.rawscene", pos, rotation);

  }else if (spawnTypeSymbol == ammoInstance){
    return createSpawnManagedPrefab(sceneId, spawnOwnerId, "../afterworld/scenes/prefabs/ammo.rawscene", pos, rotation);
  }
  modassert(false, "invalid type: " + nameForSymbol(spawnTypeSymbol));
  return -1;
}

std::unordered_map<objid, Spawnpoint> managedSpawnpoints;

int spawnTypeFromAttr(std::optional<std::string>&& value){
  modlog("do spawn spawner create", value.value());
  if (value.value() == "enemy"){
    return basicEnemyInstance;
  }
  if (value.value() == "ammo"){
    return ammoInstance;
  }
  modassert(false, "invalid spawn type");
  return -1;
}

std::set<std::string> spawnTags(std::optional<std::string>&& value){
  if (!value.has_value()){
    return {};
  }

  auto values = split(value.value(), ',');
  std::set<std::string> tags;
  for (auto &value : values){
    tags.insert(value);
  }
  return tags;
}

void spawnAddId(objid id){
  auto attr = gameapi -> getGameObjectAttr(id);
  modlog("spawn spawner add id", std::to_string(id));
  managedSpawnpoints[id] = Spawnpoint {
    .type = spawnTypeFromAttr(getStrAttr(attr, "spawn")),
    .tags = spawnTags(getStrAttr(attr, "spawntags")),
    .respawnRate = getFloatAttr(attr, "spawnrate"),
    .itemLimit = getIntFromAttr(attr, "spawnlimit"),
    .lastSlotFreeTime = std::nullopt,
    .managedIds = {},
  };
}
void spawnRemoveId(objid id){
  modlog("spawn remove id", std::to_string(id));
  managedSpawnpoints.erase(id);
  for (auto &[_, spawnpoint] : managedSpawnpoints){
    spawnpoint.managedIds.erase(id);
  }
}

void spawnEntity(objid id, Spawnpoint& spawnpoint, float currentTime){
  modlog("spawn entity", std::to_string(id));
  auto spawnPosition = gameapi -> getGameObjectPos(id, true);
  auto spawnRotation = gameapi -> getGameObjectRotation(id, true);  // maybe don't want the actual rotn but rather only on xz plane?  maybe?
  auto spawnedEntityId = spawnEntity(spawnpoint.type, id, gameapi -> listSceneId(id), spawnPosition, spawnRotation);

  modlog("spawn managed add id", std::to_string(spawnedEntityId));
  spawnpoint.managedIds.insert(spawnedEntityId);
}

void onSpawnTick(){
  float currentTime = gameapi -> timeSeconds(false);
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    if (spawnpoint.respawnRate.has_value()){
      bool underSpawnLimit = spawnpoint.managedIds.size() < spawnpoint.itemLimit;
      //std::cout << "respawnRate = " << spawnpoint.respawnRate.value() << ", currentTime = " << currentTime << ", lastSpawnTime = " << spawnpoint.lastSpawnTime.value() << ", spawn limit: " << spawnpoint.managedIds.size() << std::endl;
      
      if (!underSpawnLimit){
        spawnpoint.lastSlotFreeTime = std::nullopt;
      }else if (!spawnpoint.lastSlotFreeTime.has_value()){
        spawnpoint.lastSlotFreeTime = currentTime;
      }
    
      // Spawn rate it determines by time elapsed since the slot was opened
      bool underSpawnRate = !spawnpoint.lastSlotFreeTime.has_value() || ((currentTime - spawnpoint.lastSlotFreeTime.value()) > spawnpoint.respawnRate.value());

      if (underSpawnRate && underSpawnLimit){
        //std::cout << "spawning:  " << id << std::endl;
        spawnEntity(id, spawnpoint, currentTime);
      }
    }
  }

  ////
  //drawDebugRespawnInfo(getRespawnInfos(gameapi -> timeSeconds(false)).at(0));
}

void spawnFromAllSpawnpoints(const char* team, const char* tag){
  if (managedSpawnpoints.size() == 0){
    return;
  }
  float currentTime = gameapi -> timeSeconds(false);
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    if (tag == NULL || spawnpoint.tags.count(tag) > 0){
      spawnEntity(id, spawnpoint, currentTime);
    }
  }
}
void spawnFromRandomSpawnpoint(const char* team, const char* tag){
  if (managedSpawnpoints.size() == 0){
    return;
  }
  float currentTime = gameapi -> timeSeconds(false);

  int numSpawnpointsWithTag = 0;
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    if (tag == NULL || spawnpoint.tags.count(tag) > 0){
      numSpawnpointsWithTag++;
    }
  }

  auto spawnpointIndex = randomNumber(0, numSpawnpointsWithTag - 1);
  int currentIndex = 0;
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    if (tag == NULL || spawnpoint.tags.count(tag) > 0){
      if (spawnpointIndex == currentIndex){
        modassert(gameapi -> gameobjExists(id), std::string("spawn element does not exist: ") + std::to_string(id));
        spawnEntity(id, spawnpoint, currentTime);
        break;
      }
      currentIndex++;
    }
  }
}

void removeAllSpawnedEntities(){
  auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn-managed", std::nullopt, std::nullopt);
  for (auto spawnpointId : spawnpointIds){
    gameapi -> removeByGroupId(spawnpointId);
  }  
}


RespawnInfo respawnInfo(Spawnpoint& spawnpoint, objid id, float currentTime){
   if (!spawnpoint.lastSlotFreeTime.has_value()){
    return RespawnInfo {
      .id = id,
      .blocked = true,
      .totalTime = spawnpoint.respawnRate.has_value() ? 0.f : spawnpoint.respawnRate.value(),
      .elapsedTime = 0.f,
    };
   }
   return RespawnInfo {
     .id = id,
     .blocked = false,
     .totalTime = spawnpoint.respawnRate.has_value() ? spawnpoint.respawnRate.value() : 0.f,
     .elapsedTime = currentTime - spawnpoint.lastSlotFreeTime.value(),
   };
}

std::vector<RespawnInfo> getRespawnInfos(float currentTime){
  std::vector<RespawnInfo> respawnInfos;
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    respawnInfos.push_back(respawnInfo(spawnpoint, id, currentTime));
  }
  return respawnInfos;
}

void drawDebugRespawnInfo(RespawnInfo& respawnInfo){
  float percentage = respawnInfo.blocked ? 1.f : (respawnInfo.elapsedTime / respawnInfo.totalTime);
  std::cout << "spawn percentage :  " << percentage << std::endl;
  gameapi -> drawRect(0.f, 0.f, 0.2f, 0.2f, false, glm::vec4(0.2f, 0.2f, 0.2f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawRect(0.f, 0.f, 0.2f * percentage, 0.2f, false, glm::vec4(0.2f, 0.2f, 0.8f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
}
