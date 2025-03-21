#include "./spawn.h"

extern CustomApiBindings* gameapi;

objid createSpawnManagedPrefab(objid sceneId, objid spawnOwnerId, const char* prefab, glm::vec3 pos, glm::quat rotation){
  GameobjAttributes attr = {
    .attr = {
      { "scene", prefab },
      { "+item", "pickup-remove:prefab" },
      { "spawn-managed", static_cast<float>(spawnOwnerId)},
      { "position", pos },
    },
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes = {};
  return gameapi -> makeObjectAttr(
    sceneId, 
    std::string("[spawned-instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  ).value();
}

const int basicEnemyInstance = getSymbol("enemy");
const int turretInstance = getSymbol("turret");
const int tvInstance = getSymbol("tv");
const int crawlerInstance = getSymbol("crawler");

const int ammoInstance = getSymbol("ammo");

objid spawnEntity(int spawnTypeSymbol, objid spawnOwnerId, objid sceneId, glm::vec3 pos, glm::quat rotation){
  std::cout << "do spawn entity: " << nameForSymbol(spawnTypeSymbol) << std::endl;
  if (spawnTypeSymbol == basicEnemyInstance){
    return createSpawnManagedPrefab(sceneId, spawnOwnerId, "../afterworld/scenes/prefabs/enemy/enemy.rawscene", pos, rotation);
  }else if (spawnTypeSymbol == turretInstance){
    return createSpawnManagedPrefab(sceneId, spawnOwnerId, "../afterworld/scenes/prefabs/enemy/turret.rawscene", pos, rotation);
  } else if (spawnTypeSymbol == ammoInstance){
    return createSpawnManagedPrefab(sceneId, spawnOwnerId, "../afterworld/scenes/prefabs/ammo.rawscene", pos, rotation);
  }else if (spawnTypeSymbol == tvInstance){
    return createSpawnManagedPrefab(sceneId, spawnOwnerId, "../afterworld/scenes/prefabs/enemy/tv.rawscene", pos, rotation);
  }else if (spawnTypeSymbol == crawlerInstance){
    return createSpawnManagedPrefab(sceneId, spawnOwnerId, "../afterworld/scenes/prefabs/enemy/crawler.rawscene", pos, rotation);
  }
  modassert(false, "invalid type: " + nameForSymbol(spawnTypeSymbol));
  return -1;
}


int spawnTypeFromAttr(std::optional<std::string>&& value){
  modlog("do spawn spawner create", value.value());
  if (value.value() == "enemy"){
    return basicEnemyInstance;
  }
  if (value.value() == "ammo"){
    return ammoInstance;
  }
  if (value.value() == "turret"){
    return turretInstance;
  }
  if (value.value() == "tv"){
    return tvInstance;
  }
  if (value.value() == "crawler"){
    return crawlerInstance;
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

void spawnAddId(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, objid id){
  auto attr = getAttrHandle(id);
  modlog("spawn spawner add id", std::to_string(id));
  std::optional<float> spawnLimitFloat = getFloatAttr(attr, "spawnlimit");
  std::optional<int> spawnLimit;
  if (spawnLimitFloat.has_value()){
    spawnLimit = static_cast<int>(spawnLimitFloat.value());
  }
  managedSpawnpoints[id] = Spawnpoint {
    .type = spawnTypeFromAttr(getStrAttr(attr, "spawn")),
    .tags = spawnTags(getStrAttr(attr, "spawntags")),
    .respawnRate = getFloatAttr(attr, "spawnrate"),
    .itemLimit = spawnLimit,
    .lastSlotFreeTime = std::nullopt,
    .managedIds = {},
  };
}

void spawnRemoveId(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, objid id){
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

void onSpawnTick(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints){
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

int spawnFromAllSpawnpoints(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, const char* tag){
  if (managedSpawnpoints.size() == 0){
    return 0;
  }

  int numSpawned = 0;
  float currentTime = gameapi -> timeSeconds(false);
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    if (tag == NULL || spawnpoint.tags.count(tag) > 0){
      spawnEntity(id, spawnpoint, currentTime);
      numSpawned++;
    }
  }
  return numSpawned;
}

bool spawnFromRandomSpawnpoint(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints,  const char* tag, std::set<objid> excludeSpawnpoints){
  std::vector<objid> spawnpointIds;
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    if ((tag == NULL || spawnpoint.tags.count(tag) > 0) && excludeSpawnpoints.count(id) == 0){
      spawnpointIds.push_back(id);
    }
  }
  
  if (spawnpointIds.size() == 0){
    return false;
  }

  float currentTime = gameapi -> timeSeconds(false);
  auto spawnpointIndex = randomNumber(0, spawnpointIds.size() - 1);
  auto id = spawnpointIds.at(spawnpointIndex);
  spawnEntity(id, managedSpawnpoints.at(id), currentTime);
  return true;
}

void removeAllSpawnedEntities(){
  auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn-managed", std::nullopt, std::nullopt);
  for (auto spawnpointId : spawnpointIds){
    gameapi -> removeByGroupId(spawnpointId);
  }  
}

int numberOfSpawnManaged(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints){
  int count = 0;
  for (auto &[_, spawnpoint] : managedSpawnpoints){
    count = count + spawnpoint.managedIds.size();
  }
  return count;
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


std::vector<RespawnInfo> getRespawnInfos(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, float currentTime){
  std::vector<RespawnInfo> respawnInfos;
  for (auto &[id, spawnpoint] : managedSpawnpoints){
    respawnInfos.push_back(respawnInfo(spawnpoint, id, currentTime));
  }
  return respawnInfos;
}

void showSpawnpoints(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, bool showSpawnpoints){
  for (auto &[id, _] : managedSpawnpoints){
    setGameObjectMeshEnabled(id, showSpawnpoints ? true : false);
  }
}

void drawDebugRespawnInfo(RespawnInfo& respawnInfo){
  float percentage = respawnInfo.blocked ? 1.f : (respawnInfo.elapsedTime / respawnInfo.totalTime);
  std::cout << "spawn percentage :  " << percentage << std::endl;
  gameapi -> drawRect(0.f, 0.f, 0.2f, 0.2f, false, glm::vec4(0.2f, 0.2f, 0.2f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  gameapi -> drawRect(0.f, 0.f, 0.2f * percentage, 0.2f, false, glm::vec4(0.2f, 0.2f, 0.8f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}


