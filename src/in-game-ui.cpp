#include "./in-game-ui.h"

extern CustomApiBindings* gameapi;

struct GameUiBinding {
	std::vector<objid> ids;
};

bool isInGameUi(GameUiBinding& uiBinding, objid id){
	for (auto uiObjId : uiBinding.ids){
		if (uiObjId == id){
			return true;
		}
	}
	return false;
}

//void (*freeTexture)(std::string name, objid ownerId);

void createInGamesUiInstances(objid id){
	auto uiTexture = gameapi -> createTexture("gentexture-ingame-ui-texture", 512, 512, id);
	gameapi -> clearTexture(uiTexture, std::nullopt, std::nullopt, "../gameresources/textures/controls/up-down.png");

}

CScriptBinding inGameUiBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    GameUiBinding* uiBinding = new GameUiBinding;
    uiBinding -> ids = { id };
    createInGamesUiInstances(id);
  	return uiBinding;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    GameUiBinding* gameUi = static_cast<GameUiBinding*>(data);
    delete gameUi;
  };

  /*binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
 	
  };
  binding.onObjectHover = [](int32_t id, int32_t index, bool hoverOn) -> void {

  };

  binding.onObjectSelected = [](int32_t id, int32_t index, glm::vec3 color) -> void {

  };

  binding.onMapping = [](int32_t id, int32_t index) -> void {

  };*/

	return binding;
}

