#ifndef MOD_AFTERWORLD_GAMETYPES
#define MOD_AFTERWORLD_GAMETYPES

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "./modes/targetkill.h"
#include "./modes/deathmatch.h"
#include "./modes/race.h"
#include "./modes/ball.h"

struct GameTypes  {
  std::string name;
  std::any gametype;
  std::optional<float> startTime;
  GameTypeInfo* meta;
};

GameTypes createGametypes();
void gametypesOnMessage(GameTypes& gametypes, std::string& key, std::any& value);
DebugConfig debugPrintGametypes(GameTypes& gametype);

void gametypesOnKey(GameTypes& gametypes, int rawKey, int rawScancode, int rawAction, int rawMods);

void changeGameType(GameTypes& gametypes, const char* name, void* data);
std::optional<GametypeData> getGametypeData(GameTypes&);

#endif 