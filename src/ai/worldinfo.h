#ifndef MOD_AFTERWORLD_WORLDINFO
#define MOD_AFTERWORLD_WORLDINFO

#include "../util.h"

//////////////////////////////////////////////////////////////////////////////////


struct StateInfo {
  int symbol;
  std::set<int> tags;
  std::any data;
};

enum STATE_TYPE_HINT { STATE_NOHINT, STATE_VEC4, STATE_BOOL };
struct AnyState {
  StateInfo state;
  STATE_TYPE_HINT hint;
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
  std::vector<AnyState> anyStateValues;
  std::vector<BoolState> boolValues;
  std::vector<Vec3State> vec3Values;
};


void updateState(WorldInfo& worldInfo, int symbol, std::any value, std::set<int> tags = {}, STATE_TYPE_HINT typeHint = STATE_NOHINT);
std::optional<std::any> getState(WorldInfo& worldInfo, int symbol);
std::vector<std::any> getStateByTag(WorldInfo& worldInfo, std::set<int> tags);

// probably add templates for type convenience for above getters

//////////////////


void updateBoolState(WorldInfo& worldInfo, int symbol, bool value);
void updateVec3State(WorldInfo& worldInfo, int symbol, glm::vec3 value, std::set<int> tags = {}, std::any data = NULL);
std::optional<glm::vec3> getVec3State(WorldInfo& worldInfo, int symbol);
std::optional<Vec3State*> getVec3StateRef(WorldInfo& worldInfo, int symbol);

std::optional<bool> getBoolState(WorldInfo& worldInfo, int symbol);
std::vector<glm::vec3> getVec3StateByTag(WorldInfo& worldInfo, std::set<int> tags);
std::vector<Vec3State*> getVec3StateRefByTag(WorldInfo& worldInfo, std::set<int> tags);

void printWorldInfo(WorldInfo& worldInfo);

#endif