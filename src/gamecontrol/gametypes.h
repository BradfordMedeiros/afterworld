#ifndef MOD_AFTERWORLD_GAMETYPES
#define MOD_AFTERWORLD_GAMETYPES

#include "../../../ModEngine/src/cscript/cscript_binding.h"
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
  std::function<void(std::any& gametype, int rawKey, int rawScancode, int rawAction, int rawMods)> onKey = [](std::any& gametype, int rawKey, int rawScancode, int rawAction, int rawMods) -> void {};
  std::function<void(std::any& gametype)> onFrame = [](std::any&) -> void {};
}; 


struct GameTypes  {
  std::string name;
  std::any gametype;
  std::optional<float> startTime;
  std::optional<GameTypeInfo> meta;
};

GameTypes createGametypes();
void gametypesOnMessage(GameTypes& gametypes, std::string& key, std::any& value);
DebugConfig debugPrintGametypes(GameTypes& gametype);

void gametypesOnKey(GameTypes& gametypes, int rawKey, int rawScancode, int rawAction, int rawMods);

void changeGameType(GameTypes& gametypes, GameTypeInfo& gametype, const char* name, void* data);
void changeGameTypeNone(GameTypes& gametypes);

std::optional<GametypeData> getGametypeData(GameTypes&);

void onGametypesFrame(GameTypes& gametypes);

#endif 