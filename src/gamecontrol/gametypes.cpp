#include "./gametypes.h"

extern CustomApiBindings* gameapi;


void gametypesOnKey(GameTypes& gametypes, int rawKey, int rawScancode, int rawAction, int rawMods){
  if (gametypes.meta){
    gametypes.meta -> onKey(gametypes.gametype, rawKey, rawScancode, rawAction, rawMods);
  }
}

void changeGameType(GameTypes& gametypes, GameTypeInfo& gametype, const char* name, void* data){
  gametypes.name = name;
  gametypes.meta = gametype;
  gametypes.gametype = gametype.createGametype(data);
  gametypes.startTime = gameapi -> timeSeconds(false);
}

void changeGameTypeNone(GameTypes& gametypes){
  gametypes.name = "";
  gametypes.startTime = std::nullopt;
  gametypes.meta = std::nullopt;
  gametypes.gametype.reset();
}

GameTypes createGametypes(){
  GameTypes gametypes;
  gametypes.startTime = std::nullopt;
  gametypes.meta = std::nullopt;
  return gametypes;
}

void gametypesOnMessage(GameTypes& gametypes, std::string& key, std::any& value){
  if (gametypes.meta.has_value()){
    gametypes.meta.value().onEvent(gametypes.gametype, key, value);
  }
}

DebugConfig debugPrintGametypes(GameTypes& gametype){
  DebugConfig debugConfig { .data = {} };
  if (!gametype.meta){
    debugConfig.data.push_back({ "gametype", "no gametype" });
  }else{
    debugConfig.data.push_back({ "gametype", gametype.name });
  }
  return debugConfig;
}

void onGametypesFrame(GameTypes& gametypes){
  if (gametypes.meta.has_value()){
    gametypes.meta.value().onFrame(gametypes.gametype);
  }
}

std::optional<std::any*> getGametypeData(GameTypes& gametypes){
  if (!gametypes.meta.has_value()){
    return std::nullopt;
  }
  return &gametypes.gametype;
}