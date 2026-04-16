#include "./director.h"

extern CustomApiBindings* gameapi;

bool isControlledPlayer(int playerId);

Director createDirector(){
	return Director{
		.lastEnemySpawnTime = 0.f,
		.managedSpawnpoints = {},
	};
}

std::set<objid> occupiedSpawners(Director& director){
	std::set<objid> ids;
	for (auto &[id, spawnpoint] : director.managedSpawnpoints){
		if (spawnpoint.managedIds.size() > 0){
			ids.insert(id);
		}
	}
	return ids;
}

void handleDirector(Director& director){
	return;
	// check the spawnpoints, and then spawn from a random one 
	auto currTime = gameapi -> timeSeconds(false);
	auto sinceLastSpawn = currTime - director.lastEnemySpawnTime;

	bool enoughTime = sinceLastSpawn > 5.f;
	bool tooManyEnemies = numberOfSpawnManaged(director.managedSpawnpoints) >= 2;
	if (enoughTime && !tooManyEnemies){
		director.lastEnemySpawnTime = currTime;
		auto didSpawn = spawnFromRandomSpawnpoint(director.managedSpawnpoints, NULL, occupiedSpawners(director));
		if (!didSpawn){
			modlog("director", "could not spawn, most likely no more spawnpoints, skipping");
		}
	}
	std::cout << "number of enenmies: " << numberOfSpawnManaged(director.managedSpawnpoints) << std::endl;
}

void handleSpawnCollision(Director& director, int32_t obj1, int32_t obj2){
  if (isControlledPlayer(obj2)){
    auto objAttr = getAttrHandle(obj1);
    auto spawnPointTag = getStrAttr(objAttr, "spawn-trigger");
    if (spawnPointTag.has_value()){
      spawnFromAllSpawnpoints(director.managedSpawnpoints, spawnPointTag.value().c_str());
      gameapi -> removeByGroupId(obj1);
    }
  }else if (isControlledPlayer(obj1)){
    auto objAttr = getAttrHandle(obj2);
    auto spawnPointTag = getStrAttr(objAttr, "spawn-trigger"); 
    if (spawnPointTag.has_value()){
      spawnFromAllSpawnpoints(director.managedSpawnpoints, spawnPointTag.value().c_str());
      gameapi -> removeByGroupId(obj2);
    }
  }
}