#ifndef MOD_AFTERWORLD_SPAWN
#define MOD_AFTERWORLD_SPAWN

#include "../util.h"

struct Spawnpoint {
  int type;
  std::set<std::string> tags;
  std::optional<float> respawnRate;
  std::optional<int> itemLimit;
  std::optional<float> lastSlotFreeTime;
  std::set<objid> managedIds;
};

void spawnFromAllSpawnpoints(const char* tag);
void spawnFromRandomSpawnpoint(const char* team, const char* tag = NULL);
void removeAllSpawnedEntities();

void spawnAddId(objid id);
void spawnRemoveId(objid id);
void onSpawnTick();

struct RespawnInfo {
	objid id;
  bool blocked;
  float totalTime;
  float elapsedTime;
};

std::vector<RespawnInfo> getRespawnInfos(float currentTime);
void showSpawnpoints(bool showSpawnpoints);

#endif