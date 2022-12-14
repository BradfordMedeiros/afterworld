#include "./menu.h"

extern CustomApiBindings* gameapi;

struct MenuState {
	float ypos;
	float redness;
	bool gettingred;
	float lastTime;
	float elapsedTime;
};

void scrollTexture(MenuState& menuState, objid id){
	auto currTime = gameapi -> timeSeconds(false);
	float delta = menuState.elapsedTime * 0.01f;
	float colorDelta = menuState.elapsedTime * 0.1f;
	menuState.elapsedTime = currTime - menuState.lastTime;
	menuState.lastTime = currTime;
	menuState.ypos = menuState.ypos + delta;
	if (menuState.ypos > 1){
		menuState.ypos = 0.f;
	}
	if (menuState.gettingred){
		menuState.redness = menuState.redness + colorDelta;
		if (menuState.redness > 1){
			menuState.gettingred = false;
		}
	}else{
		menuState.redness = menuState.redness - colorDelta;
		if (menuState.redness < 0.1){
			menuState.gettingred = true;
		}
	}

	auto textureOffsetStr = std::string("0 ") + std::to_string(menuState.ypos);
	GameobjAttributes attr {
		.stringAttributes = {
			{ "textureoffset", textureOffsetStr },
		},
		.numAttributes = {},
		.vecAttr = {
			.vec3 = {},
			.vec4 = {
				//{ "tint", glm::vec4(menuState.redness + 1, 0.4f, 0.4f, 1.f) },
			},
		},
	};
	gameapi -> setGameObjectAttr(id, attr);
}


CScriptBinding menuBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  	MenuState* state = new MenuState;
  	state -> ypos = 0.f;
  	state -> redness = 0.2f;
  	state -> gettingred = true;
  	state -> lastTime = gameapi -> timeSeconds(false);
  	state -> elapsedTime = 0;
   	auto args = gameapi -> getArgs();
  	if (args.find("silent") == args.end()){
  		gameapi -> playClip("&music", sceneId);
  	}
    return state;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    MenuState* value = (MenuState*)data;
    delete value;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
  	MenuState* menu = static_cast<MenuState*>(data);
  	scrollTexture(*menu, id);
  };

  return binding;
}
