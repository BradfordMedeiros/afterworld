#ifndef MOD_AFTERWORLD_WORLDINFO
#define MOD_AFTERWORLD_WORLDINFO

#include "../util.h"

//////////////////////////////////////////////////////////////////////////////////


struct StateInfo {
  int symbol;
  std::set<int> tags;
  std::any data;
};

enum STATE_TYPE_HINT { STATE_NOHINT, STATE_VEC3 };
struct AnyState {
  StateInfo stateInfo;
  STATE_TYPE_HINT hint;
  std::any value;
};

struct Vec3State {
  StateInfo stateInfo;
  glm::vec3 value;
};

struct WorldInfo {
  std::vector<AnyState> anyValues;
  std::vector<Vec3State> vec3Values;
};


void updateState(WorldInfo& worldInfo, int symbol, std::any value, std::set<int> tags, STATE_TYPE_HINT typeHint);
std::optional<std::any> getState(WorldInfo& worldInfo, int symbol);
std::vector<std::any> getStateByTag(WorldInfo& worldInfo, std::set<int> tags);

template <typename T> 
T getAnyValue(std::any& anyValue){
  auto anyValuePtr = anycast<T>(anyValue);
  modassert(anyValuePtr, "get any value, invalid type provided");
  return *anyValuePtr;
}

template <typename T> 
std::optional<T> getState(WorldInfo& worldInfo, int symbol){
  auto anyStateValue = getState(worldInfo, symbol);
  if (!anyStateValue.has_value()){
    return std::nullopt;
  }
  auto anyValue = anyStateValue.value();
  return getAnyValue<T>(anyValue);
}

// probably add templates for type convenience for above getters

//////////////////


void updateVec3State(WorldInfo& worldInfo, int symbol, glm::vec3 value, std::set<int> tags = {}, std::any data = NULL);
std::vector<Vec3State*> getVec3StateRefByTag(WorldInfo& worldInfo, std::set<int> tags);

void printWorldInfo(WorldInfo& worldInfo);

#endif