#include "./scene_routing.h"

extern CustomApiBindings* gameapi;

extern std::vector<SceneRouterOptions> routerPathOptions;
extern std::vector<SceneRouterPath> routerPaths;
std::optional<InteractState> modeInputOverride;

GameMode gamemodeByShortcutName(std::string shortcut);
struct LevelInfo {
  std::string filepath;
  std::vector<std::vector<std::string>> additionalTokens;
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
    defaultRouterOptions("gamemenu/elevatorcontrol/"),
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
struct RawLevelData {
  std::string name;
  std::string filepath;
  std::string description;
  std::string image;
  std::string shortcut;
  glm::vec3 ambientLight;
  glm::vec3 skyboxColor;
  std::string skybox;
  std::string audioClipPath;
  std::string mode;
  std::vector<std::vector<std::string>> additionalTokens;
};


// Kind of violating the cscript interface but...idk if i care...it's not really a game logic state thing, just utility
std::vector<std::string> listFilesWithExtensionsFromPackage(std::string folder, std::vector<std::string> extensions);
bool fileExistsFromPackage(std::string filepath);

MapData parseMapData(std::string file);
std::vector<Entity*> getEntitiesByClassName(MapData& mapData, const char* name);

std::vector<RawLevelData> getRawLevelData(){
  auto query = gameapi -> compileSqlQuery("select name, filepath, description, image, shortcut, ambient, skyboxcolor, skybox, audio, attr, mode from levels", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  std::vector<RawLevelData> levelData;
  for (auto& row : result){
    RawLevelData rawLevelData {
      .name = row.at(0),
      .filepath = row.at(1),
      .description = row.at(2),
      .image = row.at(3),
      .shortcut = row.at(4),
      .ambientLight = parseVec3(row.at(5)),
      .skyboxColor = parseVec3(row.at(6)),
      .skybox = row.at(7),
      .audioClipPath = row.at(8),
      .mode = row.at(10),
    };
    {
      std::string additionalTokensStr = row.at(9);
      auto rowData = split(additionalTokensStr, ';');
      std::vector<std::vector<std::string>> additionalTokens;
      for (auto& tokenStr: rowData){
        auto values = split(tokenStr, ':');
        modassert(values.size() == 3, "invalid number of attr in level");
        additionalTokens.push_back(values);
      }
      rawLevelData.additionalTokens = additionalTokens;
    }
    levelData.push_back(rawLevelData);
  }


  auto extraMaps = listFilesWithExtensionsFromPackage("../afterworld/scenes/levels/worlds/", { "rawscene" });
  for (auto& rawsceneFile : extraMaps){
    auto filePathData = decomposePath(rawsceneFile);
    auto levelPathData = decomposePath(filePathData.dirPath);
    auto worldPathData = decomposePath(levelPathData.dirPath);

    auto levelName = levelPathData.filename;
    auto worldName = worldPathData.filename;
    if (worldName == "core" || worldName == "debug"){
      continue;
    }
    auto imageName = filePathData.dirPath + "/map.png";
    auto image = fileExistsFromPackage(imageName) ? imageName : "./res/textures/wood.jpg";
    std::cout << "dyn image: " << imageName << ", exists = " << fileExistsFromPackage(imageName) << std::endl;

    auto mapName = filePathData.dirPath + "/map.map";
    bool mapExists = fileExistsFromPackage(mapName);
    std::cout << "dyn map: " << mapName << ", exists = " << mapExists << std::endl;
      
    glm::vec3 ambientLight(0.4f, 0.4f, 0.4f); 
    glm::vec3 skyboxColor(0.8f, 0.8f, 0.8f);
    std::string skybox("../gameresources/skybox/storm");
    std::string description("[no description]");

    if (mapExists){
      auto mapData = parseMapData(mapName);
      auto& entity = getEntityByName(mapData, "worldspawn");
      auto ambient = getVec3Value(entity, "ambient");
      if (ambient.has_value()){
        ambientLight = ambient.value();
      }
      auto skyboxcolor = getVec3Value(entity, "skyboxcolor");
      if (skyboxcolor.has_value()){
        skyboxColor = skyboxcolor.value();
      }

      auto skyboxAttr = getValue(entity, "skybox");
      if (skyboxAttr.has_value()){
        skybox = *skyboxAttr.value();
      }

      auto descriptionAttr = getValue(entity, "description");
      if (descriptionAttr.has_value()){
        description = *descriptionAttr.value();
      }
    }

    levelData.push_back(RawLevelData {
      .name = levelPathData.filename,
      .filepath = rawsceneFile,
      .description = description,
      .image = image,
      .shortcut = levelPathData.filename,
      .ambientLight = ambientLight,
      .skyboxColor = skyboxColor,
      .skybox = skybox,
      .audioClipPath = "../gameresources/sound/rain.wav",
      .mode = "ball",
      .additionalTokens = {},      
    });
  }


  return levelData;
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
        .audioClipPath = rawLevel.audioClipPath,
      };    
    }
  }
  ScenarioOptions defaultScenario {
    .ambientLight = glm::vec3(0.4f, 0.4f, 0.4f),
    .skyboxColor = glm::vec3(0.f, 0.f, 1.f),
    .skybox = "./res/textures/skyboxs/desert/",
    .audioClipPath = "",
  };
  return defaultScenario;
}