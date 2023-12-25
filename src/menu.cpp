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
	auto objAttr =  gameapi -> getGameObjectAttr(id);
	auto scrollSpeedAttr = getFloatAttr(objAttr, "scrollspeed");
	auto scrollSpeed = scrollSpeedAttr.has_value() ? scrollSpeedAttr.value() : 0.1f;

	auto currTime = gameapi -> timeSeconds(false);
	float delta = menuState.elapsedTime * scrollSpeed;
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

void updateBackground(objid id, std::string image){
 	GameobjAttributes attr {
		.stringAttributes = {
			{ "texture", image },
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

std::string queryInitialBackground(){
  auto query = gameapi -> compileSqlQuery("select background from session", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  return result.at(0).at(0);
}

void updateQueryBackground(std::string image){
  auto updateQuery = gameapi -> compileSqlQuery(
    std::string("update session set ") + "background = ?", { image }
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
  modassert(validSql, "error executing sql query");
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
  		gameapi -> playClip("&music", sceneId, std::nullopt, std::nullopt);
  	}
  	updateBackground(id, queryInitialBackground());
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

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
  	MenuState* menu = static_cast<MenuState*>(data);
    if (key == "menu-background"){
    	auto newTexture = anycast<std::string>(value); 
    	modassert(newTexture, "menu-background invalid");
    	updateBackground(id, *newTexture);
    	updateQueryBackground(*newTexture);
    }
  };
  return binding;
}

