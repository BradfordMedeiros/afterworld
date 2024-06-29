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
}

GameTypes createGametypes(){
  GameTypes gametypes;
  gametypes.meta = NULL;
  changeGameType(gametypes, "deathmatch");
  return gametypes;
}

void gametypesOnMessage(GameTypes& gametypes, std::string& key, std::any& value){
  modlog("gametypes", std::string("on message: ") + key);
  if (key == "change-gametype"){
    auto gametypeName = anycast<std::string>(value);
    modassert(gametypeName, "gametypeName null");
    changeGameType(gametypes, gametypeName -> c_str());
    return;
  }
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
void gametypesOnFrame(GameTypes& gametype){
  if (!gametype.meta){
    gameapi -> drawText("gametype: no gametype", -0.9, 0.3, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  }else{
    gameapi -> drawText("gametype: " + gametype.meta -> getDebugText(gametype.gametype), -0.9, 0.3, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  }
}

void gametypesOnKey(GameTypes& gametypes, int key, int action){
  if (key == 'R') { 
    if (action == 1){
      changeGameType(gametypes, gametypes.name.c_str());
    }
  }
}
