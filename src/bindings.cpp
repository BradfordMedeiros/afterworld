#include "./bindings.h"

CustomApiBindings* gameapi = NULL;


Weapons weapons = createWeapons();
Movement movement = createMovement();
Water water;
SoundData soundData;
GameTypes gametypeSystem;
AiData aiData;

struct ManagedScene {
  std::optional<objid> id; 
  int index;
  std::string path;
};
struct SceneManagement {
  std::vector<Level> levels;
  std::optional<std::string> loadedLevel;
  std::optional<ManagedScene> managedScene;
};

std::optional<ZoomOptions> zoomOptions;
void setTotalZoom(float multiplier){
  bool isZoomedIn = multiplier < 1.f;
  if (isZoomedIn){
    zoomOptions = ZoomOptions {
      .zoomAmount = static_cast<int>((1.f / multiplier)),
    };
  }else{
    zoomOptions = std::nullopt;
  }
  setZoom(multiplier, isZoomedIn);
  setZoomSensitivity(multiplier);
  playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
}

std::vector<Level> loadLevels(){
  auto query = gameapi -> compileSqlQuery("select filepath, name from levels", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  std::vector<Level> levels = {};
  for (auto &row : result){
    levels.push_back(Level {
      .scene = row.at(0),
      .name = row.at(1),
    });
  }
  return levels;
}

SceneManagement createSceneManagement(){
  return SceneManagement {
    .levels = loadLevels(),
    .loadedLevel = std::nullopt,
    .managedScene = std::nullopt,
  };
}

void goToLevel(SceneManagement& sceneManagement, std::string levelShortName){
  pushHistory({ "playing", levelShortName }, true);
}

std::optional<std::string> levelByShortcutName(std::string shortcut){
  auto query = gameapi -> compileSqlQuery("select filepath, shortcut from levels", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  for (auto row : result){
    auto filepath = row.at(0);
    auto shortcutResult = row.at(1);
    if (shortcutResult == shortcut){
      return filepath;
    }
  }
  return std::nullopt;
}


double downTime = 0;
void setPausedMode(bool shouldBePaused){
  setPaused(shouldBePaused);
  auto pausedPath = getPathParts(2);
  if (!shouldBePaused){
    if (pausedPath.has_value() && pausedPath.value() == "paused"){  // probably shouldn't have the ui routing information here
      popHistory();
    }
    downTime = gameapi -> timeSeconds(true);
  }else{
    auto playingPath = getPathParts(0);
    if (playingPath.has_value() && playingPath.value() == "playing"){
      pushHistory({ "paused" });
      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
    }
    downTime = gameapi -> timeSeconds(true);
  }
}

void togglePauseIfInGame(){
  bool paused = isPaused();
  setPausedMode(!paused);
}

void displayGameOverMenu(){
  auto playingPath = getPathParts(0);
  bool isPlaying = playingPath.has_value() && playingPath.value() == "playing";
  if (isPlaying){
    pushHistory({ "dead" });
  }
}


struct SceneRouterPath {
  std::vector<std::string> paths;
  std::optional<std::function<std::string(std::vector<std::string> params)>> scene;
  bool makePlayer;
  std::optional<std::string> camera;
};

struct PathAndParams {
  std::string path;
  std::vector<std::string> params;
};
struct SceneRouterOptions {
  std::vector<PathAndParams> paths;
  bool paused;
  bool inGameMode;
  bool showMouse;
};


std::vector<SceneRouterPath> routerPaths = {
  SceneRouterPath {
    .paths = { "mainmenu/", "mainmenu/levelselect/", "mainmenu/settings/" },
    .scene = [](std::vector<std::string> params) -> std::string { return "../afterworld/scenes/menu.rawscene"; },
    .makePlayer = false,
    .camera = std::nullopt,
  },
  SceneRouterPath {
    .paths = { "playing/*/",  "playing/*/paused/", "playing/*/dead/" },
    .scene = [](std::vector<std::string> params) -> std::string {
      auto sceneFile = levelByShortcutName(params.at(0));
      modassert(sceneFile.has_value(), std::string("no scene file for: ") + params.at(0));
      return sceneFile.value();
    },
    .makePlayer = true,
    .camera = ">maincamera",
  },
  SceneRouterPath {
    .paths = { "mainmenu/modelviewer/" },
    .scene = [](std::vector<std::string> params) -> std::string { return "../afterworld/scenes/dev/models.rawscene"; },
    .makePlayer = false,
    .camera = ">maincamera",
  },
  SceneRouterPath {
    .paths = { "mainmenu/particleviewer/" },
    .scene = [](std::vector<std::string> params) -> std::string { return "../afterworld/scenes/dev/particles.rawscene"; },
    .makePlayer = false,
    .camera = ">maincamera",
  },
};

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
};

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
  modassert(false, std::string("no router options for: ") + path);
  return std::nullopt;
}





objid createPrefab(objid sceneId, const char* prefab, glm::vec3 pos){
  GameobjAttributes attr = {
    .attr = {
      { "scene", prefab },
      { "position", pos },
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
  return gameapi -> makeObjectAttr(
    sceneId, 
    std::string("[spawned-instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  ).value();
}

void onSceneRouteChange(SceneManagement& sceneManagement, std::string& currentPath, std::vector<std::string>& queryParams){
  modlog("scene route", std::string("path is: ") + currentPath);

  int currentIndex = 0;
  std::vector<std::string> params;
  auto router = getSceneRouter(currentPath, &currentIndex, &params);
  modlog("scene route, router, has router = ", print(router.has_value()));
  modassert(router.has_value(), std::string("no router for path: ") + currentPath);

  int matchedRouterOption = 0;
  auto routerOptions = getRouterOptions(currentPath, queryParams, &matchedRouterOption);
  modlog("router", std::string("matched router option: ") + std::to_string(matchedRouterOption));
  setRouterGameState(RouteState{
    .paused = routerOptions.value() -> paused,
    .inGameMode = routerOptions.value() -> inGameMode,
    .showMouse = routerOptions.value() -> showMouse,
  });
  setPaused(routerOptions.value() -> paused);

  if (sceneManagement.managedScene.has_value()){
    if (router.has_value() && sceneManagement.managedScene.value().index == currentIndex){
      return;
    }else{
      modlog("scene route unload", sceneManagement.managedScene.value().path);
      if (sceneManagement.managedScene.value().id.has_value()){
        gameapi -> unloadScene(sceneManagement.managedScene.value().id.value());
      }
      sceneManagement.managedScene = std::nullopt;
    }
  }

  modassert(router.has_value(), "router not found");
  if (router.has_value()){
    std::optional<objid> sceneId;
    if (router.value() -> scene.has_value()){
      sceneId = gameapi -> loadScene(router.value() -> scene.value()(params), {}, std::nullopt, {});
    }
    sceneManagement.managedScene = ManagedScene {
      .id = sceneId,
      .index = currentIndex,
      .path = currentPath,
    };
    modlog("scene route load", sceneManagement.managedScene.value().path);
    if (sceneId.has_value()){
      if (router.value() -> makePlayer){
        auto playerLocationObj = gameapi -> getObjectsByAttr("playerspawn", std::nullopt, sceneId.value());
        modassert(playerLocationObj.size() > 0, "no initial spawnpoint");
        glm::vec3 position = gameapi -> getGameObjectPos(playerLocationObj.at(0), true);
        createPrefab(sceneId.value(), "../afterworld/scenes/prefabs/player.rawscene",  position);
      }
      if (router.value() -> camera.has_value()){
        auto cameraId = findObjByShortName(router.value() -> camera.value());
        modassert(cameraId.has_value(), "lonSceneRouteChange, no camera in scene to load");
        setActivePlayer(cameraId.value());      
      }
    }
  }


}


struct GameState {
  SceneManagement sceneManagement;
  MovementEntityData movementEntities;

  std::optional<std::string> dragSelect;
  std::optional<glm::vec2> selecting;

  HandlerFns uiCallbacks;
  UiContext uiContext;

  bool printInventory;
  bool printGlobal;
  bool printGametypes;
  bool printAi;
};

MovementEntityData movementEntityData {
  .movementEntities = {},
  .activeEntity = std::nullopt,
};

MovementEntityData& getMovementData(){
  return movementEntityData;
}

std::unordered_map<std::string, std::vector<TerminalDisplayType>> terminals {
  { "test", {
    TerminalImage {
        .image = "../gameresources/build/textures/moonman.jpg",
    },
    TerminalImageLeftTextRight {
      .image = "../gameresources/build/textures/moonman.jpg",
      .text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Blandit cursus risus at ultrices mi tempus. Quam viverra orci sagittis eu volutpat odio. Non consectetur a erat nam at lectus. Sed tempus urna et pharetra pharetra massa. Eu volutpat odio facilisis mauris sit amet massa vitae tortor. Purus non enim praesent elementum facilisis leo vel. Tellus rutrum tellus pellentesque eu tincidunt tortor aliquam. Eu lobortis elementum nibh tellus molestie nunc non. Arcu dui vivamus arcu felis. Aliquam vestibulum morbi blandit cursus risus at ultrices mi. Urna et pharetra pharetra massa massa ultricies mi. Feugiat sed lectus vestibulum mattis ullamcorper. Senectus et netus et malesuada. Feugiat vivamus at augue eget arcu. Suspendisse sed nisi lacus sed viverra tellus. In nulla posuere sollicitudin aliquam ultrices sagittis orci. Vulputate eu scelerisque felis imperdiet proin fermentum leo vel. Tellus id interdum velit laoreet id donec ultrices tincidunt arcu.",
    },
    TerminalText {
      .text = "This is another page of text",
    }
  }}
};

struct TerminalInterface {
  std::string name;
  int pageIndex;
  TerminalConfig terminalConfig;
};
std::optional<TerminalInterface> terminalInterface;

TerminalConfig getTerminalConfig(std::string name, int pageIndex){
  return TerminalConfig {
    .time = gameapi -> timeSeconds(false),
    .terminalDisplay = terminals.at(name).at(pageIndex),
  };
}

void showTerminal(std::optional<std::string> name){
  if (!name.has_value()){
    terminalInterface = std::nullopt;
    setShowTerminal(false);
    return;
  }
  int pageIndex = 0;
  terminalInterface = TerminalInterface {
    .name = name.value(),
    .pageIndex = pageIndex,
    .terminalConfig = getTerminalConfig(name.value(), pageIndex),
  };
  setShowTerminal(true);
}
void nextTerminalPage(){
  if (terminalInterface.has_value()){
    TerminalInterface& terminal = terminalInterface.value();
    auto& terminalConfigs = terminals.at(terminalInterface.value().name);
    if (terminal.pageIndex < terminalConfigs.size() - 1){
      terminal.pageIndex++;
    }else{
      showTerminal(std::nullopt);
      return;
    }
    terminal.terminalConfig = getTerminalConfig(terminalInterface.value().name, terminal.pageIndex);
  }
}
void prevTerminalPage(){
  if (terminalInterface.has_value()){
    TerminalInterface& terminal = terminalInterface.value();
    auto& terminalConfigs = terminals.at(terminalInterface.value().name);
    if (terminal.pageIndex > 0){
      terminal.pageIndex--;
    }
    terminal.terminalConfig = getTerminalConfig(terminalInterface.value().name, terminal.pageIndex);
  }
}



UiContext getUiContext(GameState& gameState){
  std::function<void()> pause = [&gameState]() -> void { 
    setPausedMode(true); 
  };
  std::function<void()> resume = []() -> void { 
    setPausedMode(false); 
  };
  UiContext uiContext {
   .isDebugMode = []() -> bool { 
    auto args = gameapi -> getArgs();
    bool debugUi = args.find("debug-ui") != args.end();
    if (debugUi){
      return true;
    }
    return getStrWorldState("editor", "debug").value() == "true"; 
   },
   .showEditor = []() -> bool {
    return getGlobalState().showEditor;
   },
   .showConsole = showConsole,
   .showScreenspaceGrid = []() -> bool { return getGlobalState().showScreenspaceGrid; },
   .showGameHud = []() -> bool { return getGlobalState().showGameHud; },
   .showTerminal = []() -> std::optional<TerminalConfig> {
      if (!terminalInterface.has_value()){
        return std::optional<TerminalConfig>(std::nullopt); 
      }
      return getGlobalState().showTerminal ? terminalInterface.value().terminalConfig : std::optional<TerminalConfig>(std::nullopt); 
   },
   .showZoomOverlay = []() -> std::optional<ZoomOptions> { 
      return zoomOptions; 
   },
   .getScoreConfig = []() -> std::optional<ScoreOptions> {
      auto gametypeData = getGametypeData(gametypeSystem);
      if (!gametypeData.has_value()){
        return std::nullopt;
      }
      ScoreOptions scoreOptions {
        .timeRemaining = gametypeData.value().remainingTime,
        .gametypeName = gametypeData.value().gametypeName,
        .score1 = gametypeData.value().score1,
        .score2 = gametypeData.value().score2,
        .totalScore = gametypeData.value().totalScore,
      };
      return scoreOptions;
   },
   .levels = LevelUIInterface {
      .goToLevel = [&gameState](Level& level) -> void {
        modassert(false, std::string("level ui goToLevel: ") + level.name);
        goToLevel(gameState.sceneManagement, level.name);
      },
      .goToMenu = [&gameState]() -> void {
        pushHistory({ "mainmenu" }, true);
      }
    },
    .pauseInterface = PauseInterface {
      .elapsedTime = []() -> float { return gameapi -> timeSeconds(true) - downTime; },
      .pause = pause,
      .resume = resume,
    },
    .worldPlayInterface = WorldPlayInterface {
      .isGameMode = []() -> bool { return getGlobalState().routeState.inGameMode; },
      .isPaused = isPaused,
      .enterGameMode = enterGameMode,
      .exitGameMode = exitGameMode,
      .pause = pause,
      .resume = resume,
      .saveScene = []() -> void {
        auto sceneId = activeSceneForSelected();
        modassert(sceneId.has_value(), "save scene - no active scene");
        gameapi -> saveScene(false /*include ids */, sceneId.value(), std::nullopt /* filename */);
      },
    },
    .listScenes = []() -> std::vector<std::string> { return gameapi -> listResources("scenefiles"); },
    .loadScene = [&gameState](std::string scene) -> void {
      modassert(false, "load scene not yet implemented");
      goToLevel(gameState.sceneManagement, scene);
    },
    .newScene = [](std::string sceneName) -> void {
      const std::string sceneFolder = "./res/scenes/";
      gameapi -> createScene(sceneFolder + sceneName + ".rawscene");
    },
    .resetScene = []() -> void {
      auto sceneId = activeSceneForSelected();
      modassert(sceneId.has_value(), "resetScene  - no active scene");
      gameapi -> resetScene(sceneId.value());
    },
    .activeSceneId = activeSceneForSelected,
    .showPreviousModel = []() -> void {
      gameapi -> sendNotifyMessage("prev-model", NULL);
    },
    .showNextModel = []() -> void {
      gameapi -> sendNotifyMessage("next-model", NULL);
    },
    .consoleInterface = ConsoleInterface {
      .setShowEditor = setShowEditor,
      .setBackground = [](std::string background) -> void {
        gameapi -> sendNotifyMessage("menu-background", background);
      },
      .goToLevel = [&gameState](std::optional<std::string> level) -> void {
        modlog("gotolevel", std::string("level loading: ") + level.value());
        pushHistory({ "playing", level.value() }, true);
      },
      .routerPush = [](std::string route, bool replace) -> void {
        pushHistory({ route }, replace);
      },
      .routerPop = []() -> void {
        popHistory();
      },
      .die = []() -> void {
        auto activePlayerId = getActivePlayerId();
        if (activePlayerId.has_value()){
          doDamageMessage(activePlayerId.value(), 10000.f);   
        }
      },
    },
  };
  return uiContext;
}


AIInterface aiInterface {
  .move = [](objid agentId, glm::vec3 targetPosition, float speed) -> void {
    setEntityTargetLocation(getMovementData(), agentId, MovementRequest {
      .position = targetPosition,
      .speed = speed,
    });
  },
};

CutsceneApi cutsceneApi {
  .showLetterBox = showLetterBox,
  .setCameraPosition = setTempViewpoint,
  .popTempViewpoint = popTempViewpoint,
};

void goBackMainMenu(){
  pushHistory({ "mainmenu" }, true);
}

void printDebugStr(std::vector<std::vector<std::string>>& debugStr){
  for (int i = 0; i < debugStr.size(); i++){
    auto& row = debugStr.at(i);
    auto rowStr = join(row, ' ');
    std::cout << "printing: " << rowStr << std::endl;
    gameapi -> drawText(rowStr, -0.9f, 0.9 + (i * -0.1), 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  }
}

CScriptBinding afterworldMainBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    GameState* gameState = new GameState;
    gameState -> sceneManagement = createSceneManagement();
    gameState -> selecting = std::nullopt;
    gameState -> uiCallbacks = HandlerFns {
      .handlerFns = {},
      .handlerFns2 = {},
      .inputFns = {},
    };
    gameState -> uiContext = {};
    initGlobal();
    gameState -> dragSelect = std::nullopt;
    gameState -> uiContext = getUiContext(*gameState);

    auto args = gameapi -> getArgs();
    if (args.find("dragselect") != args.end()){
      gameState -> dragSelect = args.at("dragselect");
      modlog("bindings", std::string("drag select value: ") + gameState -> dragSelect.value());
    }

    gameState -> printInventory = args.find("print-inventory") != args.end();
    gameState -> printGlobal = args.find("print-global") != args.end();
    gameState -> printGametypes = args.find("print-gametypes") != args.end();
    gameState -> printAi = args.find("print-ai") != args.end();

    initSettings();
    registerOnRouteChanged([gameState]() -> void {
      auto currentPath = fullHistoryStr();
      auto queryParams = historyParams();
      onSceneRouteChange(gameState -> sceneManagement, currentPath, queryParams);
      modlog("routing", std::string("scene route registerOnRouteChanged: , new route: ") + currentPath);
    });

    pushHistory({ "mainmenu" }, true);

    if (args.find("level") != args.end()){
      auto scene = levelByShortcutName(args.at("level"));
      if (scene.has_value()){
        goToLevel(gameState -> sceneManagement, scene.value());
      }else{
        modlog("routing", std::string("AFTERWORLD: no level found for shortcut: ") + args.at("level"));
      }
    }

    loadDialogTree();
    soundData = createSoundData(gameapi -> rootSceneId());
    gametypeSystem = createGametypes();
    aiData = createAiData();

    return gameState;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    removeAllMovementCores();  // is this pointless?
    delete gameState;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    gameapi -> idAtCoordAsync(getGlobalState().xNdc, getGlobalState().yNdc, false, [](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
      getGlobalState().selectedId = selectedId;
      getGlobalState().texCoordUv = texCoordUv;
      modlog("texcoord", print(glm::vec2(texCoordUv)));
    });

    gameapi -> idAtCoordAsync(0.f, 0.f, false, [](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
      getGlobalState().texCoordUvView = texCoordUv;
    });

    gameState -> uiCallbacks = handleDrawMainUi(gameState -> uiContext, getGlobalState().selectedId );

    if (gameState -> dragSelect.has_value() && gameState -> selecting.has_value()){
      //selectWithBorder(gameState -> selecting.value(), glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc));
    }

    onPlayerFrame();
    tickCutscenes(cutsceneApi, gameapi -> timeSeconds(true));
    auto ammoInfo = onWeaponsFrame(weapons);
    if (ammoInfo.has_value()){
      setUIAmmoCount(ammoInfo.value().currentAmmo, ammoInfo.value().totalAmmo);
    }
    onMovementFrame(movement);
    onFrameWater(water);
    onFrameAi(aiData);


    // debug
    if (gameState -> printGametypes){
      auto debugGametypeInfo = debugPrintGametypes(gametypeSystem);
      printDebugStr(debugGametypeInfo);
    }
    if (gameState -> printInventory){
      debugPrintInventory();
    }
    if (gameState -> printGlobal){
      auto globalDebug = debugPrintGlobal();
      printDebugStr(globalDebug);
    }
    if (gameState -> printAi){
      auto aiDebug = debugPrintAi(aiData);
      printDebugStr(aiDebug);
    }
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    auto hasInputKey = gameapi -> unlock("input", id);
    if (!hasInputKey){
      return;
    }
    if (action == 1){
      auto playerId = getActivePlayerId();
      if (isPauseKey(key)){
        togglePauseIfInGame();
      }
      if (isTeleportButton(key)){
        // this probably should be aware of the bounds, an not allow to clip into wall for example
        // maybe raycast down, and then set the position so it fits 
        auto teleportPosition = getTeleportPosition();
        if (playerId.has_value() && teleportPosition.has_value()){
          playGameplayClipById(getManagedSounds().soundObjId.value(), std::nullopt, std::nullopt);
          gameapi -> setGameObjectPosition(playerId.value(), teleportPosition.value().position, true);
          gameapi -> removeByGroupId(teleportPosition.value().id);
        }
      }
      if (isExitTerminalKey(key)){
        showTerminal(std::nullopt);
      }
      onMainUiKeyPress(gameState -> uiCallbacks, key, scancode, action, mods);
    }
    handleHotkey(key, action);


    if (key == '-' && action == 0){
      setActivePlayerNext();
    }

    onWeaponsKeyCallback(weapons, key, action);
    onMovementKeyCallback(movement, key, action);

    if (key == 'R' && action == 1) {
      changeGameType(gametypeSystem, "targetkill");
    }

    if (key == 'Q' && action == 0) { 
      printWorldInfo(aiData.worldInfo);
    }
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    GameState* gameState = static_cast<GameState*>(data);

    onWeaponsMessage(weapons, key);

    if (key == "reset"){
      pushHistory({ "mainmenu" }, true);
      return;
    }
    if (key == "reload-config:levels"){
      gameState -> sceneManagement.levels = loadLevels();
      return;
    }

    if (key == "selected"){  // maybe this logic should be somewhere else and not be in dialog
      auto gameObjId = anycast<objid>(value); 
      modassert(gameObjId, "selected value invalid");
      if (!gameapi -> getGameObjNameForId(*gameObjId).has_value()){
        return;
      }
      handleInteract(*gameObjId);
      return;
    }
    if (key == "switch"){ // should be moved
      auto strValue = anycast<std::string>(value); 
      modassert(strValue != NULL, "switch value invalid");
      handleSwitch(*strValue);
      return;
    }

    if (key == "alert"){
      auto strValue = anycast<std::string>(value); 
      if (strValue != NULL){
        sendUiAlert(*strValue);
        return;
      }

      auto charStrValue = anycast<const char*>(value);
      if (charStrValue != NULL){
        sendUiAlert(*charStrValue);
        return;
      }
      modassert(false, "send alert invalid value");
    }

    if (key == "spawn"){
      auto attrValue = anycast<AttributeValue>(value); 
      modassert(attrValue, "spawn value invalid");
      auto strValue = std::get_if<std::string>(attrValue);
      modassert(strValue, "spawn not string value");
      spawnFromAllSpawnpoints("red", strValue -> c_str());
    }

    if (key == "terminal" && !getGlobalState().showTerminal){
      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
      showTerminal("test");
    }

    if (key == "ammo"){
      auto itemAcquiredMessage = anycast<ItemAcquiredMessage>(value);
      modassert(itemAcquiredMessage != NULL, "ammo message not an ItemAcquiredMessage");
      deliverAmmoToCurrentGun(weapons, itemAcquiredMessage -> targetId, itemAcquiredMessage -> amount);
    }

    onCutsceneMessages(key);
    onDialogMessage(key, value);
    onMessageSound(soundData, gameapi -> rootSceneId(), key, value);
    gametypesOnMessage(gametypeSystem, key, value);
    onAiOnMessage(aiData, key, value);
  };

  binding.onCollisionEnter = [](objid id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal, float force) -> void {
    auto gameobj1Exists = gameapi -> gameobjExists(obj1); // this check shouldn't be necessary, is bug
    auto gameobj2Exists = gameapi -> gameobjExists(obj2);
    modassert(gameobj1Exists && gameobj2Exists, "collision enter: objs do not exist");
    handleCollision(obj1, obj2, "switch-enter", "switch-enter-key", "enter");
    handleDamageCollision(obj1, obj2);
    handleMomentumCollision(obj1, obj2, pos, normal, force);
    handleBouncepadCollision(obj1, obj2, normal);
    handleInventoryOnCollision(obj1, obj2);
    onCollisionEnterWater(water, obj1, obj2);
    onCollisionEnterSound(soundData, gameapi -> rootSceneId(), obj1, obj2, pos);
  };
  binding.onCollisionExit = [](objid id, void* data, int32_t obj1, int32_t obj2) -> void {
    auto gameobj1Exists = gameapi -> gameobjExists(obj1);
    auto gameobj2Exists = gameapi -> gameobjExists(obj2);
    modassert(gameobj1Exists && gameobj2Exists, "collision exit: objs do not exist");
    handleCollision(obj1, obj2, "switch-exit", "switch-exit-key", "exit");
    onCollisionExitWater(water, obj1, obj2);
  };

  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    //modlog("input", std::string("mouse move: ") + print(glm::vec2(xPos, yPos)));
    //modlog("input",  std::string("(xNdc, yNdc)") + print(glm::vec2(xNdc, yNdc)));
    getGlobalState().xNdc = xNdc;
    getGlobalState().yNdc = yNdc;
    onWeaponsMouseMove(weapons, xPos, yPos);
    onMovementMouseMoveCallback(movement, xPos, yPos);
  };

  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    onMainUiMousePress(gameState -> uiCallbacks, button, action, getGlobalState().selectedId);

    modlog("input", std::string("on mouse down: button = ") + std::to_string(button) + std::string(", action = ") + std::to_string(action));
    if (button == 1){
      if (action == 0){
        gameState -> selecting = std::nullopt;
        getGlobalState().rightMouseDown = false;
      }else if (action == 1){
        gameState -> selecting = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
        getGlobalState().rightMouseDown = true;
        if (false){
          raycastFromCameraAndMoveTo(getMovementData(), getActivePlayerId().value());
        }
        nextTerminalPage();
      }
    }else if (button == 0){
      if (action == 0){
        getGlobalState().leftMouseDown = false;
      }else if (action == 1){
        getGlobalState().leftMouseDown = true;
        prevTerminalPage();
      }
    }else if (button == 2){
      if (action == 0){
        getGlobalState().middleMouseDown = false;
      }else if (action == 1){
        getGlobalState().middleMouseDown = true;
      }
    }
    onWeaponsMouseCallback(weapons, button, action);
  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    onMainUiScroll(amount);
    onMovementScrollCallback(movement, amount);
  };
  binding.onObjectAdded = [](int32_t _, void* data, int32_t idAdded) -> void {
    maybeAddMovementEntity(getMovementData(), idAdded);
    onMainUiObjectsChanged();
    onAiObjectAdded(aiData, idAdded);
  };
  binding.onObjectRemoved = [](int32_t _, void* data, int32_t idRemoved) -> void {
    maybeRemoveMovementEntity(getMovementData(), idRemoved);
    onActivePlayerRemoved(idRemoved);
    onMainUiObjectsChanged();
    onWeaponsObjectRemoved(weapons, idRemoved);
    onObjectRemovedWater(water, idRemoved);
    onAiObjectRemoved(aiData, idRemoved);
  };

  return binding;
}

std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api){
  std::vector<CScriptBinding> bindings;
  gameapi = &api;
  bindings.push_back(afterworldMainBinding(api, "native/main"));
  bindings.push_back(daynightBinding(api, "native/daynight"));
  bindings.push_back(tagsBinding(api, "native/tags"));
  bindings.push_back(debugBinding(api, "native/debug"));
  bindings.push_back(weatherBinding(api, "native/weather"));
  bindings.push_back(modelviewerBinding(api, "native/modelviewer"));
  bindings.push_back(particleviewerBinding(api, "native/particleviewer"));
  return bindings;
} 

