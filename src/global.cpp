#include "./global.h"

extern CustomApiBindings* gameapi;
extern GlobalState global;

GlobalState& getGlobalState(){
  return global;
}

bool isPaused(){
  return global.routeState.paused || global.userRequestedPause;
}

void toggleKeyboard(){
  global.systemConfig.showKeyboard = !global.systemConfig.showKeyboard;
  persistSave("settings", "show-keyboard", global.systemConfig.showKeyboard);
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
