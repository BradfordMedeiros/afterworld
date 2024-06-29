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
  GameTypeInfo* meta;
};

GameTypes createGametypes();
void gametypesOnMessage(GameTypes& gametypes, std::string& key, std::any& value);
void gametypesOnFrame(GameTypes& gametype);
void gametypesOnKey(GameTypes& gametypes, int key, int action);

#endif 