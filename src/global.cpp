#include "./global.h"

extern CustomApiBindings* gameapi;

GlobalState global {
  .paused = false,
};

void setPaused(bool paused){
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


