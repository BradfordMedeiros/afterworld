#ifndef MOD_AFTERWORLD_WORLDINFO
#define MOD_AFTERWORLD_WORLDINFO

#include "../util.h"

int getSymbol(std::string name);
std::string nameForSymbol(int symbol);

//////////////////////////////////////////////////////////////////////////////////

struct StateInfo {
  int symbol;
  std::vector<int> tags;
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


void updateBoolState(WorldInfo& worldInfo, std::string name, bool value);
void updateVec3State(WorldInfo& worldInfo, std::string name, glm::vec3 value, std::vector<int> tags = {});
std::optional<glm::vec3> getVec3State(WorldInfo& worldInfo, int symbol);
std::vector<glm::vec3> getVec3StateByTag(WorldInfo& worldInfo, std::vector<int> tags);

void printWorldInfo(WorldInfo& worldInfo);

#endif