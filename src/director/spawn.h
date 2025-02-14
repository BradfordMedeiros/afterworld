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

void spawnFromAllSpawnpoints(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, const char* tag);
void spawnFromRandomSpawnpoint(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, const char* tag);
void removeAllSpawnedEntities();

void spawnAddId(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, objid id);
void spawnRemoveId(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, objid id);
void onSpawnTick(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints);

struct RespawnInfo {
	objid id;
  bool blocked;
  float totalTime;
  float elapsedTime;
};

std::vector<RespawnInfo> getRespawnInfos(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, float currentTime);
void showSpawnpoints(std::unordered_map<objid, Spawnpoint>& managedSpawnpoints, bool showSpawnpoints);

#endif