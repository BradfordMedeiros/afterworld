#ifndef MOD_AFTERWORLD_WORLDINFO
#define MOD_AFTERWORLD_WORLDINFO

#include "../util.h"

//////////////////////////////////////////////////////////////////////////////////

struct StateInfo {
  int symbol;
  std::set<int> tags;
  std::any data;
};

struct BoolState { 
  StateInfo stateInfo;
  bool value;
};
struct Vec3State {
  StateInfo stateInfo;
  glm::vec3 value;
};

struct WorldInfo {
  std::vector<BoolState> boolValues;
  std::vector<Vec3State> vec3Values;
};


void updateBoolState(WorldInfo& worldInfo, int symbol, bool value);
void updateVec3State(WorldInfo& worldInfo, int symbol, glm::vec3 value, std::set<int> tags = {}, std::any data = NULL);
std::optional<glm::vec3> getVec3State(WorldInfo& worldInfo, int symbol);
std::optional<Vec3State*> getVec3StateRef(WorldInfo& worldInfo, int symbol);

std::optional<bool> getBoolState(WorldInfo& worldInfo, int symbol);
std::vector<glm::vec3> getVec3StateByTag(WorldInfo& worldInfo, std::set<int> tags);
std::vector<Vec3State*> getVec3StateRefByTag(WorldInfo& worldInfo, std::set<int> tags);

void printWorldInfo(WorldInfo& worldInfo);

#endif