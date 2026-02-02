#include "./bindings.h"

CustomApiBindings* gameapi = NULL;


Weapons weapons{};
Movement movement = createMovement();
Director director = createDirector();
Vehicles vehicles = createVehicles();

std::unordered_map<objid, ArcadeInstance> arcadeInstances; 
std::unordered_map<objid, HitPoints> hitpoints = {}; 
std::unordered_map<objid, ControllableEntity> controllableEntities;
std::unordered_map<objid, Inventory> scopenameToInventory;
std::vector<CrystalPickup> crystals; 
std::vector<LevelProgress> levelProgresses;
std::unordered_map<objid, glm::vec3> impulses;
OrbData orbData;

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
std::string defaultAudioClipPath;

struct LifeTimeObject {
  std::function<void()> fn;
  std::string hint;
};
std::unordered_map<objid, LifeTimeObject> lifetimeObjects;

struct GameModeNone{};
struct GameModeFps {
  bool makePlayer = false;
  std::optional<std::string> player;
};
struct GameModeBall{};
struct GameModeOrb {};
struct GameModeIntro{};

typedef std::variant<GameModeNone, GameModeFps, GameModeBall, GameModeOrb, GameModeIntro> GameMode;

struct ManagedScene {
  std::optional<objid> id; 
  int index;
  std::string path;
  std::optional<std::string> sceneFile;
  GameMode gameMode = GameModeNone{};
};
struct SceneManagement {
  std::vector<Level> levels;
  std::optional<ManagedScene> managedScene;
  int changedLevelFrame;
};

struct ScenarioOptions {
  glm::vec3 ambientLight;
  glm::vec3 skyboxColor;
  std::string skybox;
  std::string audioClipPath;
};

struct GameState {
  SceneManagement sceneManagement;
  MovementEntityData movementEntities;

  std::optional<std::string> dragSelect;
  std::optional<glm::vec2> selecting;

  UiData uiData;

  DebugPrintType printType;
};

GameState* gameStatePtr = NULL;
std::optional<std::string> activeLevel;

MovementEntityData& getMovementData(){
  return gameStatePtr -> movementEntities;
}

void setLifetimeObject(objid id, std::function<void()> fn, std::string hint){
  std::cout << "lifetimeObject add: " << gameapi -> getGameObjNameForId(id).value() << ", hint = " << hint << std::endl;
  //modassert(lifetimeObjects.find(id) == lifetimeObjects.end(), std::string("already lifetime object: ") + lifetimeObjects.at(id).hint);
  lifetimeObjects[id] = LifeTimeObject {
    .fn = fn,
    .hint = hint,
  };
}

void onMenu2NewGameClick(){
  ballModeNewGame();
}
void onMenu2ContinueClick(){
  ballModeLevelSelect();
}

std::vector<int> getVehicleIds(){
  std::vector<int>  vehicleIds;
  for (auto& [id, vehicle] : vehicles.vehicles){
    vehicleIds.push_back(id);
  }
  return vehicleIds;
}

void enterVehicleRaw(int playerIndex, objid vehicleId, objid id){
  enterVehicle(vehicles, vehicleId, id);
  std::optional<ControllableEntity*> controllable = getActiveControllable(playerIndex);
  modassert(controllable.has_value() && controllable.value() != NULL, "controllable invalid");
  controllable.value() -> vehicle = vehicleId;
}


bool canExitVehicle = true;
void setCanExitVehicle(bool canExit){
  canExitVehicle = canExit;
}
void exitVehicleRaw(int playerIndex, objid vehicleId, objid id){
  if (!canExitVehicle){
    return;
  }
  exitVehicle(vehicles, getActiveControllable(playerIndex).value() -> vehicle.value(), id);
  getActiveControllable(playerIndex).value() -> vehicle = std::nullopt;
}

void applyScreenshake(int playerIndex, glm::vec3 impulse){
  if (hasOption("no-shake")){
    return;
  }
  getControlledPlayer(playerIndex).shakeImpulse = impulse;
}

void setScenarioOptions(ScenarioOptions& options){
  gameapi -> setWorldState({ 
    ObjectValue {
      .object = "light",
      .attribute = "amount",
      .value = options.ambientLight,
    },
    ObjectValue {
      .object = "skybox",
      .attribute = "color",
      .value = options.skyboxColor,
    },
    ObjectValue {
      .object = "skybox",
      .attribute = "texture",
      .value = options.skybox,
    },
  });
  defaultAudioClipPath = options.audioClipPath;
  modlog("set scenario options: ambient", print(options.ambientLight));
  modlog("set scenario options: skyboxColor", print(options.skyboxColor));
  modlog("set scenario options: skybox", print(options.skybox));

}

objid createPrefab(objid sceneId, const char* prefab, glm::vec3 pos, std::unordered_map<std::string, AttributeValue> additionalFields){
  GameobjAttributes attr = {
    .attr = {
      { "scene", prefab },
      { "position", pos },
    },
  };

  for (auto &[key, payload] : additionalFields){
    attr.attr[key] = payload;
  }

  std::unordered_map<std::string, GameobjAttributes> submodelAttributes = {};
  return gameapi -> makeObjectAttr(
    sceneId, 
    std::string("[prefab-instance-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  ).value();
}

void startLevel(ManagedScene& managedScene){
  if (!managedScene.id.has_value()){
    return;
  }

  std::optional<objid> sceneId = managedScene.id;

  std::vector<objid> playerIds;

  auto gamemodeFps = std::get_if<GameModeFps>(&managedScene.gameMode);
  auto gamemodeBall = std::get_if<GameModeBall>(&managedScene.gameMode);
  auto gamemodeOrb = std::get_if<GameModeOrb>(&managedScene.gameMode);
  auto gamemodeIntro = std::get_if<GameModeIntro>(&managedScene.gameMode);
  if (gamemodeFps){
    if (gamemodeFps -> makePlayer){
      auto playerLocationObj = gameapi -> getObjectsByAttr("playerspawn", std::nullopt, std::nullopt).at(0);
      glm::vec3 position = gameapi -> getGameObjectPos(playerLocationObj, true, "[gamelogic] startLevel get player spawnpoint");
      int numberOfPlayers = getNumberOfPlayers();
      for (int i  = 0; i < numberOfPlayers; i++){
        auto initialPosition = position + glm::vec3(1.f * i, 0.f, 0.f); // TODO - this is bad, this should just have a few initial spawnpoints
        auto playerId = createPrefab(sceneId.value(), "../afterworld/scenes/prefabs/enemy/player.rawscene",  initialPosition, {});    
        playerIds.push_back(playerId);
      }
    }

    spawnFromAllSpawnpoints(director.managedSpawnpoints, "onload");

    if (playerIds.size() == 0 && gamemodeFps -> player.has_value()){
      // this doesn't work...it's looking for the player both times and order is indeterminant so sometimes it will fail setting the active player
      auto playerId = findObjByShortName(gamemodeFps -> player.value(), sceneId);
      modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
      modlog("router", std::string("setting active player: playerId id = ") + std::to_string(playerId.value()));
      setActivePlayer(movement, weapons, aiData, playerId.value(), 0);
    }

    if (playerIds.size() > 0){
      for (int i = 0; i < playerIds.size(); i++){
        auto prefabId = playerIds.at(i);
        auto playerId = findBodyPart(prefabId, gamemodeFps -> player.value().c_str());
        modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
        modlog("router", std::string("setting active player: playerId id = ") + std::to_string(playerId.value()));
        setActivePlayer(movement, weapons, aiData, playerId, i);
      }
    }
  }else if (gamemodeBall){
    //modassert(false, "gamemode ball not implemented");
    auto playerLocationObj = gameapi -> getObjectsByAttr("playerspawn", std::nullopt, std::nullopt).at(0);
    glm::vec3 position = gameapi -> getGameObjectPos(playerLocationObj, true, "[gamelogic] startLevel get player spawnpoint");
    
    // TODO - no reason to actually create the prefab here
    auto prefabId = createPrefab(sceneId.value(), "../afterworld/scenes/prefabs/enemy/player-cheap.rawscene",  position, {});    

    auto playerId = findObjByShortName("maincamera", sceneId);
    modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
    setActivePlayer(movement, weapons, aiData, playerId.value(), 0);

    startBallMode(sceneId.value());

  }else if (gamemodeOrb){
    auto cameraId = findObjByShortName(">camera-view", sceneId);
    setTempCamera(cameraId.value(), 0);
    setHudEnabled(false);
  }else if (gamemodeIntro){
    startIntroMode(sceneId.value());
  }
}
void endLevel(ManagedScene& managedScene){
  auto gamemodeBall = std::get_if<GameModeBall>(&managedScene.gameMode);
  if (gamemodeBall){
    endBallMode();
  }

  auto gamemodeOrb = std::get_if<GameModeOrb>(&managedScene.gameMode);
  if (gamemodeOrb){
    setHudEnabled(true);
  }

  auto gamemodeIntro = std::get_if<GameModeIntro>(&managedScene.gameMode);
  if (gamemodeIntro){
    endIntroMode();
  }
}

std::optional<ZoomOptions> zoomOptions;
void setTotalZoom(float multiplier, objid id){
  bool isZoomedIn = multiplier < 1.f;
  if (isZoomedIn){
    zoomOptions = ZoomOptions {
      .zoomAmount = static_cast<int>((1.f / multiplier)),
    };
  }else{
    zoomOptions = std::nullopt;
  }
  setZoom(multiplier, isZoomedIn);
  setZoomSensitivity(getMovementData(), multiplier, id);
  playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
}

void applyImpulseAffectMovement(objid id, glm::vec3 force){
  gameapi -> applyImpulse(id, force);
  impulses[id] = force;
}
std::optional<glm::vec3> getImpulseThisFrame(objid id){
  if (impulses.find(id) == impulses.end()){
    return std::nullopt;
  }
  return impulses.at(id);
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
    .changedLevelFrame = 0,
  };
}

void goToLevel(std::string levelShortName){
  pushHistory({ "playing", levelShortName }, true);
  activeLevel = levelShortName;
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
      if (modeStr == "orb"){
        return GameModeOrb{};
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

void maybeDisplayGameOver(){
  if (allPlayersDead()){
    auto playingPath = getPathParts(0);
    bool isPlaying = playingPath.has_value() && playingPath.value() == "playing";
    if (isPlaying){
      pushHistory({ "dead" });
    }
  }
}

struct SceneLoadInfo {
  std::string sceneFile;
  std::vector<std::vector<std::string>> additionalTokens;
};
struct SceneRouterPath {
  std::vector<std::string> paths;
  std::optional<std::function<SceneLoadInfo(std::vector<std::string> params)>> scene;
  std::optional<std::function<ScenarioOptions(std::vector<std::string> params)>> scenarioOptions;
  std::function<GameMode(std::vector<std::string> params)> getGameMode = [](std::vector<std::string> params) -> GameMode { return GameModeNone{}; };
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


void raiseTurret(objid id, bool raiseUp){
  Agent& agent = getAgent(aiData, id);
  if (isAgentTurret(agent)){
    auto isGunRaised = isGunRaisedTurret(agent);
    setGunTurret(agent, !isGunRaised);
  }
}

void wakeUpTv(objid id, bool active){
  Agent& agent = getAgent(aiData, id);
  if (isAgentTv(agent)){
    setTvActive(agent, active);
  }
}

void deliverPowerup(objid vehicle, objid powerupId){
  auto& powerup = tags.powerups.at(powerupId);
  if (powerup.lastRemoveTime.has_value()){
    return;
  }

  if (powerup.type == "jump"){
    setPowerupBall(vehicles, vehicle, BIG_JUMP);
  }else if (powerup.type == "dash"){
    setPowerupBall(vehicles, vehicle, LAUNCH_FORWARD);
  }else if (powerup.type == "low_gravity"){
    setPowerupBall(vehicles, vehicle, LOW_GRAVITY);
  }else if (powerup.type == "teleport"){
    setPowerupBall(vehicles, vehicle, TELEPORT);
  }else{
    modassert(false, std::string("invalid powerup type: ") + powerup.type);
    setPowerupBall(vehicles, vehicle, std::nullopt);
  }

  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt);
  
  if(!powerup.respawnRateMs.has_value()){
    gameapi -> removeObjectById(powerupId);
  }else{
    powerup.lastRemoveTime = gameapi -> timeSeconds(false);
  }
}


void setGlobalModeValues(bool isEditorMode){
  showSpawnpoints(director.managedSpawnpoints, isEditorMode);
  showTriggerVolumes(isEditorMode);
}

void onSceneRouteChange(SceneManagement& sceneManagement, std::string& currentPath, std::vector<std::string>& queryParams){
  modlog("router scene route", std::string("path is: ") + currentPath);

  int currentIndex = 0;
  std::vector<std::string> params;
  auto router = getSceneRouter(currentPath, &currentIndex, &params);
  modlog("router scene route, router, has router = ", print(router.has_value()));
  //modassert(router.has_value(), std::string("no router for path: ") + currentPath);

  std::optional<SceneLoadInfo> sceneToLoad;
  std::optional<ScenarioOptions> scenarioOptions;


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
    modlog("router scene route load", sceneToLoad.value().sceneFile);
  }
  if (router.has_value() && router.value() -> scenarioOptions.has_value()){
    scenarioOptions = router.value() -> scenarioOptions.value()(params);
  }
  

  // This is kind of weird now since can load things with additional tokens.  Maybe should be aware of the level name or something? 
  bool currentSceneFileLoaded = sceneToLoad.has_value() && sceneManagement.managedScene.has_value() && sceneManagement.managedScene.value().sceneFile.has_value() && sceneManagement.managedScene.value().sceneFile.value() == sceneToLoad.value().sceneFile;

  if (sceneManagement.managedScene.has_value()){
    if (router.has_value() && sceneManagement.managedScene.value().index == currentIndex && currentSceneFileLoaded){
      modlog("router scene route", "already loaded, returning");
      return;
    }else{
      modlog("router scene route unload", sceneManagement.managedScene.value().path);
      if (sceneManagement.managedScene.value().id.has_value()){
        endLevel(sceneManagement.managedScene.value());
        auto sceneFileName = gameapi -> listSceneFiles(sceneManagement.managedScene.value().id.value()).at(0);
        auto sceneName = gameapi -> sceneNameById(sceneManagement.managedScene.value().id.value());
        modlog("router scene route unloading", std::to_string(sceneManagement.managedScene.value().id.value()) + std::string(" ") + print(sceneName) + std::string(" ") + sceneFileName);
        gameapi -> unloadScene(sceneManagement.managedScene.value().id.value());
      }
      sceneManagement.managedScene = std::nullopt;
      activeLevel = std::nullopt;
      sceneManagement.changedLevelFrame = gameapi -> currentFrame();
    }
  }


  if (router.has_value()){
    std::optional<objid> sceneId;
    if (sceneToLoad.has_value()){
      setTempCamera(std::nullopt, 0); // find a better place for this, should be reconciled better
      setDisablePlayerControl(false, 0);

      if (sceneToLoad.value().additionalTokens.size() > 0){
        std::cout << "additional tokens: " << print(sceneToLoad.value().additionalTokens.at(0)) << std::endl;
      }else{
        std::cout << "additional tokens: " << "none" << std::endl;
    }

      sceneId = gameapi -> loadScene(sceneToLoad.value().sceneFile, sceneToLoad.value().additionalTokens, std::nullopt, {});
    }
    if (scenarioOptions.has_value()){
      setScenarioOptions(scenarioOptions.value());
    }else{
      ScenarioOptions defaultSettings {
        .ambientLight = glm::vec3(0.4f, 0.4f, 0.4f),
        .skyboxColor = glm::vec3(1.f, 1.f, 1.f),
        .skybox = "./res/textures/skyboxs/desert/",
      };
      setScenarioOptions(defaultSettings);
    }
    sceneManagement.managedScene = ManagedScene {
      .id = sceneId,
      .index = currentIndex,
      .path = currentPath,
      .sceneFile = sceneToLoad.value().sceneFile,
      .gameMode = router.value() -> getGameMode(params),
    };
    activeLevel = std::nullopt;
    sceneManagement.changedLevelFrame = gameapi -> currentFrame();
    modlog("router scene route load", sceneManagement.managedScene.value().path);
    startLevel(sceneManagement.managedScene.value());
   
  }
}

/*

struct AxisInfo {
  float leftTrigger;
  float rightTrigger;
  float leftStickX;
  float leftStickY;
  float rightStickX;
  float rightStickY;
};

enum BUTTON_TYPE { BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y, BUTTON_LEFT_STICK, BUTTON_RIGHT_STICK, BUTTON_START, BUTTON_LB, BUTTON_RB, BUTTON_HOME, BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT };
std::string print(BUTTON_TYPE button);

struct ButtonInfo {
  bool a = false;
  bool b = false;
  bool x = false;
  bool y = false;
  bool leftStick = false;
  bool rightStick = false;
  bool start = false;
  bool leftBumper = false;
  bool rightBumper = false;
  bool home = false;
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
};
struct ControlInfo {
  AxisInfo axisInfo;
  ButtonInfo buttonInfo;
};*/
std::string print(ControlInfo& controlInfo){
  std::string content;
  content += std::string("controller| a: ") + (controlInfo.buttonInfo.a ? "true" : "false") + "\n";
  content += std::string("controller| b: ") + (controlInfo.buttonInfo.b ? "true" : "false") + "\n";
  content += std::string("controller| x: ") + (controlInfo.buttonInfo.x ? "true" : "false") + "\n";
  content += std::string("controller| y: ") + (controlInfo.buttonInfo.y ? "true" : "false") + "\n";

  content += std::string("controller| leftStick: ") + (controlInfo.buttonInfo.leftStick ? "true" : "false") + "\n";
  content += std::string("controller| rightStick: ") + (controlInfo.buttonInfo.rightStick ? "true" : "false") + "\n";

  content += std::string("controller| start: ") + (controlInfo.buttonInfo.start ? "true" : "false") + "\n";

  content += std::string("controller| leftBumper: ") + (controlInfo.buttonInfo.leftBumper ? "true" : "false") + "\n";
  content += std::string("controller| rightBumper: ") + (controlInfo.buttonInfo.rightBumper ? "true" : "false") + "\n";

  content += std::string("controller| home: ") + (controlInfo.buttonInfo.home ? "true" : "false") + "\n";

  content += std::string("controller| up: ") + (controlInfo.buttonInfo.up ? "true" : "false") + "\n";
  content += std::string("controller| down: ") + (controlInfo.buttonInfo.down ? "true" : "false") + "\n";
  content += std::string("controller| left: ") + (controlInfo.buttonInfo.left ? "true" : "false") + "\n";
  content += std::string("controller| right: ") + (controlInfo.buttonInfo.right ? "true" : "false") + "\n";

  content += std::string("controller| leftTrigger: ") + std::to_string(controlInfo.axisInfo.leftTrigger) + "\n";
  content += std::string("controller| rightTrigger: ") + std::to_string(controlInfo.axisInfo.rightTrigger) + "\n";
  content += std::string("controller| leftStickX: ") + std::to_string(controlInfo.axisInfo.leftStickX) + "\n";
  content += std::string("controller| leftStickY: ") + std::to_string(controlInfo.axisInfo.leftStickY) + "\n";
  content += std::string("controller| rightStickX: ") + std::to_string(controlInfo.axisInfo.rightStickX) + "\n";
  content += std::string("controller| rightStickY: ") + std::to_string(controlInfo.axisInfo.rightStickY) + "\n";

  return content;
}


glm::vec2 smoothVelocity(glm::vec2 lookVelocity);

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

DebugConfig debugPrintAnimations(int playerIndex){
  DebugConfig debugConfig { .data = {} };
  std::vector<objid> ids;
  ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
  if (controlledPlayer.playerId.has_value()){
    ids.push_back(controlledPlayer.playerId.value());
  }
  if (ids.size() > 0){
    auto id = ids.at(0);
    auto name = gameapi -> getGameObjNameForId(id).value();
    debugConfig.data.push_back({"object", name });
    auto animationNames = gameapi -> listAnimations(id);
    for (auto &animation : animationNames){
      bool isNamePose = animation.find("pose-") == 0;
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
            gameapi -> playAnimation(id, animation, ONESHOT, std::nullopt, 0, false, std::nullopt);
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
  ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());

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

std::optional<objid> activeSceneForSelected(){
  if(gameStatePtr -> sceneManagement.managedScene.has_value() && gameStatePtr -> sceneManagement.managedScene.value().id.has_value()){
    return gameStatePtr -> sceneManagement.managedScene.value().id.value();
  }

  auto selected = gameapi -> selected();
  if (selected.size() == 0){
    return std::nullopt;
  }
  auto selectedId = gameapi -> selected().at(0);
  auto sceneId = gameapi -> listSceneId(selectedId);
  return sceneId;
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
    return getBoolWorldState("editor", "debug").value(); 
   },
   .showEditor = []() -> bool {
      return getGlobalState().showEditor;
   },
   .showConsole = showConsole,
   .showScreenspaceGrid = []() -> bool { return getGlobalState().showScreenspaceGrid; },
   .showGameHud = []() -> bool { return getGlobalState().showGameHud && !isPlayerControlDisabled(getDefaultPlayerIndex()); },
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
        debugPrintInventory(scopenameToInventory);
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
        return debugPrintActivePlayer(getDefaultPlayerIndex());
      }
      if (gameState.printType == DEBUG_ANIMATION){
        return debugPrintAnimations(getDefaultPlayerIndex());
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
   .getBallMode = showBallOptions,
   .getMenuOptions = []() -> std::optional<MainMenu2Options> {
      float duration = 0.2f;

      static bool showMenu = false;
      bool wasShowingMenu = showMenu;
      showMenu = getGlobalState().showLiveMenu;
      static std::optional<float> lastShowTime;

      glm::vec4 baseColor(1.f, 1.f, 1.f, 0.66f);

      if (showMenu){
        lastShowTime = std::nullopt;
        return MainMenu2Options {
          .backgroundColor = baseColor,
          .offsetY = 0.f, 
        };
      }

      if (wasShowingMenu && !showMenu){
        lastShowTime = gameapi -> timeSeconds(false);
      }
      if (!lastShowTime.has_value()){
        return std::nullopt;
      }

      auto timeElapsed = gameapi -> timeSeconds(false) - lastShowTime.value();
      if (timeElapsed > duration){
        return std::nullopt;
      }
      auto percentage = timeElapsed / duration;

      return MainMenu2Options{
        .backgroundColor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, (1.f - percentage) * baseColor.w),
        .offsetY = 0.f + percentage,
      };
   },
   .levels = LevelUIInterface {
      .goToLevel = [&gameState](Level& level) -> void {
        modassert(false, std::string("level ui goToLevel: ") + level.name);
        goToLevel(level.name);
      },
      .goToMenu = []() -> void {
        auto gamemodeIntro = std::get_if<GameModeIntro>(&gameStatePtr -> sceneManagement.managedScene.value().gameMode);
        auto gamemodeBall = std::get_if<GameModeBall>(&gameStatePtr -> sceneManagement.managedScene.value().gameMode);
        if (gamemodeIntro){
          goToLevel("ballselect");
          startIntroMode(gameStatePtr -> sceneManagement.managedScene.value().id.value());
        }else if (gamemodeBall){
          goToLevel("ballselect");
          ballModeLevelSelect();
        }else{
          pushHistory({ "mainmenu" }, true);
        }
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
      .setNormalMode = [&gameState]() -> void {
        auto wasInEditorMode = !isInGameMode();
        setActivePlayerEditorMode(false, getDefaultPlayerIndex());
        setShowFreecam(false);
        setShowEditor(false);
        setGlobalModeValues(false);

        if (wasInEditorMode){
          // reset scene does not work in the same frame so...just delay it for now... TODO HACKEY SHIT
          gameapi -> schedule(0, true, 0, NULL, [&gameState](void*) -> void {
            startLevel(gameState.sceneManagement.managedScene.value());
          });          
        }

      },
      .setShowEditor = [&gameState]() -> void {
        setActivePlayerEditorMode(true, getDefaultPlayerIndex());
        setShowFreecam(false);
        setShowEditor(true);
        setGlobalModeValues(true);

        bool liveEdit = false;
        if (!liveEdit){
          if (gameState.sceneManagement.managedScene.value().id.has_value()){
            gameapi -> resetScene(gameState.sceneManagement.managedScene.value().id.value());
          }
        }

      },
      .setFreeCam = []() -> void {
        setActivePlayerEditorMode(true, getDefaultPlayerIndex());
        setShowFreecam(true);
        setShowEditor(false);
      },
      .setNoClip = setNoClipMode,
      .setBackground = setMenuBackground,
      .goToLevel = [&gameState](std::optional<std::string> level) -> void {
        modlog("gotolevel", std::string("level loading: ") + level.value());
        goToLevel(level.value());
      },
      .nextLevel = []() -> void {
        modassert(false, "next level does not exist anymore");
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
      .die = []() -> void {
        killActivePlayer(getDefaultPlayerIndex());
      },
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
        ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());

        if(controlledPlayer.playerId.has_value()){
          deliverCurrentGunAmmo(controlledPlayer.playerId.value(), amount);
        }
      },
      .disableActiveEntity = [](bool enable) -> void {
        disableTpsMesh = enable;
      },
      .spawnByTag = [](std::string tag) -> void {
        spawnFromAllSpawnpoints(director.managedSpawnpoints, tag.c_str());       
      },
      .markLevelComplete = markLevelComplete,
    },
  };
  return uiContext;
}

ArcadeApi arcadeApi {
  .ensureSoundsLoaded = [](objid id, std::vector<std::string> sounds) -> std::vector<objid> {
      return ensureSoundLoadedBySceneId(id, rootSceneId(), sounds);
  },
  .releaseSounds = [](objid id) -> void {
    unloadManagedSounds(id);
  },
  .ensureTexturesLoaded = [](objid id, std::vector<std::string> textures) -> void {
    ensureManagedTexturesLoaded(id, rootSceneId(), textures);
  },
  .releaseTextures = unloadManagedTexturesLoaded,
  .playSound = [](objid clipId) -> void {
    playGameplayClipById(clipId, std::nullopt, std::nullopt);
  },
  .getResolution = [](objid id) -> glm::vec2 {
    auto texture = arcadeTextureId(id);
    if (texture.has_value()){
      return glm::vec2(1000, 1000); // this is overly coupled to the create texture call in tags
    }
    auto resolutionAttr = getWorldStateAttr("rendering", "resolution").value();
    glm::vec2* resolution = std::get_if<glm::vec2>(&resolutionAttr);
    modassert(resolution, "resolution value invalid");
    return *resolution;
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
    modlog("statecontroller state changed", std::to_string(entityId));
    tags.animationController.pendingAnimations.insert(entityId);
  }
}


// gameapi -> playAnimation(id, "walk",  LOOP, std::nullopt);
// gameapi -> playAnimation(id, "shoot", ONESHOT, { /* everything but walk */ });

void doStateControllerAnimations(){
  for (auto entityId : tags.animationController.pendingAnimations){
    if (!hasControllerState(tags.animationController, entityId)){
      continue;
    }
    auto stateAnimation = stateAnimationForController(tags.animationController, entityId);
    bool stateAnimationHasAnimation = stateAnimation && stateAnimation -> animation.has_value();
    bool matchingAnimation = stateAnimationHasAnimation && hasAnimation(entityId, stateAnimation -> animation.value());

    if (stateAnimationHasAnimation){
      modlog("statecontroller want animation", stateAnimation -> animation.value());
    }
    if (!disableAnimation && matchingAnimation){
      modlog("statecontroller animation controller play animation for state", nameForSymbol(stateAnimation -> state) + ", " + std::to_string(entityId) + ", " + print(stateAnimation -> animationBehavior));
      pushAlertMessage(nameForSymbol(stateAnimation -> state) + " " + stateAnimation -> animation.value());
//      gameapi -> playAnimation(entityId, stateAnimation -> animation.value(), stateAnimation -> animationBehavior, {});

      gameapi -> playAnimation(entityId, stateAnimation -> animation.value(), stateAnimation -> animationBehavior, controllableEntities.at(entityId).disableAnimationIds, 0, true, std::nullopt);  

    }else{
      if (stateAnimationHasAnimation && !matchingAnimation){
        if (validateAnimationControllerAnimations){
          modassert(false, std::string("no matching animation: ") + stateAnimation -> animation.value());
        }
        modlog("statecontroller animation controller play animation no matching animation for state", nameForSymbol(stateAnimation -> state) + ", for animation: " + stateAnimation -> animation.value() + ", " + std::to_string(entityId));
        pushAlertMessage(nameForSymbol(stateAnimation -> state) + " " + stateAnimation -> animation.value() + " -- missing animation");

      }
      modlog("statecontroller stop animation", std::to_string(entityId));
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
  .stopMoving = [](objid agentId) -> void {
    setEntityTargetLocation(gameStatePtr -> movementEntities, agentId, std::nullopt);
  },
  .look = [](objid agentId, glm::quat direction) -> void {
    setEntityTargetRotation(gameStatePtr -> movementEntities, agentId, direction);
  },
  .fireGun = [](objid agentId) -> void {
    fireGun(weapons, agentId);
  },
  .changeGun = [](objid agentId, const char* gun) -> void {
    maybeChangeGun(getWeaponState(weapons, agentId), gun,  agentId /*inventory */);
  },
  .changeTraits = [](objid agentId, const char* profile) -> void {
    changeMovementEntityType(getMovementData(), agentId, profile);
  },
  .playAnimation = [](objid agentId, const char* animation, AnimationType animationType){
    gameapi -> playAnimation(agentId, animation, animationType, std::nullopt, 0, false, std::nullopt);
  },
  .doDamage = doDamageMessage,
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


std::optional<objid> findChildObjBySuffix(objid id, const char* objName){
  auto children = gameapi -> getChildrenIdsAndParent(id);
  for (auto childId : children){
    auto name = gameapi -> getGameObjNameForId(childId).value();
    if (stringEndsWith(name, objName)){
      return childId;
    }
  }
  return std::nullopt;
}

void zoomIntoArcade(std::optional<objid> id, int playerIndex){
  bool zoomIn = id.has_value();
  setShowZoomArcade(zoomIn);
  setDisablePlayerControl(zoomIn, 0);
  if (!zoomIn){
    setTempCamera(std::nullopt, playerIndex);          
  }else{
    auto arcadeCameraId = findChildObjBySuffix(id.value(), ">camera");
    modassert(arcadeCameraId.has_value(), "arcadeCameraId does not have value");
    auto position = gameapi -> getGameObjectPos(id.value(), true, "[gamelogic] zoomIntoArcade get arcade camera location");
    auto rotation = gameapi -> getGameObjectRotation(id.value(), true, "[gamelogic] zoomIntoArcade get cmaera rotation");  // tempchecked
    setTempCamera(arcadeCameraId.value(), playerIndex);     
  }
}


// The default audio clip mechanism is kind of lame, since if you go between and audio zone
// and a default audio zone, it will replay the clip.  Some clips this is fine, but i don't 
// dig restarting it just because it uses a different system than the octree method. 
// I'd prefer this to be just using the tags system and be able to set a default tag value
// if you fall outside of the octree cell. 
struct AudioClip {
  objid id;
  std::string name;
};
std::unordered_map<std::string, AudioClip> audioClips;
std::optional<objid> octreeId;

std::optional<objid> defaultAudioClip;
int loadedDefaultAudioClipFrame = -1;
bool playingDefaultClip = false;

void ensureAllAudioZonesLoaded(){
  auto mainOctreeId = gameapi -> getMainOctreeId();

  bool changedOctreeState = false;
  if (octreeId != mainOctreeId){
    modlog("octree tags", "unloading");
    for (auto &[_, audioClip] : audioClips){
      gameapi -> removeByGroupId(audioClip.id);
    }
    audioClips = {};    
    octreeId = mainOctreeId;
    changedOctreeState = true;
  }

  if (changedOctreeState){
    modlog("octree tags", "loading");
    auto allTags = gameapi -> getAllTags(getSymbol("audio"));
    for (auto &tag : allTags){
      auto soundObjName = std::string("&code-sound") + uniqueNameSuffix();
      audioClips[tag.value] = AudioClip {
        .id = createSound(gameapi -> listSceneId(octreeId.value()), soundObjName, tag.value, true),
        .name = soundObjName,
      }; 
      modlog("octree tags load sound", tag.value);
    }
  }

  auto loadLevelFrame = gameStatePtr -> sceneManagement.changedLevelFrame;
  if (loadedDefaultAudioClipFrame < loadLevelFrame){
    playingDefaultClip = false;
    loadedDefaultAudioClipFrame = loadLevelFrame;
    modlog("octree tags - default audio", "reload");
    if (defaultAudioClip.has_value()){
      gameapi -> removeByGroupId(defaultAudioClip.value());
    }
    auto soundObjName = std::string("&code-sound") + uniqueNameSuffix();

    if (gameStatePtr -> sceneManagement.managedScene.has_value()){
      if (defaultAudioClipPath != ""){
        defaultAudioClip = createSound(gameapi -> rootSceneId(), soundObjName, defaultAudioClipPath, true);      
      }
    }
  }

}

std::optional<std::string> playingClip;
void ensureAmbientSound(std::vector<TagInfo>& tags){
  std::optional<std::string> clipToPlay;
  bool inAudioZone = false;
  if (tags.size() > 0){
    inAudioZone = true;
    clipToPlay = tags.at(tags.size() - 1).value;
  }
 

  ensureAllAudioZonesLoaded();

  if((!inAudioZone && playingClip.has_value()) || (inAudioZone && playingClip.has_value() && clipToPlay.has_value() && playingClip.value() != clipToPlay.value())){
    modlog("octree tags ensureAmbientSound", "stop clip");
    AudioClip& audioClip = audioClips.at(playingClip.value());
    auto sceneId = gameapi -> listSceneId(octreeId.value());
    gameapi -> stopClip(audioClip.name, sceneId);
    playingClip = std::nullopt;
  }
  if (inAudioZone && !playingClip.has_value()){
    if (playingDefaultClip){
      gameapi -> stopClipById(defaultAudioClip.value());
      playingDefaultClip = false;
    }

    modlog("octree tags ensureAmbientSound play clip", clipToPlay.value());

    modassert(audioClips.find(clipToPlay.value()) != audioClips.end(), "octree tags could not find clip");
    AudioClip& audioClip = audioClips.at(clipToPlay.value());
    playingClip = clipToPlay.value();

    playGameplayClipById(audioClip.id, std::nullopt, std::nullopt); 
  }

  if (!inAudioZone && !playingDefaultClip && defaultAudioClip.has_value()){
    playingDefaultClip = true;
    playGameplayClipById(defaultAudioClip.value(), std::nullopt, std::nullopt); 
  }

  std::cout << "tags ensure ambient sound: " << inAudioZone << std::endl;
}

void objectRemoved(objid idRemoved){
  for (auto& controlledPlayer : getPlayers()){
    if (controlledPlayer.playerId.has_value() && controlledPlayer.playerId.value() == idRemoved){
      controlledPlayer.playerId = std::nullopt;
    }
    if (controlledPlayer.activePlayerManagedCameraId.has_value() && controlledPlayer.activePlayerManagedCameraId.value() == idRemoved){
      controlledPlayer.activePlayerManagedCameraId = std::nullopt;
    }
    if (controlledPlayer.tempCamera.has_value() && controlledPlayer.tempCamera.value() == idRemoved){
      controlledPlayer.tempCamera = std::nullopt;
    }
  }

  maybeRemoveControllableEntity(aiData, gameStatePtr -> movementEntities, idRemoved);

  onActivePlayerRemoved(idRemoved);
  onMainUiObjectsChanged();

  onObjectRemovedWater(water, idRemoved);
  handleTagsOnObjectRemoved(tags, idRemoved);
  removeCameraFromOrbView(idRemoved);

  onObjRemoved(aiData, idRemoved);
}

void handleTeleport(objid idToTeleport, objid teleporterId){
  auto teleportPosition = gameapi -> getGameObjectPos(teleporterId, true, "gamelogic get teleport position");
  gameapi -> setGameObjectPosition(idToTeleport, teleportPosition, true, Hint { .hint = "teleport set posn" });
  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt);
}

void doTeleport(int32_t idToTeleport, std::string destination){
  std::cout << "doTeleport: " << idToTeleport << ", = " << destination << std::endl;
  for (auto& [teleporterId, teleportExit] : tags.teleportObjs){
    if (teleportExit.exit.has_value() && teleportExit.exit.value() == destination){
      handleTeleport(idToTeleport, teleporterId);
      std::cout << "doTeleport: found the exit" << std::endl;
      return;
    }
  }
}

void onKeyCallback(int32_t id, void* data, int key, int scancode, int action, int mods, int playerIndex){
  GameState* gameState = static_cast<GameState*>(data);

  ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

  if (action == 1){
    auto playerId = controlledPlayer.playerId;
    if (isPauseKey(key)){
      togglePauseIfInGame();
    }
    if (isTeleportButton(key) && !isPaused()){
      // this probably should be aware of the bounds, an not allow to clip into wall for example
      // maybe raycast down, and then set the position so it fits 
      auto teleportPosition = getTeleportPosition(tags);
      if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled(playerIndex) && teleportPosition.has_value()){
        handleTeleport(controlledPlayer.playerId.value(), teleportPosition.value().id);
        gameapi -> removeByGroupId(teleportPosition.value().id);
      }
    }
    if (isExitTerminalKey(key)){
      showTerminal(std::nullopt);
    }

    if (!getGlobalState().disableUiInput){
      onMainUiKeyPress(uiStateContext, gameState -> uiData.uiCallbacks, key, scancode, action, mods);
    }
    onInGameUiKeyCallback(key, scancode, action, mods);
  }
  handleHotkey(key, action);


  if (key == '-' && action == 0){
    setActivePlayerNext(movement, weapons, aiData, 0);
  }
  if (key == '=' && action == 0){
    observePlayerNext(movement, weapons, aiData, 0);
  }

  onVehicleKey(vehicles, key, action);

  if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled(playerIndex)){
    if (!(isPaused() || getGlobalState().disableGameInput)){
      onWeaponsKeyCallback(getWeaponState(weapons, controlledPlayer.playerId.value()), key, action, controlledPlayer.playerId.value());
    }
  }
  if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled(playerIndex)){
    onMovementKeyCallback(gameState -> movementEntities, movement, controlledPlayer.playerId.value(), key, action, controlledPlayer.viewport);
  }

  if (key == 'Q' && action == 0) { 
    printWorldInfo(aiData.worldInfo);
    if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled(playerIndex)){
      setInShootingMode(controlledPlayer.playerId.value(), !isInShootingMode(controlledPlayer.playerId.value()).value());
    }
  }

  debugOnKey(key, scancode, action, mods);

  if (isInteractKey(key) && (action == 1) && getGlobalState().zoomIntoArcade){
    zoomIntoArcade(std::nullopt, playerIndex);
  }
  onKeyArcade(key, scancode, action, mods);
  gametypesOnKey(gametypeSystem, key, scancode, action, mods);

  if (isInteractKey(key) && (action == 1) && controlledPlayer.playerId.has_value()){
    if (getActiveControllable(playerIndex).value() -> vehicle.has_value()){
      exitVehicleRaw(playerIndex, getActiveControllable(playerIndex).value() -> vehicle.value(), controlledPlayer.playerId.value());
    }else if (getActiveControllable(playerIndex).value() -> lookingAtVehicle.has_value()){
      enterVehicleRaw(playerIndex, getActiveControllable(playerIndex).value() -> lookingAtVehicle.value(), controlledPlayer.playerId.value());
    }
  }

  if (key == 'I' && action == 0){
    for (auto &[id, autodoor] : tags.autodoors){
      toggleAutodoor(id, autodoor);
    }
  }

  auto selectedOrb = handleOrbControls(orbData, key, action);
  if (selectedOrb.selectedOrb.has_value()){
    std::cout << "handleOrbViews orb: " << print(*selectedOrb.selectedOrb.value()) << std::endl;
    goToLevel(selectedOrb.selectedOrb.value() -> level);
    return;
  }

  if (isJumpKey(key) && action == 1){
    gameapi -> sendNotifyMessage("advance", true);
    return;
  }
}

void onMouseCallback(objid id, void* data, int button, int action, int mods, int playerIndex){
  GameState* gameState = static_cast<GameState*>(data);

  if (!getGlobalState().disableUiInput){
    onMainUiMousePress(uiStateContext, gameState -> uiData.uiContext, tags.uiData -> uiCallbacks, button, action, getGlobalState().selectedId);
  }
  onInGameUiMouseCallback(uiStateContext, tags.uiData -> uiContext, tags.inGameUi, button, action, getGlobalState().lookAtId /* this needs to come from the texture */);
  onMouseClickArcade(button, action, mods);

  modlog("input", std::string("on mouse down: button = ") + std::to_string(button) + std::string(", action = ") + std::to_string(action));

  if (button == 1){
    if (action == 0){
      gameState -> selecting = std::nullopt;
      getGlobalState().rightMouseDown = false;
    }else if (action == 1){
      gameState -> selecting = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
      getGlobalState().rightMouseDown = true;
      if (false){
        ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
        raycastFromCameraAndMoveTo(gameState -> movementEntities, controlledPlayer.playerId.value(), 0);
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

  ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
  if (controlledPlayer.playerId.has_value()){
    static float selectDistance = querySelectDistance();
    if (!isPaused() && !getGlobalState().disableGameInput && !isPlayerControlDisabled(playerIndex)){
      auto uiUpdate = onWeaponsMouseCallback(getWeaponState(weapons, controlledPlayer.playerId.value()), button, action, controlledPlayer.playerId.value(), selectDistance);
      if (uiUpdate.zoomAmount.has_value()){
        setTotalZoom(uiUpdate.zoomAmount.value(), controlledPlayer.playerId.value());
      }
    }
  }
}

void onMouseMoveCallback(objid id, void* data, double xPos, double yPos, float xNdc, float yNdc, int playerPort){ 
  GameState* gameState = static_cast<GameState*>(data);

  if (!getGlobalState().disableUiInput){
    onMainUiMouseMove(uiStateContext,  gameState -> uiData.uiContext, xPos, yPos, xNdc, yNdc);
  }
  onInGameUiMouseMoveCallback(tags.inGameUi, xPos, yPos, xNdc, yNdc);
  onMouseMoveArcade(xPos, yPos, xNdc, yNdc);

  float movementX = xNdc - getGlobalState().xNdc;
  float movementY = yNdc - getGlobalState().yNdc;
  getGlobalState().xNdc = xNdc;
  getGlobalState().yNdc = yNdc;
  getGlobalState().mouseVelocity = glm::vec2(xPos, yPos);

  { // per player code
    auto playerIndex = playerPort;
    ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
    if (controlledPlayer.playerId.has_value() && !isPaused() && !getGlobalState().disableGameInput && !isPlayerControlDisabled(playerIndex)){
      controlledPlayer.lookVelocity = glm::vec2(movementX, movementY);
    }
    if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled(playerIndex)){
      //glm::vec2 smoothedMovement = smoothVelocity(glm::vec2(xPos, yPos));
      glm::vec2 smoothedMovement = glm::vec2(xPos, yPos);
      onMovementMouseMoveCallback(gameState -> movementEntities, movement, controlledPlayer.playerId.value(), smoothedMovement.x, smoothedMovement.y, playerIndex);
    }
  }
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

    addPlayerPort(0);

    initGlobal();
    setGlobalModeValues(getGlobalState().showEditor);
    gameState -> dragSelect = std::nullopt;
    gameState -> uiData.uiContext = getUiContext(*gameState);

    crystals = loadCrystals();
    levelProgresses = loadLevelProgress();

    auto args = gameapi -> getArgs();
    if (args.find("dragselect") != args.end()){
      gameState -> dragSelect = args.at("dragselect");
      modlog("bindings", std::string("drag select value: ") + gameState -> dragSelect.value());
    }

    if (args.find("coop") != args.end()){
      setNumberPlayers(2);
      addPlayerPort(1);
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

    if (hasOption("config-server") && hasOption("config-connected") && hasOption("remote-mod")){
      auto configUrl = getArgOption("config-server");
      auto modpath = configUrl + getArgOption("remote-mod");
      modlog("remote mod load start", modpath);
      bool success = gameapi -> downloadFile(modpath, "../afterworld/mods/modfile");
      modlog("remote mod load success", modpath);

      gameapi -> mountPackage("../afterworld/mods/modfile");
    }
    
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
      "../gameresources/build/weapons/scrapgun.gltf",
      "../gameresources/build/weapons/electrogun.gltf",
      "../gameresources/build/weapons/fork.gltf",
    });
  
    tags = createTags(&gameState -> uiData);

    handleOnAddedTagsInitial(tags); // not sure i actually need this since are there any objects added?
    generateWaterMesh();
    
    //addWaterObj(gameapi -> rootSceneId());
    
    if (hasOption("arcade")){
      addArcadeType(-1, getArgOption("arcade"), std::nullopt);
      getGlobalState().disableUiInput = true;
    }


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

    auto info = gameapi -> getControlInfo(0);     
    if (info.has_value()){
      ControlInfo2& controls = info.value();
      auto keycallbacks = remapFrameToKeys(0, controls);
      for (auto& keycallback : keycallbacks){
        onKeyCallback(id, data, keycallback.key, keycallback.scancode, keycallback.action, keycallback.mods, keycallback.playerPort);
      }
      auto mouseCallbacks = remapFrameToMouse(0, controls);
      for (auto& mouseCallback : mouseCallbacks){
        onMouseCallback(id, data, mouseCallback.button, mouseCallback.action, mouseCallback.mods, mouseCallback.playerPort);
      }

      float deadzone = 0.15f;

      float rightStickX = controls.thisFrame.axisInfo.rightStickX;
      float rightStickY = controls.thisFrame.axisInfo.rightStickY;
      if (rightStickX < deadzone && rightStickX > (-1 * deadzone)){
        rightStickX = 0.f;
      }
      if (rightStickY < deadzone && rightStickY > (-1 * deadzone)){
        rightStickY = 0.f;
      }

      float sensitivity = 10.f;
      if (controls.thisFrame.buttonInfo.rightStick){
        sensitivity = 2.f;
      }
      onMouseMoveCallback(id, data, sensitivity * rightStickX, sensitivity * -1 * rightStickY, 0.f, 0.f, 0.f);

    }

    tickCutscenes2();

    std::vector<EntityUpdate> entityUpdates;
    //////// needs multiviewport work ///////////////////////////////

    if (isInGameMode()){
      // Control params are reset by movement so put before that
      onVehicleFrame(vehicles, getControlParamsByPort(movement, 0));

    }

    if (isInGameMode()){

      ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());
      if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled(getDefaultPlayerIndex())){
        std::vector<MovementActivePlayer> activePlayers;

        for (auto& player : getPlayers()){
          activePlayers.push_back(MovementActivePlayer {
            .activeId = player.playerId.value(),
            .playerPort = player.viewport,
          });
        }

        auto uiUpdate = onMovementFrame(gameState -> movementEntities, movement, isGunZoomed, disableTpsMesh, entityUpdates, activePlayers);
        setUiSpeed(uiUpdate.velocity, false ? uiUpdate.lookVelocity : std::nullopt);
      }

      impulses = {};

      std::vector<WeaponsUiUpdate> uiUpdates;
      if (controlledPlayer.playerId.has_value() && !isPaused() && !isPlayerControlDisabled(getDefaultPlayerIndex())){
        auto alive = activePlayerAlive(getDefaultPlayerIndex()).value();
        uiUpdates = onWeaponsFrame(weapons, controlledPlayer.playerId.value(), controlledPlayer.lookVelocity, getPlayerVelocity(getDefaultPlayerIndex()), getWeaponEntityData, 
          [](objid id) -> objid {
            ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());
            return controlledPlayer.activePlayerManagedCameraId.value();  // kind of wrong, but i think, kind of right in practice
          }, 
          ThirdPersonWeapon {
            .getWeaponParentId = [](objid id) -> std::optional<objid> {
              auto children = gameapi -> getChildrenIdsAndParent(id);
              for (auto childId : children){
                auto name = gameapi -> getGameObjNameForId(childId).value();
                if (stringEndsWith(name, "RightHand")){
                  return childId;
                }
              }
              return std::nullopt;
            },
          },
          !alive,
          alive
        );
      }

      for (auto& uiUpdate : uiUpdates){
        std::optional<objid> lookingAtVehicle;
        if (uiUpdate.raycastId.has_value()){
          if (isVehicle(vehicles, uiUpdate.raycastId.value()) &&  !getActiveControllable(getDefaultPlayerIndex()).value() -> vehicle.has_value()){
            lookingAtVehicle = uiUpdate.raycastId.value();
            gameapi -> drawText("Press E to enter", 0.f, 0.f, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
          }
        }
        getActiveControllable(getDefaultPlayerIndex()).value() -> lookingAtVehicle = lookingAtVehicle;

        setShowActivate(uiUpdate.showActivateUi);

        if (uiUpdate.ammoInfo.has_value()){
          setUIAmmoCount(uiUpdate.ammoInfo.value().currentAmmo, uiUpdate.ammoInfo.value().totalAmmo);
          DeliverAmmoMessage ammoArcadeMessage {
            .ammo = uiUpdate.ammoInfo.value().currentAmmo,
          };
          std::any value = ammoArcadeMessage;
          onMessageArcade(value);
        }else{
          setUIAmmoCount(0, 0);
        }
        if (uiUpdate.bloomAmount.has_value()){
          ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());
          drawBloom(controlledPlayer.playerId.value(), controlledPlayer.playerId.value(), -1.f, uiUpdate.bloomAmount.value()); // 0.002f is just a min amount for visualization, not actual bloom
        }
        modlog("current weapon", uiUpdate.currentGunName.has_value() ? *uiUpdate.currentGunName.value() : "");
        setUiWeapon(uiUpdate.currentGunName.has_value() ? *uiUpdate.currentGunName.value() : std::optional<std::string>(std::nullopt));

        if (uiUpdate.didFire){
          modlog("ui update", "fire gun");
          float magnitude = static_cast<int>(std::rand()) % 5;
          applyScreenshake(getDefaultPlayerIndex(), glm::vec3(magnitude * glm::cos(std::rand()), magnitude * glm::cos(std::rand()), 0.f));
        }else{
          setShowActivate(false);
          setUIAmmoCount(0, 0);
          setUiWeapon(std::nullopt);
        }

      }
    }

    drawAllCurves(id);

    ////////////////////////////////////////////////////
    if (isInGameMode()){
      for (auto& player : getPlayers()){
        auto playerPosition = getActivePlayerPosition(player.viewport);
        if (playerPosition.has_value()){
          drawWaypoints(waypoints, playerPosition.value());
        }
      }

      handleEntitiesOnRails(id, gameapi -> rootSceneId());
      //handleEntitiesRace();


      handleDirector(director);
      setUiGemCount(GemCount {
        .currentCount = numberOfCrystals(),
        .totalCount = totalCrystals(),
      });
    }

    //std::optional<glm::vec2> mainUiCursorCoord = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
    std::optional<glm::vec2> mainUiCursorCoord;
    gameState -> uiData.uiCallbacks = handleDrawMainUi(uiStateContext, tags.uiData -> uiContext, getGlobalState().selectedId, std::nullopt, mainUiCursorCoord);
    modassert(tags.uiData, "tags.uiData NULL");
    
    onInGameUiFrame(uiStateContext, tags.inGameUi, tags.uiData->uiContext, std::nullopt, ndiCoord);
    
    if (isInGameMode()){
      std::optional<UiHealth> uiHealth;

      {
        for (auto& player : getPlayers()){
          ControlledPlayer& controlledPlayer = getControlledPlayer(player.viewport);
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
          setUiHealth(player.viewport, uiHealth);
        }
      }
      onFrameWater(water);

      if (!hasOption("no-ai")){
        static bool showAi = getArgEnabled("ai-debug");
        onFrameAi(aiData, showAi);
      }
      onFrameDaynight();
      onGametypesFrame(gametypeSystem);

      updateArcade();
      drawArcade();
    }

    onTagsFrame(tags);
    handleOrbViews(orbData);
    
    doStateControllerAnimations();

    if (levelShortcutToLoad.has_value()){
      goToLevel(levelShortcutToLoad.value());
      levelShortcutToLoad = std::nullopt;
    }

    // debug
    debugOnFrame();

    for (auto &update : entityUpdates){
      if (update.pos.has_value()){
        gameapi -> setGameObjectPosition(update.id, update.pos.value(), true, Hint { .hint = update.posHint });  
      }
      if (update.rot.has_value()){
        gameapi -> setGameObjectRot(update.id, update.rot.value(), true, Hint { .hint = update.rotHint });
      }
    }

    /// temp code 
    // TODO multiviewport sound
    auto cameraPos = gameapi -> getCameraTransform(getDefaultPlayerIndex());
    auto audioSymbol = getSymbol("audio");
    auto tags = gameapi -> getTag(audioSymbol, cameraPos.position);
    std::cout << "tags game camera pos is: " << print(cameraPos.position) << ", tags: [";
    bool inAudioZone = false;
    for (auto tag : tags){
      std::cout << std::endl << "tags   " << nameForSymbol(tag.key) << " " << tag.value << std::endl;
      if (tag.key == audioSymbol){
        inAudioZone = true;     
      }
    }
    ensureAmbientSound(tags);
    std::cout << "]" << std::endl;

    static std::optional<OctreeMaterial> material;
    static std::optional<OctreeMaterial> lastMaterial;
    static float changeTime = 0.f;

    material = gameapi -> getMaterial(cameraPos.position);

    bool changed = material != lastMaterial;
    if (changed){
      changeTime = gameapi -> timeSeconds(false);
    }
    
    std::cout << "material is: " << ((material == OCTREE_MATERIAL_WATER) ? "water" : "not water") << std::endl;
    
    bool isWater = material.has_value() && material.value() == OCTREE_MATERIAL_WATER;
    float timeElapsed = gameapi -> timeSeconds(false) - changeTime;
    float fadeDuration = 0.1f;
    glm::vec3 tintColor(0.f, 0.1f, 0.3f);
    float alpha = timeElapsed / fadeDuration;
    if (alpha > 1){
      alpha = 1;
    }

    if (isInGameMode() || getGlobalState().isFreeCam){
      if (isWater){
        gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(tintColor.x, tintColor.y, tintColor.z, alpha * 0.3), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
      }else{
        // this is wrong since starts from 0.3
        gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(tintColor.x, tintColor.y, tintColor.z, 0.3 - (alpha * 0.3)), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
      }
    }
    lastMaterial = material;


    std::set<objid> lifetimeIdsToRemove;
    for (auto &[id, lifetimeObject] : lifetimeObjects){
      if (!gameapi -> gameobjExists(id)){
        lifetimeObject.fn();
        lifetimeIdsToRemove.insert(id);
      }
    }
    for (auto id : lifetimeIdsToRemove){
      std::cout << "lifetimeObject rm: hint = " << lifetimeObjects.at(id).hint << std::endl;
      lifetimeObjects.erase(id);
    }
    lifetimeIdsToRemove = {};
  };

  binding.onFrameAfterUpdate = [](int32_t id, void* data) -> void {
    modlog("onFrameAfterUpdate", "late frame update");
    GameState* gameState = static_cast<GameState*>(data);

    auto& allPlayers = getPlayers();
    for (auto& player : allPlayers){
      auto activePlayer = player.playerId;
      auto thirdPersonCamera = getCameraForThirdPerson(player.viewport);

      if (activePlayer.has_value()){
        auto id = activePlayer.value();
        if (getMovementData().movementEntities.find(id) == getMovementData().movementEntities.end()){
          continue;
        }
        MovementEntity& movementEntity = getMovementData().movementEntities.at(id);

        const float shakeStiffness = 100.f;
        const float shakeDamping = 20.f;

        if (player.shakeImpulse.has_value()){
          player.shakeVelocity += player.shakeImpulse.value();
          player.shakeImpulse = std::nullopt;
        }
        glm::vec3 springForce(-1 * player.shakeOffset.x * shakeStiffness, -1 * player.shakeOffset.y * shakeStiffness, -1 * player.shakeOffset.z * shakeStiffness);
        glm::vec3 dampingForce(-1 * player.shakeVelocity.x * shakeDamping, -1 * player.shakeVelocity.y * shakeDamping, -1 * player.shakeVelocity.z * shakeDamping);
        glm::vec3 acceleration(springForce.x + dampingForce.x, springForce.y + dampingForce.y, springForce.z + dampingForce.z);

        player.shakeVelocity.x += acceleration.x * gameapi -> timeElapsed();
        player.shakeVelocity.y += acceleration.y * gameapi -> timeElapsed();
        player.shakeVelocity.z += acceleration.z * gameapi -> timeElapsed();

        player.shakeOffset.x  += player.shakeVelocity.x * gameapi -> timeElapsed();
        player.shakeOffset.y  += player.shakeVelocity.y * gameapi -> timeElapsed();
        player.shakeOffset.z  += player.shakeVelocity.z * gameapi -> timeElapsed();

        if (thirdPersonCamera.has_value()){
          auto controllable = getActiveControllable(player.viewport);
          if (controllable.value() -> vehicle.has_value()){
            auto vehiclePos = gameapi -> getGameObjectPos(controllable.value() -> vehicle.value(), true, "[gamelogic] lateUpdate - vehicle camera pos");
          
            auto vehicleRot = gameapi -> getGameObjectRotation(controllable.value() -> vehicle.value(), true, "[gamelogic] lateUpdate - vehicle camera rot");
            auto offsetRot = quatFromDirection(glm::vec3(0.f, -1.f, -2.f));
            auto finalRot = vehicleRot * offsetRot;

            auto cameraOffset = finalRot * glm::vec3(0.f, 0.f, 10.f);

            auto thirdPerson = lookThirdPersonCalc(vehicles.vehicles.at(controllable.value() -> vehicle.value()).managedCamera, controllable.value() -> vehicle.value());
            glm::vec3 screenShake = thirdPerson.rotation * player.shakeOffset;

            gameapi -> setGameObjectPosition(thirdPersonCamera.value(), thirdPerson.position + screenShake, true, Hint { .hint = "[gamelogic] lateUpdate - set vehicle camera" });
            gameapi -> setGameObjectRot(thirdPersonCamera.value(), thirdPerson.rotation, true, Hint { .hint = "[gamelogic] lateUpdate - set vehicle camera" });

            return;
          }

          if (movementEntity.managedCamera.thirdPersonMode){
            auto thirdPersonInfo = lookThirdPersonCalc(movementEntity.managedCamera, id);
            glm::vec3 screenShake = thirdPersonInfo.rotation * player.shakeOffset;
            gameapi -> setGameObjectPosition(thirdPersonCamera.value(), thirdPersonInfo.position + screenShake, true, Hint { .hint = "[gamelogic] onMovementFrame1" });
            gameapi -> setGameObjectRot(thirdPersonCamera.value(), thirdPersonInfo.rotation, true, Hint { .hint = "[gamelogic] onMovementFrame2 rot" });        
          }else{
            auto rotation = weaponLookDirection(movementEntity.movementState);
            auto playerPos = gameapi -> getGameObjectPos(movementEntity.playerId, true, "[gamelogic] onMovementFrame - entity pos for set first person camera");
            glm::vec3 screenShake = rotation * player.shakeOffset;
            gameapi -> setGameObjectPosition(thirdPersonCamera.value(), playerPos + screenShake, true, Hint { .hint = "[gamelogic] onMovementFrame1" });
            gameapi -> setGameObjectRot(thirdPersonCamera.value(), rotation, true, Hint { .hint = "[gamelogic] onMovementFrame2 rot" });     
          } 
        }else{
          modassert(false, "no third person camaera uhhhh");
        }
      }
    }
  };

  binding.onKeyCallback = [](int32_t id, void* data, int rawKey, int rawScancode, int rawAction, int rawMods) -> void {
    auto keycallback = remapDeviceKeys(rawKey, rawScancode, rawAction, rawMods);
    onKeyCallback(id, data, keycallback.key, keycallback.scancode, keycallback.action, keycallback.mods, keycallback.playerPort);
  };

  binding.onController = [](int32_t id, void* data, int joystick, bool connected){
    modlog("controller connection event", std::to_string(joystick) + " - " + (connected ? "connected" : "disconnected"));
    std::cout << "controller onController debug" << std::endl;
    if (connected){

    }

  };
  binding.onControllerKey = [](int32_t id, void* data, int joystick, BUTTON_TYPE button, bool keyDown){
    modlog("controller key", std::to_string(joystick) + ", key = " + print(button) + ", keydown = " + (keyDown ? "true" : "false"));

    auto keycallbackOpt = remapControllerToKeys(joystick, button, keyDown);
    if (keycallbackOpt.has_value()){
      auto keycallback = keycallbackOpt.value();
      onKeyCallback(id, data, keycallback.key, keycallback.scancode, keycallback.action, keycallback.mods, keycallback.playerPort);
    }

    //auto values = gameapi -> controllerInput(joystick);
    //values.axis.leftTrigger > 
  };

  std::cout << "controller: created onControllerKey" << std::endl;

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    GameState* gameState = static_cast<GameState*>(data);

    if (key == "save-gun"){
      ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());
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
      spawnFromAllSpawnpoints(director.managedSpawnpoints, strValue -> c_str());
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
      goToLink(linkToValue.value());
    }

    if (key == "ammo"){
      auto itemAcquiredMessage = anycast<ItemAcquiredMessage>(value);
      modassert(itemAcquiredMessage != NULL, "ammo message not an ItemAcquiredMessage");
      deliverCurrentGunAmmo(itemAcquiredMessage -> targetId, itemAcquiredMessage -> amount);
    }
    if (key == "gem-pickup"){
      auto itemAcquiredMessage = anycast<ItemAcquiredMessage>(value);
      modassert(itemAcquiredMessage != NULL, "gem-pickup message not an ItemAcquiredMessage");
      auto position = gameapi -> getGameObjectPos(itemAcquiredMessage -> targetId, true, "[gamelogic] get position for gem pickup to play sound");
      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, position);

      auto gem = getSingleAttr(itemAcquiredMessage -> itemId, "gem-label");
      if (gem.has_value()){
        pickupCrystal(gem.value());
      }else{
        modassert(false, "no label for gem");
      }

      saveData();
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

    if (key == "arcade"){
      auto attrValue = anycast<MessageWithId>(value); 
      modassert(attrValue, "activate-switch message invalid arcade");
      zoomIntoArcade(attrValue -> id, getPlayerIndex(attrValue -> playerId.value()).value());
    }

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
    handleSpawnCollision(obj1, obj2);

    handleLevelEndCollision(obj1, obj2);
    handleTeleportCollision(obj1, obj2);
    handlePowerupCollision(obj1, obj2);
    handleTriggerZone(obj1, obj2);

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

  binding.onMouseMoveCallback = [](objid id, void* data, double rawXPos, double rawYPos, float rawXNdc, float rawYNdc) -> void { 
    //modlog("input", std::string("mouse move: ") + print(glm::vec2(xPos, yPos)));
    //modlog("input",  std::string("(xNdc, yNdc)") + print(glm::vec2(xNdc, yNdc)));

    auto mouseMoveCallback = remapMouseMovement(rawXPos, rawYPos, rawXNdc, rawYNdc);
    onMouseMoveCallback(id, data, mouseMoveCallback.xPos, mouseMoveCallback.yPos, mouseMoveCallback.xNdc, mouseMoveCallback.yNdc, mouseMoveCallback.playerPort);
  };

  binding.onMouseCallback = [](objid id, void* data, int rawButton, int rawAction, int rawMods) -> void {

    std::cout << "controller mouse rawButton: " << rawButton << ", action = " << rawAction << std::endl;

    auto mouseCallback = remapMouseCallback(rawButton, rawAction, rawMods);
    onMouseCallback(id, data, mouseCallback.button, mouseCallback.action, mouseCallback.mods, mouseCallback.playerPort);

  };

  binding.onScrollCallback = [](objid id, void* data, double rawAmount) -> void {
    auto scrollCallback = remapScrollCallback(rawAmount);

    if (!getGlobalState().disableUiInput){
      onMainUiScroll(uiStateContext, tags.uiData->uiContext, scrollCallback.amount);
    }
    onInGameUiScrollCallback(tags.inGameUi, scrollCallback.amount);
    onMovementScrollCallback(movement, scrollCallback.amount, scrollCallback.playerPort);
  };
  binding.onObjectAdded = [](int32_t _, void* data, int32_t idAdded) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    modlog("objchange onObjectAdded", gameapi -> getGameObjNameForId(idAdded).value());

    onAddControllableEntity(aiData, gameStatePtr -> movementEntities, idAdded);
    handleOnAddedTags(tags, idAdded);

    onMainUiObjectsChanged();
    onObjAdded(aiData, idAdded);
  };
  binding.onObjectRemoved = [](int32_t _, void* data, int32_t idRemoved) -> void {
    modlog("objchange onObjectRemoved", gameapi -> getGameObjNameForId(idRemoved).value());
    objectRemoved(idRemoved);
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

std::vector<const char*> getAdditionalPathsToValidate(){
  return paths::allResources;
}