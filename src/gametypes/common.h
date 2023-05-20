#ifndef MOD_AFTERWORLD_GAMETYPES_COMMON
#define MOD_AFTERWORLD_GAMETYPES_COMMON

#include <functional>
#include <variant>
#include "../util.h"

struct TargetKillMode;
typedef std::variant<TargetKillMode> GameType;

struct GameTypeInfo {
  std::string gametypeName;
  std::vector<std::string> events;
  std::function<GameType()> createGametype;
  std::function<bool(GameType&, std::string& event, AttributeValue value)> onEvent;
  std::function<std::string(GameType&)> getDebugText;
}; 


#endif 