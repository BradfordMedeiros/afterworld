#ifndef MOD_AFTERWORLD_SPAWN
#define MOD_AFTERWORLD_SPAWN

#include "./util.h"

void spawnFromAllSpawnpoints(const char* team);
void spawnFromRandomSpawnpoint(const char* team);
void removeAllSpawnedEntities();

#endif