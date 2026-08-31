#include "./scene_routing.h"

extern CustomApiBindings* gameapi;

extern std::vector<SceneRouterOptions> routerPathOptions;
extern std::vector<SceneRouterPath> routerPaths;
std::optional<InteractState> modeInputOverride;

GameMode gamemodeByShortcutName(std::string shortcut);
struct LevelInfo {
  std::string filepath;
  std::vector<std::vector<std::string>> additionalTokens;

  std::optional<std::string> additionalFilepath;
};
std::optional<LevelInfo> levelByShortcutName(std::string shortcut);
ScenarioOptions scenarioOptionsByShortcutName(std::string shortcut);



std::function<InteractState()> basicInteract(bool paused, bool inGameMode, bool showMouse){
  return [paused, inGameMode, showMouse]() -> InteractState {
    return InteractState {
      .paused = paused,
      .inGameMode = inGameMode,
      .showMouse = showMouse,
    };
  };
}

std::function<InteractState()> withDefaults(std::function<InteractState()> interact){
  return [interact]() -> InteractState {
    if (getGlobalState().systemConfig.showConsole){
      return InteractState {
        .paused = true,
        .inGameMode = false,
        .showMouse = true,
      };
    }
    if (getGlobalState().showEditor){
       return InteractState {
        .paused = false,
        .inGameMode = true,
        .showMouse = true,
      };     
    }
    return interact();
  };
}


SceneRouterOptions defaultRouterOptions(std::string path){
  SceneRouterOptions options {
    .paths = { 
      PathAndParams { .path = path }, 
    },
    .getInteract = basicInteract(false, false, true),
  };
  return options;
}


std::optional<SceneRouterOptions*> getRouterOptions(std::string& path, int * _index){
  *_index = 0;
  for (int i = 0; i < routerPathOptions.size(); i++){
    auto &routerOptions = routerPathOptions.at(i);
    for (int j = 0; j < routerOptions.paths.size(); j++){
      auto pathMatch = matchPath(path, routerOptions.paths.at(j).path);
      if (pathMatch.matches){
        *_index = i;
        return &routerOptions;
      }
    }
  }
  return std::nullopt;
}


std::optional<SceneRouterPath*> getSceneRouter(std::string& path, int* _index, std::vector<std::string>* _params){
  *_index = 0;
  for (int i = 0; i < routerPaths.size(); i++){
    auto &router = routerPaths.at(i);
    for (int j = 0; j < router.paths.size(); j++){
      auto pathMatch = matchPath(path, router.paths.at(j));
      if (pathMatch.matches){
        *_index = i;
        *_params = pathMatch.params;
        return &router;
      }
    }
  }
  return std::nullopt;
}

void inputOverride(bool paused, bool showMouse){
  modeInputOverride = InteractState {
    .paused = paused,
    .inGameMode = true,
    .showMouse = showMouse,
  };
}
void inputOverride(){
  modeInputOverride = std::nullopt;
}

std::vector<SceneRouterOptions> routerPathOptions = {
    SceneRouterOptions {
      .paths = { 
        PathAndParams { .path = "mainmenu/" }, 
        PathAndParams { .path = "mainmenu/levelselect/" }, 
        PathAndParams { .path = "mainmenu/settings/" }, 
      },
      .getInteract = withDefaults(basicInteract(false, false, true)),
    },
    SceneRouterOptions {
      .paths = {  
        PathAndParams { .path = "playing/*/" }, 
      },
      .getInteract = withDefaults([]() -> InteractState {
        if (modeInputOverride.has_value()){
          return modeInputOverride.value();
        }
        return InteractState {
            .paused = getGlobalState().userRequestedPause,
            .inGameMode = true,
            .showMouse = false,
        };
      }),
    },
    SceneRouterOptions {
      .paths = { 
        PathAndParams { .path = "mainmenu/modelviewer/" },  
        PathAndParams { .path = "mainmenu/particleviewer/" },
      },
      .getInteract = withDefaults(basicInteract(false, false, true)),
    },
    SceneRouterOptions {
      .paths = { 
        PathAndParams { .path = "loading/" },  
      },
      .getInteract = withDefaults(basicInteract(true, false, true)),
    },
    defaultRouterOptions("debug/"),
    defaultRouterOptions("debug/wheel/"),
};

std::vector<SceneRouterPath> routerPaths = {
  SceneRouterPath {
    .paths = { "mainmenu/", "mainmenu/levelselect/", "mainmenu/settings/", "debug/wheel/" },
    .scene = [](std::vector<std::string> params) -> SceneLoadInfo { 
      return SceneLoadInfo {
        .sceneFile = "../afterworld/scenes/menu.rawscene",
        .additionalTokens = {},
      };
    },
    .scenarioOptions = std::nullopt,
  },
  SceneRouterPath {
    .paths = { "playing/*/" },
    .scene = [](std::vector<std::string> params) -> SceneLoadInfo {
      auto sceneFile = levelByShortcutName(params.at(0));
      modassert(sceneFile.has_value(), std::string("no scene file for: ") + params.at(0));
      return SceneLoadInfo {
        .sceneFile = sceneFile.value().filepath,
        .additionalTokens = sceneFile.value().additionalTokens,
      };
    },
    .additionalScene = [](std::vector<std::string> params) -> std::optional<SceneLoadInfo> {
      auto sceneFile = levelByShortcutName(params.at(0));
      modassert(sceneFile.has_value(), std::string("no scene file for: ") + params.at(0));

      std::cout << "file add: " << print(sceneFile.value().additionalFilepath) << std::endl;
      if (!sceneFile.value().additionalFilepath.has_value()){
        return std::nullopt;
      }
      return SceneLoadInfo {
        .sceneFile = sceneFile.value().additionalFilepath.value(),
      };
    },
    .scenarioOptions = [](std::vector<std::string> params) -> ScenarioOptions {
      return scenarioOptionsByShortcutName(params.at(0));
    },
    .getGameMode = [](std::vector<std::string> params) -> GameMode {
      return gamemodeByShortcutName(params.at(0));
    },
  },
  SceneRouterPath {
    .paths = { "loading/" },
    .scene = [](std::vector<std::string> params) -> SceneLoadInfo { 
      return SceneLoadInfo {
        .sceneFile = "../afterworld/scenes/loading.rawscene",
        .additionalTokens = {},
      };
    },
    .scenarioOptions = std::nullopt,
    .getGameMode = [](std::vector<std::string> params) -> GameMode {
      return GameModeFps{};
    }
  },
  SceneRouterPath {
    .paths = { "mainmenu/modelviewer/" },
    .scene = [](std::vector<std::string> params) -> SceneLoadInfo { 
      return SceneLoadInfo {
        .sceneFile = "../afterworld/scenes/dev/models.rawscene",
        .additionalTokens = {},
      };
    },
    .scenarioOptions = std::nullopt,
    .getGameMode = [](std::vector<std::string> params) -> GameMode {
      return GameModeFps {
        .makePlayer = false,
        .player = "maincamera",
      };
    }
  },
  SceneRouterPath {
    .paths = { "mainmenu/particleviewer/" },
    .scene = [](std::vector<std::string> params) -> SceneLoadInfo { 
      return SceneLoadInfo {
        .sceneFile = "../afterworld/scenes/dev/particles.rawscene",
        .additionalTokens = {},
      };
    },
    .scenarioOptions = std::nullopt,
    .getGameMode = [](std::vector<std::string> params) -> GameMode {
      return GameModeFps {
        .makePlayer = false,
        .player = "maincamera",
      };
    }
  },
};


//////////////////////////////////// LEVEL QUERY CODE ////////////////////////////////////


// Kind of violating the cscript interface but...idk if i care...it's not really a game logic state thing, just utility
std::vector<std::string> listFilesWithExtensionsFromPackage(std::string folder, std::vector<std::string> extensions);
bool fileExistsFromPackage(std::string filepath);

MapData parseMapData(std::string file);
std::vector<Entity*> getEntitiesByClassName(MapData& mapData, const char* name);

std::vector<RawLevelData> getRawLevelData(){
  std::vector<RawLevelData> levelData;

  auto extraMaps = listFilesWithExtensionsFromPackage("../afterworld/scenes/levels/worlds/", { "rawscene" });
  for (auto& rawsceneFile : extraMaps){
    auto filePathData = decomposePath(rawsceneFile);
    auto levelPathData = decomposePath(filePathData.dirPath);
    auto worldPathData = decomposePath(levelPathData.dirPath);

    auto levelName = levelPathData.filename;
    auto worldName = worldPathData.filename;
 
    auto imageName = filePathData.dirPath + "/map.png";
    auto image = fileExistsFromPackage(imageName) ? imageName : "./res/textures/wood.jpg";
    std::cout << "dyn image: " << imageName << ", exists = " << fileExistsFromPackage(imageName) << std::endl;

    auto mapName = filePathData.dirPath + "/map.map";
    bool mapExists = fileExistsFromPackage(mapName);
    std::cout << "dyn map: " << mapName << ", exists = " << mapExists << std::endl;
      
    std::optional<std::string> additionalSceneFilepath = filePathData.dirPath + "/items.rawscene2";
    bool additionalSceneExists = fileExistsFromPackage(additionalSceneFilepath.value());
    if (!additionalSceneExists){
      additionalSceneFilepath = std::nullopt;
    }

    std::string configFile = filePathData.dirPath + "/config.json";
    bool configExists = fileExistsFromPackage(configFile);
    

    glm::vec3 ambientLight(0.4f, 0.4f, 0.4f); 
    glm::vec3 skyboxColor(1.f, 1.f, 1.f);
    std::string skybox("../gameresources/skybox/storm");
    std::string description("[no description]");
    std::string mode("ball");
    std::optional<std::string> weather;
    glm::vec2 chromatic(0.f, 0.f);

    if (configExists){
      bool success = true;
      auto data = gameapi -> loadFromJsonFile2(configFile, &success, false);
      modassert(success, "error parsing json");

      if (data.find("ambient") != data.end()){
        auto ambientPtr = std::get_if<std::vector<float>>(&data.at("ambient"));
        modassert(ambientPtr -> size() == 3, std::string("unexpected ambientPtr value, got size = ") + std::to_string(ambientPtr -> size()));
        ambientLight = glm::vec3(ambientPtr -> at(0), ambientPtr -> at(1), ambientPtr -> at(2));
      }
      if (data.find("skyboxcolor") != data.end()){
        auto skyboxColorPtr = std::get_if<std::vector<float>>(&data.at("skyboxcolor"));
        modassert(skyboxColorPtr -> size() == 3, std::string("unexpected skybox value, got size = ") + std::to_string(skyboxColorPtr -> size()));
        skyboxColor = glm::vec3(skyboxColorPtr -> at(0), skyboxColorPtr -> at(1), skyboxColorPtr -> at(2));
      }
      if (data.find("skybox") != data.end()){
        auto skyboxPtr = std::get_if<std::string>(&data.at("skybox"));
        skybox = *skyboxPtr; 
      }
      if (data.find("description") != data.end()){
        auto descriptionPtr = std::get_if<std::string>(&data.at("description"));
        description = *descriptionPtr;
      }
      if (data.find("mode") != data.end()){
        auto modePtr = std::get_if<std::string>(&data.at("mode"));
        mode = *modePtr;
      }
      if (data.find("weather") != data.end()){
        auto weatherPtr = std::get_if<std::string>(&data.at("weather"));
        weather = *weatherPtr;   
      }
      if (data.find("mode") != data.end()){
        auto modePtr = std::get_if<std::string>(&data.at("mode"));
        mode = *modePtr;
      }

      if (data.find("chromatic") != data.end()){
        auto chromaticPtr = std::get_if<std::vector<float>>(&data.at("chromatic"));
        modassert(chromaticPtr -> size() == 2, std::string("unexpected chromaticPtr value, got size = ") + std::to_string(chromaticPtr -> size()));
        chromatic = glm::vec2(chromaticPtr -> at(0), chromaticPtr -> at(1));
      }

    }


    levelData.push_back(RawLevelData {
      .name = levelPathData.filename,
      .filepath = rawsceneFile,
      .additionalFilepath = additionalSceneFilepath,
      .description = description,
      .image = image,
      .shortcut = levelPathData.filename,
      .ambientLight = ambientLight,
      .skyboxColor = skyboxColor,
      .skybox = skybox,
      .weather = weather,
      .audioClipPath = "../gameresources/sound/rain.wav",
      .mode = mode,
      .chromatic = chromatic,
      .additionalTokens = {},

      .configFile = configFile,   
      .configFileExists = configExists,
    });
  }


  return levelData;
}

std::optional<RawLevelData> currLevelData(std::string levelName){
  auto allLevels = getRawLevelData();
  for (auto& level : allLevels){
    if (level.name == levelName){
      return level;
    }
  }
  return std::nullopt;
}

void updateRawLevelData(std::string levelName, UpdateLevel updateLevel){
  auto oldLevelData = currLevelData(levelName);
  if (oldLevelData.has_value()){

    auto level = oldLevelData.value();

  
    std::unordered_map<std::string, JsonType> data;
    if (level.configFileExists){
      bool success = true;
      data = gameapi -> loadFromJsonFile2(level.configFile, &success, false);
      modassert(success, "error parsing json");
    }

    if (updateLevel.description.has_value()){
      data["description"] = updateLevel.description.value();
    }
    if (updateLevel.skybox.has_value()){
      data["skybox"] = updateLevel.skybox.value();
    }

    if (updateLevel.ambient.has_value()){
      data["ambient"] = std::vector<float>({ updateLevel.ambient.value().r,  updateLevel.ambient.value().g, updateLevel.ambient.value().b });
    }

    if (updateLevel.chromatic.has_value()){
      data["chromatic"] = std::vector<float>({ updateLevel.chromatic.value().x,  updateLevel.chromatic.value().y });
    }

    if (updateLevel.skyboxColor.has_value()){
      data["skyboxcolor"] = std::vector<float>({ updateLevel.skyboxColor.value().r,  updateLevel.skyboxColor.value().g, updateLevel.skyboxColor.value().b });
    }

    if (updateLevel.weather.has_value()){
      data["weather"] =  updateLevel.weather.value();
    }else{
      data["weather"] = "default";
    }

    gameapi -> saveToJsonFile2(level.configFile, data);

  }

}

GameMode gamemodeByShortcutName(std::string shortcut){
  auto rawLevels = getRawLevelData();
  for (auto& rawLevel : rawLevels){
    if (rawLevel.shortcut == shortcut){
      if (rawLevel.mode == "none"){
        return GameModeNone{}; 
      }
      if (rawLevel.mode == "ball"){
        return GameModeBall{};
      }
      if (rawLevel.mode == "video"){
        return GameModeVideo{};
      }
      if (rawLevel.mode == "boot"){
        return GameModeBoot{};
      }
      if (rawLevel.mode == "fps"){
        return GameModeFps {
          .makePlayer = true,
          .player = "maincamera",
        };
      }
      break;
    }
  }

  return GameModeFps {
    .makePlayer = true,
    .player = "maincamera",
  };
}

std::vector<Level> loadLevels(){
  auto rawLevels = getRawLevelData();

  std::vector<Level> levels;
  for (auto& rawLevel : rawLevels){
    levels.push_back(Level {
      .scene = rawLevel.filepath,
      .name = rawLevel.name,
    });
  }
  return levels;
}

std::optional<LevelInfo> levelByShortcutName(std::string shortcut){
  auto rawLevels = getRawLevelData();
  for (auto& rawLevel : rawLevels){
    if (rawLevel.shortcut == shortcut){
      return LevelInfo {
        .filepath = rawLevel.filepath,
        .additionalTokens = rawLevel.additionalTokens,
        .additionalFilepath = rawLevel.additionalFilepath,
      };
    }
  }
  return std::nullopt;
}

std::vector<UILevel> queryLevels(){
  auto rawLevels = getRawLevelData();
  std::vector<UILevel> levels;
  for (auto& rawLevel : rawLevels){
    levels.push_back(UILevel{
      .name = rawLevel.name,
      .description = rawLevel.description,
      .image = rawLevel.image,
      .shortcut = rawLevel.shortcut,
    });
  }
  return levels;
}

ScenarioOptions scenarioOptionsByShortcutName(std::string shortcut){
  auto rawLevels = getRawLevelData();
  for (auto& rawLevel : rawLevels){
    if (rawLevel.shortcut == shortcut){
      return ScenarioOptions {
        .ambientLight = rawLevel.ambientLight,
        .skyboxColor = rawLevel.skyboxColor,
        .skybox = rawLevel.skybox,
        .weather = rawLevel.weather,
        .audioClipPath = rawLevel.audioClipPath,
        .chromatic = rawLevel.chromatic,

      };    
    }
  }
  ScenarioOptions defaultScenario {
    .ambientLight = glm::vec3(0.4f, 0.4f, 0.4f),
    .skyboxColor = glm::vec3(0.f, 0.f, 1.f),
    .skybox = "./res/textures/skyboxs/desert/",
    .audioClipPath = "",
    .chromatic = glm::vec2(0.f, 0.f),
  };
  return defaultScenario;
}