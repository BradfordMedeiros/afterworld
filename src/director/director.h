#ifndef MOD_AFTERWORLD_DIRECTOR
#define MOD_AFTERWORLD_DIRECTOR

#include "./spawn.h"

struct Director {
	float lastEnemySpawnTime;
	std::unordered_map<objid, Spawnpoint> managedSpawnpoints;
};

Director createDirector();
void handleDirector(Director& director);
void handleSpawnCollision(Director& director, int32_t obj1, int32_t obj2);


#endif 