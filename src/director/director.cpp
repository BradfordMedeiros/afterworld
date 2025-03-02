#include "./director.h"

extern CustomApiBindings* gameapi;

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