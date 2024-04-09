#include "./weather.h"

// maybe the right way to do this is for this to dynamically create some billboards in front of the player?

extern CustomApiBindings* gameapi;

struct Weather {
	std::optional<objid> weatherEmitter;
};

void changeWeather(Weather& weather, std::string name, objid sceneId){
  if (weather.weatherEmitter.has_value()){
    gameapi -> removeByGroupId(weather.weatherEmitter.value());
  }
  weather.weatherEmitter = std::nullopt;
  if (name == "none"){
    return;
  }

  auto query = gameapi -> compileSqlQuery("select texture, duration, rate, particle-limit, tint from weather where name = ?", { name });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  std::map<std::string, GameobjAttributes> submodelAttributes;
  GameobjAttributes particleAttr {
    .stringAttributes = { 
      { "state", "enabled" },    
      { "+physics", "enabled" },
      { "+physics_type", "dynamic" },  
      { "+layer", "basicui" },    ///////
      { "+mesh", "../gameresources/build/primitives/plane_xy_1x1.gltf"},
      { "+texture", strFromFirstSqlResult(result, 0) },
    },
    .numAttributes = { 
      { "duration", floatFromFirstSqlResult(result, 1) },
      { "rate", floatFromFirstSqlResult(result, 2) },
      { "limit", floatFromFirstSqlResult(result, 3) },
    },
    .vecAttr = {  
      .vec3 = { 
        {"position", glm::vec3(0, 1.5f, -1.f) }, 
        {"+scale", glm::vec3(0.2f, 0.2f, 0.2f) },
        { "+physics_gravity", glm::vec3(0.f, -1.f, -1.f) },
        {"!position", glm::vec3(0.001f, 0.001f, 0.001f) },
        {"?position", glm::vec3(0.1f, 0.001f, 0.001f) },
      },  
      .vec4 = {
        {"+tint", vec4FromFirstSqlResult(result, 4)  },
      } 
    },
  };
  weather.weatherEmitter = gameapi -> makeObjectAttr(sceneId, std::string("+code-weather-") + uniqueNameSuffix(), particleAttr, submodelAttributes); 
}

CScriptBinding weatherBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
	binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Weather* weather = new Weather;
    weather -> weatherEmitter = std::nullopt;
    changeWeather(*weather, "none", sceneId);
    return weather;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Weather* weather = static_cast<Weather*>(data);
    delete weather;
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    if (key == "weather"){
      Weather* weather = static_cast<Weather*>(data);
      auto strValue = anycast<std::string>(value);
      modassert(strValue, "weather strValue is NULL");
      auto sceneId = gameapi -> listSceneId(id);
      changeWeather(*weather, *strValue, sceneId);
    }
  };
	 return binding;
}