#ifndef MOD_AFTERWORLD_DIRECTOR
#define MOD_AFTERWORLD_DIRECTOR

#include "./spawn.h"

struct Director {
	float lastEnemySpawnTime;
};

Director createDirector();
void handleDirector(Director& director);


#endif 