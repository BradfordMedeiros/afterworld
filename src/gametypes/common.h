#ifndef MOD_AFTERWORLD_GAMETYPES_COMMON
#define MOD_AFTERWORLD_GAMETYPES_COMMON

#include <functional>
#include "../util.h"

struct GameTypeInfo {
  std::string gametypeName;
  std::vector<std::string> events;
  std::function<std::any()> createGametype;
  std::function<bool(std::any&, std::string& event, AttributeValue value)> onEvent;
  std::function<std::string(std::any&)> getDebugText;
}; 


#endif 