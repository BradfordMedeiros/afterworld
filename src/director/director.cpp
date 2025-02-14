#include "./director.h"

extern CustomApiBindings* gameapi;

std::unordered_map<objid, Spawnpoint> managedSpawnpoints;

Director createDirector(){
	return Director{
		.lastEnemySpawnTime = 0.f,
	};
}


void handleDirector(Director& director){
	// check the spawnpoints, and then spawn from a random one 
	auto currTime = gameapi -> timeSeconds(false);
	auto sinceLastSpawn = currTime - director.lastEnemySpawnTime;

	bool enoughTime = sinceLastSpawn > 5.f;
	bool tooManyEnemies = numberOfSpawnManaged(managedSpawnpoints) >= 3;
	if (enoughTime && !tooManyEnemies){
		director.lastEnemySpawnTime = currTime;
		spawnFromRandomSpawnpoint(managedSpawnpoints, NULL);
	}
	std::cout << "number of enenmies: " << numberOfSpawnManaged(managedSpawnpoints) << std::endl;
}