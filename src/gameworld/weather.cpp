#include "./weather.h"

// maybe the right way to do this is for this to dynamically create some billboards in front of the player?

extern CustomApiBindings* gameapi;

int getNumberOfPlayers();

objid createWeatherEffect(std::string path, std::optional<glm::vec3> scale, std::optional<glm::vec4> tint){
    auto sceneId = gameapi -> rootSceneId();
    GameobjAttributes emitterAttr { 
      .attr = {
        { "effekseer", path },
        { "state", "enabled" },
      } 
    };
    std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
    std::string emitterName = std::string("+emitterweather-") + std::to_string(getUniqueObjId());
    auto emitter = gameapi -> makeObjectAttr(sceneId, emitterName, emitterAttr, submodelAttributes);
    return emitter.value();
}

//    { "rain", EffekData { .path  = "./res/particles/rain2.efkefc", .scale = glm::vec3(0.4f, 0.4f, 0.4f) }},


struct WeatherEffect {
  std::string path;
  bool flash = false;
};

std::unordered_map<std::string, WeatherEffect> weatherEffects {
  { "rain", WeatherEffect { 
      .path =  "./res/particles/rain2.efkefc",
  }},
  { 
    "storm", WeatherEffect {
      .path =  "./res/particles/rain2.efkefc",
      .flash = true,
  }},

};

void changeWeather(Weather& weather, std::optional<std::string> name){
  if (weather.weatherEmitter.has_value()){
    gameapi -> removeByGroupId(weather.weatherEmitter.value());
  }
  weather.weatherEmitter = std::nullopt;

  if (!name.has_value()){
    std::cout << "weather: change to " << "none" << std::endl;
    return;
  }
  
  auto& effect = weatherEffects.at(name.value());
  weather.weatherEmitter = createWeatherEffect(effect.path, std::nullopt, std::nullopt);
  std::cout << "weather: change to " << name.value() << std::endl;
}

void onWeatherFrame(Weather& weather){
  auto numPlayers = getNumberOfPlayers();
  modassert(numPlayers == 1, "weather system does not support more than 1 player");

  if (!weather.weatherEmitter.has_value()){
    return;
  }
  auto cameraTransform = gameapi -> getCameraTransform(0);
  auto weatherEmitterId = weather.weatherEmitter.value();
  gameapi -> setGameObjectPosition(weatherEmitterId, cameraTransform.position, true, Hint { .hint = "update weather transform" });  


  static float lastLightingTime = gameapi -> timeSeconds(false);
  static bool inLighting = false;
  static glm::vec3 oldLighting(0.f, 0.f, 0.f);

  auto currTime = gameapi -> timeSeconds(false);
  if (currTime - lastLightingTime > 0.1f && inLighting){
    inLighting = false;
    gameapi -> setWorldState({ 
       ObjectValue {
         .object = "light",
         .attribute = "amount",
         .value = oldLighting,
       },
    });
  }

  if (currTime - lastLightingTime > 10.f){
    std::cout << "weather: lighting flash" << std::endl;
    lastLightingTime = currTime;
    inLighting = true;

    glm::vec3 ambientLight = glm::vec3(1.f, 1.f, 1.f);

    //std::optional<AttributeValue> getWorldStateAttr(const char* object, const char* attribute){
    auto ambientLightAttr = getWorldStateAttr("light", "amount");
    modassert(ambientLightAttr.has_value(), "no ambient light state");
    auto vec3Ptr = std::get_if<glm::vec3>(&ambientLightAttr.value());
    modassert(vec3Ptr, "ambient light not vec3");
    oldLighting = *vec3Ptr;

    gameapi -> setWorldState({ 
       ObjectValue {
         .object = "light",
         .attribute = "amount",
         .value = ambientLight,
       },
    });

  }
}