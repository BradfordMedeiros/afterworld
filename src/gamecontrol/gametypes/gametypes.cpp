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
  modlog("gametypes", std::string("on message: ") + key);
  if (gametypes.meta.has_value()){
    for (auto &event : gametypes.meta.value().events){
      if (key == event){
        bool gameFinished = gametypes.meta.value().onEvent(gametypes.gametype, event, value);
        if (gameFinished){
          changeGameTypeNone(gametypes);
          return;
        }
      }
    }
  }
}


DebugConfig debugPrintGametypes(GameTypes& gametype){
  DebugConfig debugConfig { .data = {} };
  if (!gametype.meta){
    debugConfig.data.push_back({ "gametype", "no gametype" });
  }else{
    debugConfig.data.push_back({ "gametype", gametype.meta -> getDebugText(gametype.gametype) });
  }
  return debugConfig;
}

std::optional<GametypeData> getGametypeData(GameTypes& gametypes){
  if (!gametypes.meta.has_value()){
    return std::nullopt;
  }
  if (!gametypes.startTime.has_value()){
    return std::nullopt;
  }

  auto scoreInfo = gametypes.meta.value().getScoreInfo(gametypes.gametype, gametypes.startTime.value());
  return scoreInfo;
}

void onGametypesFrame(GameTypes& gametypes){
  if (gametypes.meta.has_value()){
    gametypes.meta.value().onFrame(gametypes.gametype);
  }
}