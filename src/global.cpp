#include "./global.h"

extern CustomApiBindings* gameapi;

GlobalState global {
  .showEditor = false,
  .showScreenspaceGrid = false,
  .showConsole = false,
  .showGameHud = false,
  .showTerminal = false,
  .lastToggleTerminalTime = 0.f,
  .xNdc = 0.f,
  .yNdc = 0.f,
  .texCoordUv = glm::vec2(0.f, 0.f),
  .texCoordUvView = glm::vec2(0.f, 0.f),
  .selectedId = std::nullopt,

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

  .playerVelocity = glm::vec3(0.f, 0.f, 0.f),
};

std::string print(GlobalState& globalState){
  std::string value;
  value += "showEditor = " + print(globalState.showEditor) + "\n";
  value += "showConsole = " + print(globalState.showConsole) + "\n";
  value += "disableGameInput = " + print(globalState.disableGameInput) + "\n";

  value += "paused = " + print(globalState.routeState.paused) + "\n";
  value += "inGameMode = " + print(globalState.routeState.inGameMode) + "\n";
  value += "showMouse = " + print(globalState.routeState.showMouse) + "\n";

  return value;
}

void debugPrintGlobal(){
    const float fontSize = 0.02f;
    drawRightText("global\n---------------", -0.9, 0.9f - fontSize, fontSize, std::nullopt, std::nullopt);

    std::vector<std::string> valuesToPrint;
    valuesToPrint.push_back(std::string("showEditor : ") + print(global.showEditor));
    valuesToPrint.push_back(std::string("showConsole : ") + print(global.showConsole));
    valuesToPrint.push_back(std::string("showGameHud : ") + print(global.showGameHud));
    valuesToPrint.push_back(std::string("showTerminal : ") + print(global.showTerminal));
    valuesToPrint.push_back(std::string("disableGameInput : ") + print(global.disableGameInput));
    valuesToPrint.push_back(std::string("routeState.paused : ") + print(global.routeState.paused));
    valuesToPrint.push_back(std::string("routeState.inGameMode : ") + print(global.routeState.inGameMode));
    valuesToPrint.push_back(std::string("routeState.showMouse : ") + print(global.routeState.showMouse));



    int offset = 0;
    for (int i = 0; i < valuesToPrint.size(); i++){
      drawRightText(valuesToPrint.at(i), -0.9, 0.9f - ((offset + 3) * fontSize), fontSize, glm::vec4(0.8f, 0.8f, 0.8f, 1.f), std::nullopt);
      offset++;
    }
}


void updateState(){
  global.disableGameInput = global.showConsole || !global.routeState.inGameMode || global.showEditor || global.routeState.paused || global.showTerminal;
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
  }else{
    gameapi -> setWorldState({ 
      ObjectValue {
        .object = "editor",
        .attribute = "disableinput",
        .value = "true",
      },
    });  
  }


  if (!global.routeState.inGameMode){
    gameapi -> setWorldState({
      ObjectValue {
        .object = "mouse",
        .attribute = "cursor",
        .value = "normal",
      },
    });
  }else{
    if (global.showEditor || global.routeState.paused){
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

  gameapi -> setWorldState({
   ObjectValue {
     .object = "world",
     .attribute = "paused",
     .value = global.routeState.paused ? "true" : "false",
   }
  });

  std::cout << print(global) << std::endl;
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
    std::string("update session set ") + "editor = " + (showEditor ? "true" : "false"), {}
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
  modassert(validSql, "error executing sql query");
}

void setShowEditor(bool shouldShowEditor){
  modlog("update show editor", std::to_string(shouldShowEditor));
  global.showEditor = shouldShowEditor;
  queryUpdateShowEditor(shouldShowEditor);
  updateState();
}

void initGlobal(){
  auto args = gameapi -> getArgs();
  if (args.find("godmode") != args.end()){
    global.godMode = args.at("godmode") == "true";
  }
  global.showEditor = queryShowEditor();
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
  updateState();
}

void setShowTerminal(bool showTerminal){
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

void setPlayerVelocity(glm::vec3 velocity){
  getGlobalState().playerVelocity = velocity;
}

glm::vec3 getPlayerVelocity(){
  return getGlobalState().playerVelocity;
}