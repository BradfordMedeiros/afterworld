#include "./gametypes.h"

extern CustomApiBindings* gameapi;

std::vector<GameTypeInfo> gametypes = {
  getTargetKill(),
  getDeathmatchMode(),
  getRaceMode(),
};

GameTypeInfo* gametypeByName(const char* name) {
  for (auto &gametype : gametypes){
    if (gametype.gametypeName == name){
      return &gametype;
    }
  }
  return NULL;
}

void changeGameType(GameTypes& gametypes, const char* name){
  gametypes.name = name;
  auto gametypeInfo = gametypeByName(name);
  if (!gametypeInfo){
    gametypes.meta = NULL;
    return;
  }
  gametypes.meta = gametypeInfo;
  gametypes.gametype = gametypeInfo -> createGametype();
  gametypes.startTime = gameapi -> timeSeconds(false);
}

GameTypes createGametypes(){
  GameTypes gametypes;
  gametypes.startTime = std::nullopt;
  gametypes.meta = NULL;
  return gametypes;
}

void gametypesOnMessage(GameTypes& gametypes, std::string& key, std::any& value){
  modlog("gametypes", std::string("on message: ") + key);
  if (gametypes.meta){
    for (auto &event : gametypes.meta -> events){
      if (key == event){
        bool gameFinished = gametypes.meta -> onEvent(gametypes.gametype, event, value);
        if (gameFinished){
          changeGameType(gametypes, "nogame");
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
  if (gametypes.meta == NULL){
    return std::nullopt;
  }
  if (!gametypes.startTime.has_value()){
    return std::nullopt;
  }

  auto scoreInfo = gametypes.meta -> getScoreInfo(gametypes.gametype, gametypes.startTime.value());
  return scoreInfo;
}