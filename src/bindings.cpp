#include "./bindings.h"

CustomApiBindings* gameapi = NULL;


Weapons weapons{};
Movement movement = createMovement();
extern ControlledPlayer controlledPlayer;

Water water;
SoundData soundData;
GameTypes gametypeSystem;
AiData aiData;
Weather weather;
Waypoints waypoints {
  .waypoints = {},
};
Tags tags{};
std::optional<std::string> levelShortcutToLoad;

struct ManagedScene {
  std::optional<objid> id; 
  int index;
  std::string path;
  std::optional<std::string> sceneFile;
};
struct SceneManagement {
  std::vector<Level> levels;
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

bool disableAnimation = false;
bool disableTpsMesh = false;
bool validateAnimationControllerAnimations = true;

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
    .managedScene = std::nullopt,
  };
}

void goToLevel(std::string levelShortName){
  pushHistory({ "playing", levelShortName }, true);
}
void goToLink(std::string link){
  pushHistory({ "loading" }, true);
  gameapi -> schedule(0, true, 5000, NULL, [link](void*) -> void {
    goToLevel(link);
  });
}
void goBackMainMenu(){
  pushHistory({ "mainmenu" }, true);
}
void goToNextLevel(){
  if (canAdvanceProgress()){
    advanceProgress();
    auto nextLink = getCurrentLink();
    modassert(nextLink.has_value(), "no next link");
    goToLink(nextLink.value());    
  }else{
    goBackMainMenu();
  }
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
  std::optional<std::string> player;
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
    .paths = { "mainmenu/", "mainmenu/levelselect/", "mainmenu/settings/", "debug/wheel/" },
    .scene = [](std::vector<std::string> params) -> std::string { return "../afterworld/scenes/menu.rawscene"; },
    .makePlayer = false,
    .player = std::nullopt,
  },
  SceneRouterPath {
    .paths = { "playing/*/",  "playing/*/paused/", "playing/*/dead/" },
    .scene = [](std::vector<std::string> params) -> std::string {
      auto sceneFile = levelByShortcutName(params.at(0));
      modassert(sceneFile.has_value(), std::string("no scene file for: ") + params.at(0));
      return sceneFile.value();
    },
    .makePlayer = true,
    .player = "maincamera",
  },
  SceneRouterPath {
    .paths = { "loading/" },
    .scene = [](std::vector<std::string> params) -> std::string { return "../afterworld/scenes/loading.rawscene"; },
    .makePlayer = false,
    .player = std::nullopt,
  },
  SceneRouterPath {
    .paths = { "mainmenu/modelviewer/" },
    .scene = [](std::vector<std::string> params) -> std::string { return "../afterworld/scenes/dev/models.rawscene"; },
    .makePlayer = false,
    .player = "maincamera",
  },
  SceneRouterPath {
    .paths = { "mainmenu/particleviewer/" },
    .scene = [](std::vector<std::string> params) -> std::string { return "../afterworld/scenes/dev/particles.rawscene"; },
    .makePlayer = false,
    .player = "maincamera",
  },
};

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
  return std::nullopt;
}


bool isGunZoomed(objid id){
  return getWeaponState(weapons, id).isGunZoomed;
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
  modlog("router scene route", std::string("path is: ") + currentPath);

  int currentIndex = 0;
  std::vector<std::string> params;
  auto router = getSceneRouter(currentPath, &currentIndex, &params);
  modlog("router scene route, router, has router = ", print(router.has_value()));
  //modassert(router.has_value(), std::string("no router for path: ") + currentPath);

  std::optional<std::string> sceneToLoad;


  int matchedRouterOption = 0;
  auto routerOptions = getRouterOptions(currentPath, queryParams, &matchedRouterOption);
  modassert(routerOptions.has_value(), std::string("no router options for: ") + currentPath);
  modlog("router", std::string("matched router option: ") + std::to_string(matchedRouterOption));
  setRouterGameState(RouteState{
    .paused = routerOptions.value() -> paused,
    .inGameMode = routerOptions.value() -> inGameMode,
    .showMouse = routerOptions.value() -> showMouse,
  });
  setPaused(routerOptions.value() -> paused);

  if (router.has_value() && router.value() -> scene.has_value()){
    sceneToLoad = router.value() -> scene.value()(params);
    modlog("router scene route load", sceneToLoad.value());
  }
  

  bool currentSceneFileLoaded = sceneToLoad.has_value() && sceneManagement.managedScene.has_value() && sceneManagement.managedScene.value().sceneFile.has_value() && sceneManagement.managedScene.value().sceneFile.value() == sceneToLoad.value();

  if (sceneManagement.managedScene.has_value()){
    if (router.has_value() && sceneManagement.managedScene.value().index == currentIndex && currentSceneFileLoaded){
      modlog("router scene route", "already loaded, returning");
      return;
    }else{
      modlog("router scene route unload", sceneManagement.managedScene.value().path);
      if (sceneManagement.managedScene.value().id.has_value()){
        auto sceneFileName = gameapi -> listSceneFiles(sceneManagement.managedScene.value().id.value()).at(0);
        auto sceneName = gameapi -> sceneNameById(sceneManagement.managedScene.value().id.value());
        modlog("router scene route unloading", std::to_string(sceneManagement.managedScene.value().id.value()) + std::string(" ") + print(sceneName) + std::string(" ") + sceneFileName);
        gameapi -> unloadScene(sceneManagement.managedScene.value().id.value());
      }
      sceneManagement.managedScene = std::nullopt;
    }
  }


  if (router.has_value()){
    std::optional<objid> sceneId;
    if (sceneToLoad.has_value()){
      sceneId = gameapi -> loadScene(sceneToLoad.value(), {}, std::nullopt, {});
    }
    sceneManagement.managedScene = ManagedScene {
      .id = sceneId,
      .index = currentIndex,
      .path = currentPath,
      .sceneFile = sceneToLoad,
    };
    modlog("router scene route load", sceneManagement.managedScene.value().path);
    if (sceneId.has_value()){
      if (router.value() -> makePlayer){
        auto playerLocationObj = gameapi -> getObjectsByAttr("playerspawn", std::nullopt, sceneId.value());
        modassert(playerLocationObj.size() > 0, "no initial spawnpoint");
        glm::vec3 position = gameapi -> getGameObjectPos(playerLocationObj.at(0), true);
        createPrefab(sceneId.value(), "../afterworld/scenes/prefabs/player.rawscene",  position);
      }
      if (router.value() -> player.has_value()){

        // this doesn't work...it's looking for the player both times and order is indeterminant so sometimes it will fail setting the active player
        auto playerId = findObjByShortName(router.value() -> player.value(), sceneId);
        modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
        modlog("router", std::string("setting active player: playerId id = ") + std::to_string(playerId.value()));

        setActivePlayer(movement, weapons, aiData, playerId.value());
      }
    }
  }
}


struct GameState {
  SceneManagement sceneManagement;
  MovementEntityData movementEntities;

  std::optional<std::string> dragSelect;
  std::optional<glm::vec2> selecting;

  UiData uiData;

  DebugPrintType printType;
};

GameState* gameStatePtr = NULL;

MovementEntityData& getMovementData(){
  return gameStatePtr -> movementEntities;
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

bool keyIsDown(int key){
  extern GLFWwindow* window;
  return glfwGetKey(window, key) == GLFW_PRESS;
}
glm::vec2 getMouseVelocity(){
  return getGlobalState().mouseVelocity;
}

bool isPose(std::string name){
  return name.find("pose-") == 0;
}
DebugConfig debugPrintAnimations(){
  DebugConfig debugConfig { .data = {} };
  std::vector<objid> ids;
  if (controlledPlayer.playerId.has_value()){
    ids.push_back(controlledPlayer.playerId.value());
  }
  if (ids.size() > 0){
    auto id = ids.at(0);
    auto name = gameapi -> getGameObjNameForId(id).value();
    debugConfig.data.push_back({"object", name });
    auto animationNames = gameapi -> listAnimations(id);
    for (auto &animation : animationNames){
      bool isNamePose = isPose(animation);
      if (isNamePose){
        debugConfig.data.push_back({ animation, DebugItem {
          .text = "[POSE]",
          .onClick = [id, animation]() -> void {
            gameapi -> setAnimationPose(id, animation, 0.f);
          },
        }});   
      }else{
        debugConfig.data.push_back({ animation, DebugItem {
          .text = "[PLAY]",
          .onClick = [id, animation]() -> void {
            gameapi -> playAnimation(id, animation, ONESHOT);
          },
        }});        
      }
    }
    if (animationNames.size() == 0){
      debugConfig.data.push_back({ "[no-animations]" });
    }
  }else{
    debugConfig.data.push_back({ "animations" });
  }
  return debugConfig;
}

void deliverCurrentGunAmmo(objid id, int ammoAmount){
  deliverAmmoToCurrentGun(getWeaponState(weapons, id), ammoAmount, id);
}

std::optional<glm::vec3> oldGravity;
void setNoClipMode(bool enable){
  if (controlledPlayer.playerId.has_value()){
    if (enable){
      oldGravity = getSingleVec3Attr(controlledPlayer.playerId.value(), "physics_gravity");
      setGameObjectGravity(controlledPlayer.playerId.value(), glm::vec3(0.f, 0.f, 0.f));
      gameapi -> setSingleGameObjectAttr(controlledPlayer.playerId.value(), "physics_collision", "nocollide");

    }else{
      if(oldGravity.has_value()){
        setGameObjectGravity(controlledPlayer.playerId.value(), oldGravity.value());
        gameapi -> setSingleGameObjectAttr(controlledPlayer.playerId.value(), "physics_collision", "collide");
        oldGravity = std::nullopt;
      }
    }
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
    bool debugUi = getArgEnabled("debug-ui");
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
   .showGameHud = []() -> bool { return getGlobalState().showGameHud && !isPlayerControlDisabled(); },
   .showTerminal = []() -> std::optional<TerminalConfig> {
      if (!terminalInterface.has_value()){
        return std::optional<TerminalConfig>(std::nullopt); 
      }
      return getGlobalState().showTerminal ? terminalInterface.value().terminalConfig : std::optional<TerminalConfig>(std::nullopt); 
   },
   .showZoomOverlay = []() -> std::optional<ZoomOptions> { 
      return zoomOptions; 
   },
   .showKeyboard = []() -> bool { 
      return getGlobalState().showKeyboard;
   },
   .debugConfig = [&gameState]() -> std::optional<DebugConfig> {
      if (gameState.printType == DEBUG_GLOBAL){
        return debugPrintGlobal();
      }
      if (gameState.printType == DEBUG_INVENTORY){
        debugPrintInventory();
        return std::nullopt;
      }
      if (gameState.printType == DEBUG_GAMETYPE){
        return debugPrintGametypes(gametypeSystem);
      }
      if (gameState.printType == DEBUG_AI){
        return debugPrintAi(aiData);
      }
      if (gameState.printType == DEBUG_HEALTH){
        return debugPrintHealth();
      }
      if (gameState.printType == DEBUG_ACTIVEPLAYER){
        return debugPrintActivePlayer();
      }
      if (gameState.printType == DEBUG_ANIMATION){
        return debugPrintAnimations();
      }
      return std::nullopt;
   },
   .getScoreConfig = []() -> std::optional<ScoreOptions> {
      auto gametypeData = getGametypeData(gametypeSystem);
      if (!gametypeData.has_value() || getGlobalState().showEditor){
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
        goToLevel(level.name);
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
      goToLevel(scene);
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
    .showPreviousModel = modelViewerPrevModel,
    .showNextModel = modelViewerNextModel,
    .playSound = []() -> void {
      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
    },
    .consoleInterface = ConsoleInterface {
      .setNormalMode = []() -> void {
        setActivePlayerEditorMode(false);
        setShowFreecam(false);
        setShowEditor(false);
      },
      .setShowEditor = []() -> void {
        setActivePlayerEditorMode(true);
        setShowFreecam(false);
        setShowEditor(true);
      },
      .setFreeCam = []() -> void {
        setActivePlayerEditorMode(true);
        setShowFreecam(true);
        setShowEditor(false);
      },
      .setNoClip = setNoClipMode,
      .setBackground = setMenuBackground,
      .goToLevel = [&gameState](std::optional<std::string> level) -> void {
        modlog("gotolevel", std::string("level loading: ") + level.value());
        setProgressByShortname(level.value());
        goToLevel(level.value());
      },
      .nextLevel = []() -> void {
        goToNextLevel();
      },
      .takeScreenshot = [](std::string path) -> void {
        gameapi -> saveScreenshot(path);
      },
      .routerPush = [](std::string route, bool replace) -> void {
        pushHistory({ route }, replace);
      },
      .routerPop = []() -> void {
        popHistory();
      },
      .die = killActivePlayer,
      .toggleKeyboard = []() -> void {
        toggleKeyboard();
      },
      .setShowDebugUi = [&gameState](DebugPrintType printType) -> void {
        gameState.printType = printType;
      },
      .showWeapon = [&gameState](bool showWeapon) -> void {
        modlog("console weapons", std::string("show weapon: ") + print(showWeapon));
        setShowWeaponModel(showWeapon);
      },
      .deliverAmmo = [](int amount) -> void {
        if(controlledPlayer.playerId.has_value()){
          deliverCurrentGunAmmo(controlledPlayer.playerId.value(), amount);
        }
      },
      .disableActiveEntity = [](bool enable) -> void {
        disableTpsMesh = enable;
      },
    },
  };
  return uiContext;
}

CutsceneApi cutsceneApi {
  .showLetterBox = showLetterBox,
  .setCameraPosition = [](std::optional<std::string> camera, glm::vec3 position, glm::quat rotation, std::optional<float> duration) -> void {
    modlog("cutscene api", "setCameraPosition");
    if (!camera.has_value()){
      setTempCamera(std::nullopt);
    }else{
      auto testViewObj = findObjByShortName(camera.value(), std::nullopt);
      setTempCamera(testViewObj.value());      
      gameapi -> moveCameraTo(testViewObj.value(), position, duration);
      gameapi -> setGameObjectRot(testViewObj.value(), rotation, true);
    }
  },
  .setPlayerControllable = [](bool isPlayable) -> void {
    modlog("cutscene api", "setPlayerControllable");
    setDisablePlayerControl(!isPlayable);
  },
  .goToNextLevel = goToNextLevel,
  .setWorldState = [](std::string field, std::string name, AttributeValue value) -> void {
    gameapi -> setWorldState({
      ObjectValue {
        .object = field,
        .attribute = name,
        .value = value,
      },
    });  
  }
};


bool hasAnimation(objid entityId, std::string& animationName){
  return gameapi -> listAnimations(entityId).count(animationName) > 0;
}


void doAnimationTrigger(objid entityId, const char* transition){
  if (!hasControllerState(tags.animationController, entityId)){
    return;
  }
  bool changedState = triggerControllerState(tags.animationController, entityId, getSymbol(transition));
  if (changedState){
    tags.animationController.pendingAnimations.insert(entityId);
  }
}
void doStateControllerAnimations(){
  for (auto entityId : tags.animationController.pendingAnimations){
    auto stateAnimation = stateAnimationForController(tags.animationController, entityId);
    bool stateAnimationHasAnimation = stateAnimation && stateAnimation -> animation.has_value();
    bool matchingAnimation = stateAnimationHasAnimation && hasAnimation(entityId, stateAnimation -> animation.value());
    if (!disableAnimation && matchingAnimation){
      modlog("animation controller play animation for state", nameForSymbol(stateAnimation -> state));
      gameapi -> playAnimation(entityId, stateAnimation -> animation.value(), stateAnimation -> animationBehavior);  
    }else{
      if (stateAnimationHasAnimation && !matchingAnimation){
        if (validateAnimationControllerAnimations){
          modassert(false, std::string("no matching animation: ") + stateAnimation -> animation.value());
        }
        modlog("animation controller play animation no matching animation for state", nameForSymbol(stateAnimation -> state) + ", for animation: " + stateAnimation -> animation.value());
      }
      gameapi -> stopAnimation(entityId);
    }
  }
  tags.animationController.pendingAnimations = {};
}

UiStateContext uiStateContext {
  .routerHistory = &getMainRouterHistory(),
  .uiState = createUiState(),
};


AIInterface aiInterface {
  .move = [](objid agentId, glm::vec3 targetPosition, float speed) -> void {
    setEntityTargetLocation(gameStatePtr -> movementEntities, agentId, MovementRequest {
      .position = targetPosition,
      .speed = speed * 0.6f,
    });
  },
  .fireGun = [](objid agentId) -> void {
    fireGun(weapons, agentId);
  },
};

float querySelectDistance(){
  auto traitQuery = gameapi -> compileSqlQuery("select select-distance from traits where profile = ?", { "default" });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(traitQuery, &validSql);
  modassert(validSql, "error executing sql query");
  float selectDistance = floatFromFirstSqlResult(result, 0);
  return selectDistance;
}

bool entityInShootingMode(objid id){
  return isInShootingMode(id).value();
}


CScriptBinding afterworldMainBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  
  if (getArgEnabled("help")){
    printGameOptionsHelp();
    exit(0);
  }

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    GameState* gameState = new GameState;
    gameStatePtr = gameState;

    weapons = createWeapons();

    gameState -> sceneManagement = createSceneManagement();
    gameState -> movementEntities = MovementEntityData {};
    gameState -> selecting = std::nullopt;
    gameState -> uiData = {
      .uiContext = {},
      .uiCallbacks = HandlerFns {
        .handlerFns = {},
        .handlerFns2 = {},
        .inputFns = {},
      }
    };
    initGlobal();
    reloadSettingsConfig(movement, "default");
    gameState -> dragSelect = std::nullopt;
    gameState -> uiData.uiContext = getUiContext(*gameState);

    auto args = gameapi -> getArgs();
    if (args.find("dragselect") != args.end()){
      gameState -> dragSelect = args.at("dragselect");
      modlog("bindings", std::string("drag select value: ") + gameState -> dragSelect.value());
    }

    if (getArgEnabled("debug-shoot")){
      setDrawDebugVector(true);
    }

    gameState -> printType = DEBUG_NONE;

    if (getArgChoice("print-type", "inventory")){
      gameState -> printType = DEBUG_INVENTORY;
    }else if (getArgChoice("print-type", "global")){
      gameState -> printType = DEBUG_GLOBAL;
    }else if (getArgChoice("print-type", "gametype")){
      gameState -> printType = DEBUG_GAMETYPE;
    }else if (getArgChoice("print-type", "ai")){
      gameState -> printType = DEBUG_AI;
    }else if (getArgChoice("print-type", "health")){
      gameState -> printType = DEBUG_HEALTH;
    }else if (getArgChoice("print-type", "active")){
      gameState -> printType = DEBUG_ACTIVEPLAYER;
    }else if (getArgChoice("print-type", "animation")){
      gameState -> printType = DEBUG_ANIMATION;
    }
    
    if (args.find("uiselection-texture") != args.end()){
      setShowSelectionTexture(true);
    }
    if (getArgEnabled("no-animation")){
      disableAnimation = true;
    }

    disableTpsMesh = getArgEnabled("no-mesh-tp");
    validateAnimationControllerAnimations = getArgEnabled("validate-animation");

    initSettings();
    registerOnRouteChanged(
      getMainRouterHistory(),
      [gameState]() -> void {
        auto currentPath = fullHistoryStr();
        auto queryParams = historyParams();
        onSceneRouteChange(gameState -> sceneManagement, currentPath, queryParams);
        modlog("routing", std::string("scene route registerOnRouteChanged: , new route: ") + currentPath);
      }
    );


    if (hasOption("level")){
      levelShortcutToLoad = getArgOption("level");;
    }else if (args.find("route") == args.end()){
      pushHistory({ "mainmenu" }, true);
    }else{
      pushHistory(split(args.at("route"), '/'), true);
    }

    loadDialogTree();

    soundData = createSoundData(gameapi -> rootSceneId());
    ensureDefaultSoundsLoadced(gameapi -> rootSceneId());

    gametypeSystem = createGametypes();
    aiData = createAiData();
    maybeSpawnLightFromArgs();

    loadAllMaterials(gameapi -> rootSceneId());
    loadParticleEmitters(gameapi -> rootSceneId());

    ensurePrecachedModels(
      gameapi -> rootSceneId(),
    { 
      "/home/brad/Desktop/testmodel/scene.gltf" /* br*/, 
      "../gameresources/build/weapons/scrapgun.gltf",
      "../gameresources/build/weapons/electrogun.gltf",
      "../gameresources/build/weapons/fork.gltf",
    });
  
    tags = createTags(&gameState -> uiData);

    handleOnAddedTagsInitial(tags); // not sure i actually need this since are there any objects added?
    generateWaterMesh();
    
    //addWaterObj(gameapi -> rootSceneId());
    
    return gameState;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    removeAllMovementCores();  // is this pointless?
    delete gameState;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    gameapi -> idAtCoordAsync(getGlobalState().xNdc, getGlobalState().yNdc, false, std::nullopt, [](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
      getGlobalState().selectedId = selectedId;
      getGlobalState().texCoordUv = texCoordUv;
      //modlog("texcoord", print(glm::vec2(texCoordUv)));
    });

    gameapi -> idAtCoordAsync(0.f, 0.f, false, std::nullopt, [](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
      getGlobalState().lookAtId = selectedId;
      getGlobalState().texCoordUvView = texCoordUv;
    });

    auto ndiCoord = uvToNdi(getGlobalState().texCoordUvView);

    if (gameState -> dragSelect.has_value() && gameState -> selecting.has_value()){
      //selectWithBorder(gameState -> selecting.value(), glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc));
    }

    tickCutscenes(cutsceneApi, gameapi -> timeSeconds(false));
    if (controlledPlayer.playerId.has_value() && !isPaused() && !isPlayerControlDisabled()){  
      auto uiUpdate = onWeaponsFrame(weapons, controlledPlayer.playerId.value(), controlledPlayer.lookVelocity, getPlayerVelocity(), getWeaponEntityData, 
        [](objid id) -> objid {
          return controlledPlayer.activePlayerManagedCameraId.value();  // kind of wrong, but i think, kind of right in practice
        }, 
        ThirdPersonWeapon {
          .getWeaponParentId = [](objid id) -> std::optional<objid> {
            auto children = gameapi -> getChildrenIdsAndParent(id);
            for (auto childId : children){
              auto name = gameapi -> getGameObjNameForId(childId).value();
              if (stringEndsWith(name, "mixamorig:RightHand")){
                return childId;
              }
            }
            return std::nullopt;
          },
        }
      );
      setShowActivate(uiUpdate.showActivateUi);
      if (uiUpdate.ammoInfo.has_value()){
        setUIAmmoCount(uiUpdate.ammoInfo.value().currentAmmo, uiUpdate.ammoInfo.value().totalAmmo);
      }else{
        setUIAmmoCount(0, 0);
      }
      if (uiUpdate.bloomAmount.has_value()){
        drawBloom(controlledPlayer.playerId.value(), controlledPlayer.playerId.value(), -1.f, uiUpdate.bloomAmount.value()); // 0.002f is just a min amount for visualization, not actual bloom
      }
      modlog("current weapon", uiUpdate.currentGunName.has_value() ? *uiUpdate.currentGunName.value() : "");
      setUiWeapon(uiUpdate.currentGunName.has_value() ? *uiUpdate.currentGunName.value() : std::optional<std::string>(std::nullopt));
    }else{
      setShowActivate(false);
      setUIAmmoCount(0, 0);
      setUiWeapon(std::nullopt);
    }

    setUiGemCount(listGems().size());

    if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled()){
      const bool showLookVelocity = false;
      auto thirdPersonCamera = getCameraForThirdPerson();
      if (!thirdPersonCamera.has_value()){
        if (!gameState -> movementEntities.movementEntities.at(controlledPlayer.playerId.value()).managedCamera.thirdPersonMode){
          thirdPersonCamera = 0;
        }else{
          modassert(false, "cannot use third person mode, no camera provided");
        }
      }
 
      auto uiUpdate = onMovementFrame(gameState -> movementEntities, movement, controlledPlayer.playerId.value(), isGunZoomed, thirdPersonCamera.value(), disableTpsMesh);
      setUiSpeed(uiUpdate.velocity, showLookVelocity ? uiUpdate.lookVelocity : std::nullopt);

      drawAllCurves(id);
      handleEntitiesOnRails(id, gameapi -> rootSceneId());
      //handleEntitiesRace();
    }

    auto playerPosition = getActivePlayerPosition();
    if (playerPosition.has_value()){
      drawWaypoints(waypoints, playerPosition.value());
    }
    //std::optional<glm::vec2> mainUiCursorCoord = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
    std::optional<glm::vec2> mainUiCursorCoord;

    auto start = std::chrono::steady_clock::now();
   
    gameState -> uiData.uiCallbacks = handleDrawMainUi(uiStateContext, tags.uiData -> uiContext, getGlobalState().selectedId, std::nullopt, mainUiCursorCoord);
    modassert(tags.uiData, "tags.uiData NULL");
    
    auto end = std::chrono::steady_clock::now();
    std::chrono::microseconds timeUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    //modlog("main ui time us: ", std::to_string(timeUs.count())); 

    onInGameUiFrame(uiStateContext, tags.inGameUi, tags.uiData->uiContext, std::nullopt, ndiCoord);
    
    std::optional<UiHealth> uiHealth;
    auto activePlayer = controlledPlayer.playerId;
    if (activePlayer.has_value()){
      auto health = getHealth(activePlayer.value());
      if (health.has_value()){
        uiHealth = UiHealth {
          .health = health.value().current,
          .totalHealth = health.value().total,
        };
      }
    }
    setUiHealth(uiHealth);
    
    onFrameWater(water);
    onFrameAi(aiData);
    onFrameDaynight();
    onTagsFrame(tags);
    doStateControllerAnimations();

    if (levelShortcutToLoad.has_value()){
      setProgressByShortname(levelShortcutToLoad.value());
      goToLevel(levelShortcutToLoad.value());
      levelShortcutToLoad = std::nullopt;
    }

    // debug
    debugOnFrame();
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    auto hasInputKey = gameapi -> unlock("input", id);
    if (!hasInputKey){
      return;
    }
    if (action == 1){
      auto playerId = controlledPlayer.playerId;
      if (isPauseKey(key)){
        togglePauseIfInGame();
      }
      if (isTeleportButton(key) && !isPaused()){
        // this probably should be aware of the bounds, an not allow to clip into wall for example
        // maybe raycast down, and then set the position so it fits 
        auto teleportPosition = getTeleportPosition(tags);
        if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled() && teleportPosition.has_value()){
          playGameplayClipById(getManagedSounds().soundObjId.value(), std::nullopt, std::nullopt);
          gameapi -> setGameObjectPosition(controlledPlayer.playerId.value(), teleportPosition.value().position, true);
          gameapi -> removeByGroupId(teleportPosition.value().id);
        }
      }
      if (isExitTerminalKey(key)){
        showTerminal(std::nullopt);
      }

      onMainUiKeyPress(uiStateContext, gameState -> uiData.uiCallbacks, key, scancode, action, mods);
      onInGameUiKeyCallback(key, scancode, action, mods);
    }
    handleHotkey(key, action);


    if (key == '-' && action == 0){
      setActivePlayerNext(movement, weapons, aiData);
    }

    if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled()){
      if (!(isPaused() || getGlobalState().disableGameInput)){
        onWeaponsKeyCallback(getWeaponState(weapons, controlledPlayer.playerId.value()), key, action, controlledPlayer.playerId.value());
      }
    }
    if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled()){
      onMovementKeyCallback(gameState -> movementEntities, movement, controlledPlayer.playerId.value(), key, action);
    }

    if (key == 'Q' && action == 0) { 
      printWorldInfo(aiData.worldInfo);
      if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled()){
        setInShootingMode(controlledPlayer.playerId.value(), !isInShootingMode(controlledPlayer.playerId.value()).value());
      }
    }

    debugOnKey(key, scancode, action, mods);
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    GameState* gameState = static_cast<GameState*>(data);

    if (key == "save-gun"){
      saveGunTransform(getWeaponState(weapons, controlledPlayer.playerId.value()).weaponValues);
    }

    if (key == "reset"){
      pushHistory({ "mainmenu" }, true);
      return;
    }
    if (key == "reload-config:levels"){
      gameState -> sceneManagement.levels = loadLevels();
      return;
    }

    if (key == "selected"){
      auto gameObjId = anycast<objid>(value); 
      modassert(gameObjId, "selected value invalid");
      if (!gameapi -> getGameObjNameForId(*gameObjId).has_value()){
        return;
      }
      handleInteract(*gameObjId);
      return;
    }
    if (key == "switch"){
      auto strValue = anycast<std::string>(value); 
      modassert(strValue != NULL, "switch value invalid");
      handleSwitch(tags.switches, *strValue);
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

    if (key == "link"){
      auto attrValue = anycast<MessageWithId>(value); 
      modassert(attrValue, "link message invalid");
      auto linkToValue = getSingleAttr(attrValue -> id, "link");
      modassert(linkToValue.has_value(), "link id does not have a link tag");
      if (linkToValue.value() == "next"){ // hackey but that's ok!
        goToNextLevel();
      }else{
        goToLink(linkToValue.value());
      }
    }

    if (key == "ammo"){
      auto itemAcquiredMessage = anycast<ItemAcquiredMessage>(value);
      modassert(itemAcquiredMessage != NULL, "ammo message not an ItemAcquiredMessage");
      if (controlledPlayer.playerId.has_value() && (controlledPlayer.playerId.value() == itemAcquiredMessage -> targetId)){
        deliverCurrentGunAmmo(controlledPlayer.playerId.value(), itemAcquiredMessage -> amount);
      }
    }
    if (key == "gem-pickup"){
      auto itemAcquiredMessage = anycast<ItemAcquiredMessage>(value);
      modassert(itemAcquiredMessage != NULL, "gem-pickup message not an ItemAcquiredMessage");
      auto position = gameapi -> getGameObjectPos(itemAcquiredMessage -> targetId, true);
      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, position);
      pickupGem("testgem");
    }
    if (key == "activate-switch"){
      auto attrValue = anycast<MessageWithId>(value); 
      modassert(attrValue, "activate-switch message invalid");
      auto switchValue = getSingleAttr(attrValue -> id, "activate-switch-type");
      modassert(switchValue.has_value(), "activate-switch does not have a activate-switch-type");
      handleSwitch(tags.switches, switchValue.value());
    }

    if (key == "play-material-sound"){
      auto soundPosition = anycast<MessagePlaySound>(value);
      modassert(soundPosition != NULL, "sound position not given");
      playMaterialSound(soundData, gameapi -> rootSceneId(), soundPosition -> position, soundPosition -> material);
    }

    onCutsceneMessages(key);
    gametypesOnMessage(gametypeSystem, key, value);
    onAiOnMessage(aiData, key, value);

    if (key == "weather"){
      onWeatherMessage(weather, value, gameapi -> rootSceneId());
    }

    onTagsMessage(tags, key, value);
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
    GameState* gameState = static_cast<GameState*>(data);

    getGlobalState().xNdc = xNdc;
    getGlobalState().yNdc = yNdc;
    getGlobalState().mouseVelocity = glm::vec2(xPos, yPos);
    if (controlledPlayer.playerId.has_value() && !isPaused() && !getGlobalState().disableGameInput && !isPlayerControlDisabled()){
      controlledPlayer.lookVelocity = glm::vec2(xPos, yPos);
    }
    if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled()){
      onMovementMouseMoveCallback(gameState -> movementEntities, movement, controlledPlayer.playerId.value(), xPos, yPos);
    }

    onMainUiMouseMove(uiStateContext,  gameState -> uiData.uiContext, xPos, yPos, xNdc, yNdc);
    onInGameUiMouseMoveCallback(tags.inGameUi, xPos, yPos, xNdc, yNdc);
  };

  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    onMainUiMousePress(uiStateContext, gameState -> uiData.uiContext, tags.uiData -> uiCallbacks, button, action, getGlobalState().selectedId);
    onInGameUiMouseCallback(uiStateContext, tags.uiData -> uiContext, tags.inGameUi, button, action, getGlobalState().lookAtId /* this needs to come from the texture */);

    modlog("input", std::string("on mouse down: button = ") + std::to_string(button) + std::string(", action = ") + std::to_string(action));
    if (button == 1){
      if (action == 0){
        gameState -> selecting = std::nullopt;
        getGlobalState().rightMouseDown = false;
      }else if (action == 1){
        gameState -> selecting = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
        getGlobalState().rightMouseDown = true;
        if (false){
          raycastFromCameraAndMoveTo(gameState -> movementEntities, controlledPlayer.playerId.value());
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
    if (controlledPlayer.playerId.has_value()){
      static float selectDistance = querySelectDistance();
      if (!isPaused() && !getGlobalState().disableGameInput && !isPlayerControlDisabled()){
        auto uiUpdate = onWeaponsMouseCallback(getWeaponState(weapons, controlledPlayer.playerId.value()), button, action, controlledPlayer.playerId.value(), selectDistance);
        if (uiUpdate.zoomAmount.has_value()){
          setTotalZoom(uiUpdate.zoomAmount.value());
        }
      }
    }
  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    onMainUiScroll(uiStateContext, tags.uiData->uiContext, amount);
    onInGameUiScrollCallback(tags.inGameUi, amount);
    onMovementScrollCallback(movement, amount);
  };
  binding.onObjectAdded = [](int32_t _, void* data, int32_t idAdded) -> void {
    GameState* gameState = static_cast<GameState*>(data);

    onAddControllableEntity(aiData, gameStatePtr -> movementEntities, idAdded);
    handleOnAddedTags(tags, idAdded);

    onMainUiObjectsChanged();
  };
  binding.onObjectRemoved = [](int32_t _, void* data, int32_t idRemoved) -> void {
    GameState* gameState = static_cast<GameState*>(data);

    modlog("onObjectRemoved", gameapi -> getGameObjNameForId(idRemoved).value());

    if (controlledPlayer.playerId.has_value() && controlledPlayer.playerId.value() == idRemoved){
      controlledPlayer.playerId = std::nullopt;
    }
    maybeRemoveControllableEntity(aiData, gameStatePtr -> movementEntities, idRemoved);

    auto playerKilled = onActivePlayerRemoved(idRemoved);
    if (playerKilled){
      displayGameOverMenu();
    }
    onMainUiObjectsChanged();
    if (controlledPlayer.playerId.has_value()){
      if (controlledPlayer.playerId.value() == idRemoved){
        modlog("weapons", "remove player");
        controlledPlayer.playerId = std::nullopt;
      }
    }

    onObjectRemovedWater(water, idRemoved);
    handleTagsOnObjectRemoved(tags, idRemoved);
  };

  return binding;
}

std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api){
  std::vector<CScriptBinding> bindings;
  gameapi = &api;
  bindings.push_back(afterworldMainBinding(api, "native/main"));
  bindings.push_back(modelviewerBinding(api, "native/modelviewer"));
  bindings.push_back(particleviewerBinding(api, "native/particleviewer"));
  return bindings;
} 

