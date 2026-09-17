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