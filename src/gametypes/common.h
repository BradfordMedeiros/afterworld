#ifndef MOD_AFTERWORLD_GAMETYPES_COMMON
#define MOD_AFTERWORLD_GAMETYPES_COMMON

#include <functional>
#include "../util.h"

struct GametypeData {
  const char* gametypeName;
  int score1;
  int score2;
  int totalScore;
  float remainingTime;
};

struct GameTypeInfo {
  std::string gametypeName;
  std::vector<std::string> events;
  std::function<std::any(void*)> createGametype;
  std::function<bool(std::any&, std::string& event, std::any& value)> onEvent;
  std::function<std::string(std::any&)> getDebugText;
  std::function<std::optional<GametypeData>(std::any& gametype, float startTime)> getScoreInfo;
}; 


#endif 