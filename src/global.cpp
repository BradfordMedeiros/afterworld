#include "./global.h"

extern CustomApiBindings* gameapi;

GlobalState global {
  .paused = true,
  .showScreenspaceGrid = false,
  .xNdc = 0.f,
  .yNdc = 0.f,
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

GlobalState& getGlobalState(){
  return global;
}


