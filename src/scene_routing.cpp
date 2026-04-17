#include "./scene_routing.h"

extern CustomApiBindings* gameapi;


SceneRouterOptions defaultRouterOptions(std::string path){
  SceneRouterOptions options {
    .paths = { 
      PathAndParams { .path = path, .params = {} }, 
    },
    .paused = false,
    .inGameMode = false,
    .showMouse = true,
  };
  return options;
}

bool matchQueryParams(std::vector<std::string>& queryParams, std::vector<std::string>& routerParams){
  for (auto &routerParam : routerParams){
    bool foundMatch = false;
    for (auto &queryParam : queryParams){
      if (queryParam == routerParam){
        foundMatch = true;
        break;
      }
    }
    if (!foundMatch){
      return false;
    }
  }
  return true;
}

extern std::vector<SceneRouterOptions> routerPathOptions;
std::optional<SceneRouterOptions*> getRouterOptions(std::string& path, std::vector<std::string>& queryParams, int * _index){
  *_index = 0;
  for (int i = 0; i < routerPathOptions.size(); i++){
    auto &routerOptions = routerPathOptions.at(i);
    for (int j = 0; j < routerOptions.paths.size(); j++){
      auto pathMatch = matchPath(path, routerOptions.paths.at(j).path);
      auto queryParamsMatch = matchQueryParams(queryParams, routerOptions.paths.at(j).params);
      if (pathMatch.matches && queryParamsMatch){
        *_index = i;
        return &routerOptions;
      }
    }
  }
  return std::nullopt;
}

GameMode gamemodeByShortcutName(std::string shortcut){
  auto query = gameapi -> compileSqlQuery("select shortcut, mode from levels", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query gamemodeByShortcutName");
  for (auto row : result){
    auto shortcutResult = row.at(0);
    if (shortcutResult == shortcut){
      auto modeStr = row.at(1);
      if (modeStr == "none"){
        return GameModeNone{}; 
      }
      if (modeStr == "ball"){
        return GameModeBall{};
      }
      if (modeStr == "intro"){
        return GameModeIntro{};
      }
    }
  }
  return GameModeFps {
    .makePlayer = true,
    .player = "maincamera",
  };
}

struct LevelInfo {
  std::string filepath;
  std::vector<std::vector<std::string>> additionalTokens;
};
std::optional<LevelInfo> levelByShortcutName(std::string shortcut){
  auto query = gameapi -> compileSqlQuery("select filepath, shortcut, attr from levels", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  for (auto row : result){
    auto filepath = row.at(0);
    auto shortcutResult = row.at(1);
  
    std::string additionalTokensStr = row.at(2);

    auto rowData = split(additionalTokensStr, ';');

    std::vector<std::vector<std::string>> additionalTokens;
    for (auto& tokenStr: rowData){
      auto values = split(tokenStr, ':');
      modassert(values.size() == 3, "invalid number of attr in level");
      additionalTokens.push_back(values);
    }

    if (shortcutResult == shortcut){
      return LevelInfo {
        .filepath = filepath,
        .additionalTokens = additionalTokens,
      };
    }
  }
  return std::nullopt;
}

ScenarioOptions scenarioOptionsByShortcutName(std::string shortcut){
  auto query = gameapi -> compileSqlQuery("select shortcut, ambient, skyboxcolor, skybox, audio from levels", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  for (auto row : result){
    auto shortcutResult = row.at(0);
    if (shortcutResult == shortcut){
      return ScenarioOptions {
        .ambientLight = parseVec3(row.at(1)),
        .skyboxColor = parseVec3(row.at(2)),
        .skybox = row.at(3),
        .audioClipPath = row.at(4),
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

extern std::vector<SceneRouterPath> routerPaths;
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

std::vector<SceneRouterOptions> routerPathOptions = {
    SceneRouterOptions {
      .paths = { 
        PathAndParams { .path = "*/", .params = { "console" }}, 
        PathAndParams { .path = "*/*/", .params = { "console" }}, 
      },
      .paused = true,
      .inGameMode = false,
      .showMouse = true,
    },
    SceneRouterOptions {
      .paths = { 
        PathAndParams { .path = "mainmenu/", .params = {} }, 
        PathAndParams { .path = "mainmenu/levelselect/", .params = {} }, 
        PathAndParams { .path = "mainmenu/settings/", .params = {} }, 
      },
      .paused = false,
      .inGameMode = false,
      .showMouse = true,
    },
    SceneRouterOptions {
      .paths = {  
        PathAndParams { .path = "playing/*/", .params = { "editor" } }, 
        PathAndParams { .path = "playing/*/", .params = { "terminal" } }, 
        PathAndParams { .path = "playing/*/", .params = { "livemenu" } }, 
      },
      .paused = false,
      .inGameMode = true,
      .showMouse = true,
    },
    SceneRouterOptions {
      .paths = {  
        PathAndParams { .path = "playing/*/", .params = {} }, 
      },
      .paused = false,
      .inGameMode = true,
      .showMouse = false,
    },
    SceneRouterOptions {
      .paths = {  
        PathAndParams { .path = "playing/*/paused/", .params = {} }, 
        PathAndParams { .path = "playing/*/dead/", .params = {} },
      },
      .paused = true,
      .inGameMode = true,
      .showMouse = true,
    },
    SceneRouterOptions {
      .paths = { 
        PathAndParams { .path = "mainmenu/modelviewer/", .params = {} },  
        PathAndParams { .path = "mainmenu/particleviewer/", .params = {} },
      },
      .paused = false,
      .inGameMode = false,
      .showMouse = true,
    },
    SceneRouterOptions {
      .paths = { 
        PathAndParams { .path = "loading/", .params = {} },  
      },
      .paused = true,
      .inGameMode = false,
      .showMouse = true,
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
    .paths = { "playing/*/",  "playing/*/paused/", "playing/*/dead/"},
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