#ifndef MOD_AFTERWORLD_GAMETYPES
#define MOD_AFTERWORLD_GAMETYPES

#include "../../../../ModEngine/src/cscript/cscript_binding.h"
#include "../../util.h"
#include "./common.h"

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