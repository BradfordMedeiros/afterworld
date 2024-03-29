#include "./global.h"

extern CustomApiBindings* gameapi;

GlobalState global {
  .paused = true,
  .inGameMode = false,
  .showEditor = false,
  .showScreenspaceGrid = false,
  .showConsole = false,
  .xNdc = 0.f,
  .yNdc = 0.f,
  .selectedId = std::nullopt,

  .godMode = false,

  .xsensitivity = 1.f,
  .ysensitivity = 1.f,
  .invertY = false,
  .disableGameInput = false,
};

void updateMouse(){
  global.disableGameInput = false;
  if (!global.inGameMode){
    gameapi -> setWorldState({
      ObjectValue {
        .object = "mouse",
        .attribute = "cursor",
        .value = "normal",
      },
    });
  }else{
    if (global.showEditor || global.paused){
      global.disableGameInput = true;
      gameapi -> setWorldState({
        ObjectValue {
          .object = "mouse",
          .attribute = "cursor",
          .value = "normal",
        },
      });
    }else{
      gameapi -> setWorldState({
        ObjectValue {
          .object = "mouse",
          .attribute = "cursor",
          .value = "capture",
        },
      });    
    }
  }

  if (global.showEditor){
    gameapi -> setWorldState({ 
      ObjectValue {
        .object = "editor",
        .attribute = "disableinput",
        .value = "false",
      },
    });    
  }else{
    gameapi -> setWorldState({ 
      ObjectValue {
        .object = "editor",
        .attribute = "disableinput",
        .value = "true",
      },
    });  
  }
}

void setPaused(bool paused){
  modlog("paused toggle", std::string("paused state: ") + print(paused));
  global.paused = paused;
  gameapi -> setWorldState({
   ObjectValue {
     .object = "world",
     .attribute = "paused",
     .value = paused ? "true" : "false",
   }
  });
  updateMouse();
}

bool isPaused(){
	return global.paused;
}

void enterGameMode(){
  global.inGameMode = true;
  setPaused(false);
  updateMouse();
}
void exitGameMode(){
  global.inGameMode = false;
  setPaused(true);
  updateMouse();
}

GlobalState& getGlobalState(){
  return global;
}

bool queryShowEditor(){
  auto query = gameapi -> compileSqlQuery("select editor from session", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  return result.at(0).at(0) == "true";
}

void updateShowEditor(bool showEditor){
  modlog("update show editor", std::to_string(showEditor));
  global.showEditor = showEditor;
  updateMouse();
}

void initGlobal(){
  auto args = gameapi -> getArgs();
  if (args.find("godmode") != args.end()){
    global.godMode = args.at("godmode") == "true";
  }
  updateShowEditor(queryShowEditor());
}

