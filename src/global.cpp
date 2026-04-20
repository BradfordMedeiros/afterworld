#include "./global.h"

extern CustomApiBindings* gameapi;

extern bool godMode;

void setActivePlayerEditorMode(bool, int);
int getDefaultPlayerIndex();
void persistSave(std::string scope, std::string key, JsonType value);

extern GlobalState global;

bool disableGameInput(){
  auto shouldDisable = global.systemConfig.showConsole || !global.routeState.inGameMode || global.showEditor || global.routeState.paused || global.showTerminal || global.isFreeCam;
  return shouldDisable;
}

void updateState(){
  global.showGameHud = !disableGameInput() && !global.disableHud;

  if (global.routeState.showMouse){
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

  if (global.showEditor){
    gameapi -> setWorldState({ 
      ObjectValue {
        .object = "editor",
        .attribute = "disableinput",
        .value = "false",
      },
    });    
  }else if (global.isFreeCam){
    gameapi -> setWorldState({ 
      ObjectValue {
        .object = "editor",
        .attribute = "disableinput",
        .value = "camera",
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

  gameapi -> setWorldState({
   ObjectValue {
     .object = "world",
     .attribute = "paused",
     .value = (global.routeState.paused || global.showEditor) ? "true" : "false",
   }
  });
}

void setPaused(bool paused){
  modlog("paused toggle", std::string("paused state: ") + print(paused));
  global.userRequestedPause = paused;
}

bool isPaused(){
	return global.userRequestedPause;
}

GlobalState& getGlobalState(){
  return global;
}


void setShowEditor(bool shouldShowEditor){
  modlog("update show editor", std::to_string(shouldShowEditor));
  global.showEditor = shouldShowEditor;
  persistSave("settings", "show-editor", shouldShowEditor);
}


void toggleKeyboard(){
  global.systemConfig.showKeyboard = !global.systemConfig.showKeyboard;
  persistSave("settings", "show-keyboard", global.systemConfig.showKeyboard);
}

void initGlobal(){
  auto args = gameapi -> getArgs();
  if (args.find("godmode") != args.end()){
    godMode = true;
  }
  global.showEditor = getSaveBoolValue("settings", "show-editor", false);
  setShowEditor(global.showEditor);
  setActivePlayerEditorMode(global.showEditor, getDefaultPlayerIndex());
  global.systemConfig.showKeyboard = getSaveBoolValue("settings", "show-keyboard", false);
}

bool queryConsoleCanEnable(){
  auto query = gameapi -> compileSqlQuery("select console from session", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  return result.at(0).at(0) == "true";
}
bool showConsole(){
  static bool canEnableConsole = queryConsoleCanEnable();
  if (!canEnableConsole){
   return false;
  }
  return getGlobalState().systemConfig.showConsole;
}

void setShowConsole(bool showConsole){
  static bool canEnableConsole = queryConsoleCanEnable();
  if (!canEnableConsole){
    return;
  }
  global.systemConfig.showConsole = showConsole;
}

void setShowTerminal(bool showTerminal){
  if (showTerminal == getGlobalState().showTerminal){
    return;
  }

  auto currTime = gameapi -> timeSeconds(false);
  if (currTime - getGlobalState().lastToggleTerminalTime > 0.1f){
    getGlobalState().lastToggleTerminalTime = currTime;
    getGlobalState().showTerminal = showTerminal;
  }
}

void setHudEnabled(bool enableHud){
  getGlobalState().disableHud = !enableHud;
}
