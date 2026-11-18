#include "./swinfo.h"

extern CustomApiBindings* gameapi;
extern GameTypes gametypeSystem;


GameTypeInfo simpleUi {
  .gametypeName = "simpleui",
  .createGametype = [](void* data) -> std::any {
  	return NULL;
  },
  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> void {},
  .onFrame = [](std::any& gametype) -> void {
 	std::cout << "sw info on frame" << std::endl;
  },
};

void startSimpleUiMode(objid sceneId){
	changeGameType(gametypeSystem, simpleUi, "simpleui", NULL);
}
