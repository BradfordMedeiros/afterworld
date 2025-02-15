#include "./global.h"

extern CustomApiBindings* gameapi;
void pushHistoryParam(std::string);
void rmHistoryParam(std::string);
void setActivePlayerEditorMode(bool);


GlobalState global {  // static-state
  .showEditor = false,
  .isFreeCam = false,
  .showScreenspaceGrid = false,
  .showConsole = false,
  .showKeyboard = false,
  .showGameHud = false,
  .disableUiInput = false,
  .zoomIntoArcade = false,
  .showTerminal = false,
  .lastToggleTerminalTime = 0.f,
  .xNdc = 0.f,
  .yNdc = 0.f,
  .mouseVelocity = glm::vec2(0.f, 0.f),
  .texCoordUv = glm::vec2(0.f, 0.f),
  .texCoordUvView = glm::vec2(0.f, 0.f),
  .selectedId = std::nullopt,
  .lookAtId = std::nullopt,

  .godMode = false,

  .xsensitivity = 1.f,
  .ysensitivity = 1.f,
  .invertY = false,
  .disableGameInput = false,

  .leftMouseDown = false,
  .rightMouseDown = false,
  .middleMouseDown = false,

  .routeState = RouteState {
    .paused = true,
    .inGameMode = false,
    .showMouse = true,
  },
};


DebugConfig debugPrintGlobal(){
  DebugConfig debugConfig { .data = {} };
  debugConfig.data.push_back({ "showEditor", print(global.showEditor) });
  debugConfig.data.push_back({ "showConsole", print(global.showConsole) });
  debugConfig.data.push_back({ "showKeyboard" , print(global.showKeyboard), DebugItem {
    .text = "TOGGLE",
    .onClick = []() -> void {
      toggleKeyboard();
    },
  }});
  debugConfig.data.push_back({ "showGameHud" , print(global.showGameHud) });
  debugConfig.data.push_back({ "showTerminal" , print(global.showTerminal) });
  debugConfig.data.push_back({ "disableGameInput", print(global.disableGameInput) });
  debugConfig.data.push_back({ "routeState.paused", print(global.routeState.paused) });
  debugConfig.data.push_back({ "routeState.inGameMode", print(global.routeState.inGameMode) });
  debugConfig.data.push_back({ "routeState.showMouse", print(global.routeState.showMouse) });
  return debugConfig;
}


void updateState(){
  global.disableGameInput = global.showConsole || !global.routeState.inGameMode || global.showEditor || global.routeState.paused || global.showTerminal || global.isFreeCam;
  global.showGameHud = !global.disableGameInput;

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
     .value = global.routeState.paused ? "true" : "false",
   }
  });
}

void setPaused(bool paused){
  modlog("paused toggle", std::string("paused state: ") + print(paused));
  global.routeState.paused = paused;
  updateState();
}

bool isPaused(){
	return global.routeState.paused;
}

void enterGameMode(){
  modassert(false, "enter game mode not yet implemented");
}
void exitGameMode(){
  modassert(false, "exit game mode not yet implemented");
}

void setRouterGameState(RouteState routeState){
  global.routeState = routeState;
  updateState();
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

void queryUpdateShowEditor(bool showEditor){
  auto updateQuery = gameapi -> compileSqlQuery(
    std::string("update session set ") + "editor = ?", { (showEditor ? "true" : "false") }
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
  modassert(validSql, "error executing sql query");
}

void setShowEditor(bool shouldShowEditor){
  modlog("update show editor", std::to_string(shouldShowEditor));
  global.showEditor = shouldShowEditor;
  queryUpdateShowEditor(shouldShowEditor);

  if (shouldShowEditor){
    pushHistoryParam("editor");
  }else{
    rmHistoryParam("editor");
  }
}

void setShowFreecam(bool isFreeCam){
  global.isFreeCam = isFreeCam;
  updateState();
}

bool queryShowKeyboard(){
  auto query = gameapi -> compileSqlQuery("select keyboard from session", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  return result.at(0).at(0) == "true";
}
void queryUpdateShowKeyboard(bool showKeyboard){
  auto updateQuery = gameapi -> compileSqlQuery(
    std::string("update session set ") + "keyboard = ?" , { (showKeyboard ? "true" : "false") }
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
  modassert(validSql, "error executing sql query");
}

void toggleKeyboard(){
  global.showKeyboard = !global.showKeyboard;
  queryUpdateShowKeyboard(global.showKeyboard);
}


void initGlobal(){
  auto args = gameapi -> getArgs();
  if (args.find("godmode") != args.end()){
    global.godMode = true;
  }
  global.showEditor = queryShowEditor();
  setShowEditor(global.showEditor);
  setActivePlayerEditorMode(global.showEditor);
  global.showKeyboard = queryShowKeyboard();
  updateState();
}


bool leftMouseDown(){
  return global.leftMouseDown;
}
bool rightMouseDown(){
  return global.rightMouseDown;
}
bool middleMouseDown(){
  return global.middleMouseDown;
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
  return getGlobalState().showConsole;
}

void setShowConsole(bool showConsole){
  static bool canEnableConsole = queryConsoleCanEnable();
  if (!canEnableConsole){
    return;
  }
  global.showConsole = showConsole;
  if (showConsole){
    pushHistoryParam("console");
  }else{
    rmHistoryParam("console");
  }
  updateState();
}



void setShowTerminal(bool showTerminal){
  if (showTerminal){
    pushHistoryParam("terminal");
  }else{
    rmHistoryParam("terminal");
  }
  if (showTerminal == getGlobalState().showTerminal){
    return;
  }

  auto currTime = gameapi -> timeSeconds(false);
  if (currTime - getGlobalState().lastToggleTerminalTime > 0.1f){
    getGlobalState().lastToggleTerminalTime = currTime;
    getGlobalState().showTerminal = showTerminal;
    updateState();
  }
}

void setShowZoomArcade(bool zoomIn){
  getGlobalState().zoomIntoArcade = zoomIn;
  updateState();
}
