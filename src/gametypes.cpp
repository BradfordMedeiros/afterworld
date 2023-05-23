#include "./gametypes.h"

extern CustomApiBindings* gameapi;

std::vector<GameTypeInfo> gametypes = {
  getTargetKill(),
  getDeathmatchMode(),
  getRaceMode(),
};

struct GameTypes  {
  std::string name;
  std::any gametype;
  GameTypeInfo* meta;
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

CScriptBinding gametypesBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  	GameTypes* gametype = new GameTypes;
    gametype -> meta = NULL;
    changeGameType(*gametype, "race");
    return gametype;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
  	GameTypes* gametype = static_cast<GameTypes*>(data);
    delete gametype;
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
    GameTypes* gametypes = static_cast<GameTypes*>(data);
    modlog("gametypes", std::string("on message: ") + key);
    if (key == "change-gametype"){
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "change-gametype attr value wrong type");
      changeGameType(*gametypes, strValue -> c_str());
      return;
    }

    if (gametypes -> meta){
      for (auto &event : gametypes -> meta -> events){
        if (key == event){
          bool gameFinished = gametypes -> meta -> onEvent(gametypes -> gametype, event, value);
          if (gameFinished){
            changeGameType(*gametypes, "nogame");
            return;
          }
        }
      }
    }
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    GameTypes* gametype = static_cast<GameTypes*>(data);
    if (!gametype -> meta){
      gameapi -> drawText("gametype: no gametype", -0.9, 0.3, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
    }else{
      gameapi -> drawText("gametype: " + gametype -> meta -> getDebugText(gametype -> gametype), -0.9, 0.3, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
    }
  };
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    GameTypes* gametype = static_cast<GameTypes*>(data);
    if (key == 'R') { 
      if (action == 1){
        changeGameType(*gametype, gametype -> name.c_str());
      }
    }
  };

  return binding;
}
