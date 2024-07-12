#ifndef MOD_AFTERWORLD_GAMETYPES
#define MOD_AFTERWORLD_GAMETYPES

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "./modes/targetkill.h"
#include "./modes/deathmatch.h"
#include "./modes/race.h"

struct GameTypes  {
  std::string name;
  std::any gametype;
  std::optional<float> startTime;
  GameTypeInfo* meta;
};

GameTypes createGametypes();
void gametypesOnMessage(GameTypes& gametypes, std::string& key, std::any& value);
DebugConfig debugPrintGametypes(GameTypes& gametype);

void changeGameType(GameTypes& gametypes, const char* name);
std::optional<GametypeData> getGametypeData(GameTypes&);

#endif 