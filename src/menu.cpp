#include "./menu.h"

extern CustomApiBindings* gameapi;

void updateBackground(objid id, std::string image){
	if (!getSingleAttr(id, "background").has_value()){
		return;
	}
  setGameObjectTexture(id, image);
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
   	auto args = gameapi -> getArgs();
  	if (args.find("silent") == args.end()){
  		playMusicClip("&music", sceneId, std::nullopt, std::nullopt);
  	}
  	updateBackground(id, queryInitialBackground());
    return NULL;
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    if (key == "menu-background"){
    	auto newTexture = anycast<std::string>(value); 
    	modassert(newTexture, "menu-background invalid");
    	updateBackground(id, *newTexture);
    	updateQueryBackground(*newTexture);
    }
  };
  return binding;
}

