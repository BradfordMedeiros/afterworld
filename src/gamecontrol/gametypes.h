#ifndef MOD_AFTERWORLD_GAMETYPES
#define MOD_AFTERWORLD_GAMETYPES

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"

struct GameTypeInfo {
  std::string gametypeName;
  std::function<std::any(void*)> createGametype;
  std::function<void(std::any&, std::string& event, std::any& value)> onEvent;
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

void onGametypesFrame(GameTypes& gametypes);

#endif 