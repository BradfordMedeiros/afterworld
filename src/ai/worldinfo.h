#ifndef MOD_AFTERWORLD_WORLDINFO
#define MOD_AFTERWORLD_WORLDINFO

#include "../util.h"

//////////////////////////////////////////////////////////////////////////////////

enum STATE_TYPE_HINT { STATE_NOHINT, STATE_VEC3, STATE_ENTITY_POSITION, STATE_ID };
struct EntityPosition {
  objid id;
  glm::vec3 position;
};

struct AnyState {
  int symbol;
  int ownerId;
  std::set<int> tags;
  STATE_TYPE_HINT hint;
  std::any value;
};

struct WorldInfo {
  std::vector<AnyState> anyValues;
};

void updateState(WorldInfo& worldInfo, int symbol, std::any value, std::set<int> tags, STATE_TYPE_HINT typeHint, int ownerId);
std::optional<std::any> getState(WorldInfo& worldInfo, int symbol);
std::vector<std::any> getStateByTag(WorldInfo& worldInfo, std::set<int> tags);
void freeState(WorldInfo& worldInfo, objid ownerId);

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
  std::cout << "type: " << anyValue.type().name() << std::endl;

  return getAnyValue<T>(anyValue);
}

template <typename T>
std::vector<T> getStateByTag(WorldInfo& worldInfo, std::set<int> tags){
  std::vector<T> values;
  auto anyValues = getStateByTag(worldInfo, tags);
  for (auto &anyValue : anyValues){
    values.push_back(getAnyValue<T>(anyValue));
  }
  return values;
}
void printWorldInfo(WorldInfo& worldInfo);



#endif