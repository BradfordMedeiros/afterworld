#include "./director.h"

extern CustomApiBindings* gameapi;


Director createDirector(){
	return Director{
		.lastEnemySpawnTime = 0.f,
	};
}

void spawnRandomly(){
	
}

void handleDirector(Director& director){
	// check the spawnpoints, and then spawn from a random one 
	auto currTime = gameapi -> timeSeconds(false);
	auto sinceLastSpawn = currTime - director.lastEnemySpawnTime;
	if (sinceLastSpawn > 5.f){
		director.lastEnemySpawnTime = currTime;
		modlog("director", "spawn enemy");
	}


}