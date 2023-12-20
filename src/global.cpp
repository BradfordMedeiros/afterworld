#include "./global.h"

extern CustomApiBindings* gameapi;

GlobalState global {
  .paused = true,
  .inGameMode = false,
  .showScreenspaceGrid = false,
  .showConsole = false,
  .xNdc = 0.f,
  .yNdc = 0.f,
  .selectedId = std::nullopt,

  .godMode = false,
};

void setPaused(bool paused){
  modlog("paused toggle", std::string("paused state: ") + print(paused));
  global.paused = paused;
  gameapi -> setWorldState({
   ObjectValue {
     .object = "world",
     .attribute = "paused",
     .value = paused ? "true" : "false",
   },
 });
}

bool isPaused(){
	return global.paused;
}

void enterGameMode(){
  global.inGameMode = true;
  setPaused(false);
}
void exitGameMode(){
  global.inGameMode = false;
  setPaused(true);
}

GlobalState& getGlobalState(){
  return global;
}

void initGlobal(){
  auto args = gameapi -> getArgs();
  if (args.find("godmode") != args.end()){
    global.godMode = args.at("godmode") == "true";
  }
}


