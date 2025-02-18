#include "./director.h"

extern CustomApiBindings* gameapi;



Director createDirector(){
	return Director{
		.lastEnemySpawnTime = 0.f,
		.managedSpawnpoints = {},
	};
}


void handleDirector(Director& director){
	// check the spawnpoints, and then spawn from a random one 
	auto currTime = gameapi -> timeSeconds(false);
	auto sinceLastSpawn = currTime - director.lastEnemySpawnTime;

	bool enoughTime = sinceLastSpawn > 5.f;
	bool tooManyEnemies = numberOfSpawnManaged(director.managedSpawnpoints) >= 2;
	if (enoughTime && !tooManyEnemies){
		director.lastEnemySpawnTime = currTime;
		spawnFromRandomSpawnpoint(director.managedSpawnpoints, NULL);
	}
	std::cout << "number of enenmies: " << numberOfSpawnManaged(director.managedSpawnpoints) << std::endl;
}