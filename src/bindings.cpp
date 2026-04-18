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
std::vector<LevelProgress> levelProgresses;
std::unordered_map<objid, glm::vec3> impulses;
std::set<objid> objectsInKillplane;
std::unordered_map<objid, std::set<objid>> triggerZoneIdToElements;
std::unordered_map<objid, GlassTexture> objIdToGlassTexture;
std::unordered_map<objid, Laser> lasers;
std::unordered_map<objid, GravityWell> gravityWells;
std::unordered_map<objid, TriggerColor> triggerColors;
std::unordered_map<objid, TeleportExit> teleportObjs;
std::unordered_map<objid, Powerup> powerups;
std::unordered_map<objid, LinkGunObj> linkGunObj;
StateController animationController = createStateController();
OrbData orbData;
Water water;
SoundData soundData;
GameTypes gametypeSystem;
AiData aiData;
Weather weather;
Waypoints waypoints;
std::set<objid> textureScrollObjIds;
AudioZones audiozones;
InGameUi inGameUi;
std::unordered_map<objid, SpinObject> idToRotateTimeAdded;
std::unordered_map<objid, EmissionObject> emissionObjects;
std::unordered_map<objid, HealthColorObject> healthColorObjects;
std::unordered_map<objid, ExplosionObj> explosionObjects;
UiData* uiDataPtr = NULL;
std::optional<glm::vec3> oldGravity;  // wtf?
ArcadeApi arcadeApi = createArcadeApi();
std::unordered_map<objid, ExtraSurfaceVelocity> extraVelocity;

std::optional<std::string> levelShortcutToLoad;

struct LifeTimeObject {
  std::function<void()> fn;
  std::string hint;
};
std::unordered_map<objid, LifeTimeObject> lifetimeObjects;
std::optional<SceneRouterOptions*> currentRoute;

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

SceneManagement sceneManagement;
MovementEntityData movementEntities;
std::optional<std::string> dragSelect;
std::optional<glm::vec2> selecting;
UiData uiData;
DebugPrintType printType;

std::optional<std::string> activeLevel;
int numberOfPlayers = 1;
int mainPlayerControl = 0;
std::vector<ControlledPlayer> players; // TODO static state

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
  setZoomSensitivity(movementEntities, multiplier, id);
  playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt, false);
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
      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt, false);
    }
    downTime = gameapi -> timeSeconds(true);
  }
}

void togglePauseIfInGame(){
  bool paused = isPaused();
  setPausedMode(!paused);
}


void setGlobalModeValues(bool isEditorMode){
  showSpawnpoints(director.managedSpawnpoints, isEditorMode);
}


void onSceneRouteChange(SceneManagement& sceneManagement, std::string& currentPath){
  modlog("router scene route", std::string("path is: ") + currentPath);

  int currentIndex = 0;
  std::vector<std::string> params;
  auto router = getSceneRouter(currentPath, &currentIndex, &params);
  modlog("router scene route, router, has router = ", print(router.has_value()));
  //modassert(router.has_value(), std::string("no router for path: ") + currentPath);

  std::optional<SceneLoadInfo> sceneToLoad;
  std::optional<ScenarioOptions> scenarioOptions;

  int matchedRouterOption = 0;
  auto routerOptions = getRouterOptions(currentPath, &matchedRouterOption);
  currentRoute = routerOptions;

  // reset state

  getGlobalState().showGameOver = false;
  ///

  modassert(routerOptions.has_value(), std::string("no router options for: ") + currentPath);
  modlog("router", std::string("matched router option: ") + std::to_string(matchedRouterOption));

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
        stopMode(sceneManagement.managedScene.value().gameMode);
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
    startMode(sceneManagement.managedScene.value().gameMode, sceneManagement.managedScene.value().id.value());
  }
}

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
  if(sceneManagement.managedScene.has_value() && sceneManagement.managedScene.value().id.has_value()){
    return sceneManagement.managedScene.value().id.value();
  }

  auto selected = gameapi -> selected();
  if (selected.size() == 0){
    return std::nullopt;
  }
  auto selectedId = gameapi -> selected().at(0);
  auto sceneId = gameapi -> listSceneId(selectedId);
  return sceneId;
}

void goToMenu(){
  auto gamemodeIntro = std::get_if<GameModeIntro>(&sceneManagement.managedScene.value().gameMode);
  auto gamemodeBall = std::get_if<GameModeBall>(&sceneManagement.managedScene.value().gameMode);
  if (gamemodeIntro){
    goToLevel("ballselect");
    startIntroMode(sceneManagement.managedScene.value().id.value());
  }else if (gamemodeBall){
    goToLevel("ballselect");
    ballModeLevelSelect();
  }else{
    pushHistory({ "mainmenu" }, true);
  }
}
UiContext getUiContext(){
  std::function<void()> pause = []() -> void { 
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
   .showGameOver = []() -> bool { return getGlobalState().showGameOver; },
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
   .debugConfig = []() -> std::optional<DebugConfig> {
      if (printType == DEBUG_GLOBAL){
        return debugPrintGlobal();
      }
      if (printType == DEBUG_INVENTORY){
        debugPrintInventory(scopenameToInventory);
        return std::nullopt;
      }
      if (printType == DEBUG_GAMETYPE){
        return debugPrintGametypes(gametypeSystem);
      }
      if (printType == DEBUG_AI){
        return debugPrintAi(aiData);
      }
      if (printType == DEBUG_HEALTH){
        return debugPrintHealth();
      }
      if (printType == DEBUG_ACTIVEPLAYER){
        return debugPrintActivePlayer(getDefaultPlayerIndex());
      }
      if (printType == DEBUG_ANIMATION){
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
      .goToLevel = [](Level& level) -> void {
        modassert(false, std::string("level ui goToLevel: ") + level.name);
        goToLevel(level.name);
      },
      .goToMenu = goToMenu,
    },
    .pauseInterface = PauseInterface {
      .elapsedTime = []() -> float { return gameapi -> timeSeconds(true) - downTime; },
      .pause = pause,
      .resume = resume,
    },
    .worldPlayInterface = WorldPlayInterface {
      .isGameMode = []() -> bool { return getGlobalState().routeState.inGameMode; },
      .isPaused = isPaused,
      .enterGameMode = []() -> void {},
      .exitGameMode = []() -> void {},
      .pause = pause,
      .resume = resume,
      .saveScene = []() -> void {
        auto sceneId = activeSceneForSelected();
        modassert(sceneId.has_value(), "save scene - no active scene");
        gameapi -> saveScene(false /*include ids */, sceneId.value(), std::nullopt /* filename */);
      },
    },
    .listScenes = []() -> std::vector<std::string> { return gameapi -> listResources("scenefiles"); },
    .loadScene = [](std::string scene) -> void {
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
      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt, false);
    },
    .consoleInterface = ConsoleInterface {
      .setNormalMode = []() -> void {
        auto wasInEditorMode = !isInGameMode();
        setActivePlayerEditorMode(false, getDefaultPlayerIndex());
        setShowFreecam(false);
        setShowEditor(false);
        setGlobalModeValues(false);

        if (wasInEditorMode){
          // reset scene does not work in the same frame so...just delay it for now... TODO HACKEY SHIT
          gameapi -> schedule(0, true, 0, NULL, [](void*) -> void {
            //startLevel(gameState.sceneManagement.managedScene.value());
          });          
        }

      },
      .setShowEditor = []() -> void {
        setActivePlayerEditorMode(true, getDefaultPlayerIndex());
        setShowFreecam(false);
        setShowEditor(true);
        setGlobalModeValues(true);

        bool liveEdit = false;
        if (!liveEdit){
          if (sceneManagement.managedScene.value().id.has_value()){
            gameapi -> resetScene(sceneManagement.managedScene.value().id.value());
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
      .goToLevel = [](std::optional<std::string> level) -> void {
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
      .setShowDebugUi = [](DebugPrintType newPrintType) -> void {
        printType = newPrintType;
      },
      .showWeapon = [](bool showWeapon) -> void {
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

UiStateContext uiStateContext {
  .routerHistory = &getMainRouterHistory(),
  .uiState = createUiState(),
};


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

  maybeRemoveControllableEntity(aiData, movementEntities, idRemoved);

  onActivePlayerRemoved(idRemoved);
  onMainUiObjectsChanged();

  onObjectRemovedWater(water, idRemoved);
  handleTagsOnObjectRemoved(idRemoved);
  removeCameraFromOrbView(idRemoved);

  onObjRemoved(aiData, idRemoved);

  removeSurfaceModifier(idRemoved);
  handleRemoveKillplaneCollision(idRemoved);
  handleOnTriggerRemove(idRemoved);
}

void onKeyCallback(int32_t id, void* data, int key, int scancode, int action, int mods, int playerIndex){
  ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);

  if (action == 1){
    auto playerId = controlledPlayer.playerId;
    if (isPauseKey(key)){
      togglePauseIfInGame();
    }
    if (isTeleportButton(key) && !isPaused()){
      // this probably should be aware of the bounds, an not allow to clip into wall for example
      // maybe raycast down, and then set the position so it fits 
      auto teleportPosition = getTeleportPosition();
      if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled(playerIndex) && teleportPosition.has_value()){
        handleTeleport(controlledPlayer.playerId.value(), teleportPosition.value().id);
        gameapi -> removeByGroupId(teleportPosition.value().id);
      }
    }
    if (isExitTerminalKey(key)){
      showTerminal(std::nullopt);
    }

    if (!getGlobalState().disableUiInput){
      onMainUiKeyPress(uiStateContext, uiData.uiCallbacks, key, scancode, action, mods);
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
    onMovementKeyCallback(movementEntities, movement, controlledPlayer.playerId.value(), key, action, controlledPlayer.viewport);
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
      exitVehicleEntity(playerIndex, getActiveControllable(playerIndex).value() -> vehicle.value(), controlledPlayer.playerId.value());
    }else if (getActiveControllable(playerIndex).value() -> lookingAtVehicle.has_value()){
      enterVehicleEntity(playerIndex, getActiveControllable(playerIndex).value() -> lookingAtVehicle.value(), controlledPlayer.playerId.value());
    }
  }

  auto selectedOrb = handleOrbControls(orbData, key, action);
  onModeOrbSelect(selectedOrb);

  if (isJumpKey(key) && action == 1){
    gameapi -> sendNotifyMessage("advance", true);
    return;
  }
}

void onMouseCallback(objid id, void* data, int button, int action, int mods, int playerIndex){
  if (!getGlobalState().disableUiInput){
    onMainUiMousePress(uiStateContext, uiData.uiContext, uiDataPtr -> uiCallbacks, button, action, getGlobalState().selectedId);
  }
  onInGameUiMouseCallback(uiStateContext, uiDataPtr -> uiContext, inGameUi, button, action, getGlobalState().lookAtId /* this needs to come from the texture */);
  onMouseClickArcade(button, action, mods);
  onVehicleMouseClick(vehicles, button, action, mods);

  modlog("input", std::string("on mouse down: button = ") + std::to_string(button) + std::string(", action = ") + std::to_string(action));

  if (button == 1){
    if (action == 0){
      selecting = std::nullopt;
      getGlobalState().rightMouseDown = false;
    }else if (action == 1){
      selecting = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
      getGlobalState().rightMouseDown = true;
      if (false){
        ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
        raycastFromCameraAndMoveTo(movementEntities, controlledPlayer.playerId.value(), 0);
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
  if (!getGlobalState().disableUiInput){
    onMainUiMouseMove(uiStateContext,  uiData.uiContext, xPos, yPos, xNdc, yNdc);
  }
  onInGameUiMouseMoveCallback(inGameUi, xPos, yPos, xNdc, yNdc);
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
      onMovementMouseMoveCallback(movementEntities, movement, controlledPlayer.playerId.value(), smoothedMovement.x, smoothedMovement.y, playerIndex);
    }
  }
}

void onTranslateController(objid id, void* data){
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
}

CScriptBinding afterworldMainBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  if (getArgEnabled("help")){
    printGameOptionsHelp();
    exit(0);
  }

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    weapons = createWeapons();

    sceneManagement = createSceneManagement();
    movementEntities = MovementEntityData {};
    selecting = std::nullopt;
    uiData = {
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
    dragSelect = std::nullopt;
    uiData.uiContext = getUiContext();

    levelProgresses = loadLevelProgress();

    auto args = gameapi -> getArgs();
    if (args.find("dragselect") != args.end()){
      dragSelect = args.at("dragselect");
      modlog("bindings", std::string("drag select value: ") + dragSelect.value());
    }

    if (args.find("coop") != args.end()){
      setNumberPlayers(2);
      addPlayerPort(1);
    }

    if (getArgEnabled("debug-shoot")){
      setDrawDebugVector(true);
    }

    printType = DEBUG_NONE;

    if (getArgChoice("print-type", "inventory")){
      printType = DEBUG_INVENTORY;
    }else if (getArgChoice("print-type", "global")){
      printType = DEBUG_GLOBAL;
    }else if (getArgChoice("print-type", "gametype")){
      printType = DEBUG_GAMETYPE;
    }else if (getArgChoice("print-type", "ai")){
      printType = DEBUG_AI;
    }else if (getArgChoice("print-type", "health")){
      printType = DEBUG_HEALTH;
    }else if (getArgChoice("print-type", "active")){
      printType = DEBUG_ACTIVEPLAYER;
    }else if (getArgChoice("print-type", "animation")){
      printType = DEBUG_ANIMATION;
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
      []() -> void {  // I hate this callback.  I should just query a flag in the main loop and do it intentionally
        auto currentPath = fullHistoryStr();
        onSceneRouteChange(sceneManagement, currentPath);
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
  
    addAnimationController(animationController);

    uiDataPtr = &uiData;

    handleOnAddedTagsInitial(); // not sure i actually need this since are there any objects added?
    generateWaterMesh();
        
    if (hasOption("arcade")){
      addArcadeType(-1, getArgOption("arcade"), std::nullopt);
      getGlobalState().disableUiInput = true;
    }


    return NULL;
  };

  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    removeAllMovementCores();  // is this pointless?
  };

  binding.onFrame = [](int32_t id, void* data) -> void { 
    // CONTROLS ///////////////////////////
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
    if (dragSelect.has_value() && selecting.has_value()){
      //selectWithBorder(gameState -> selecting.value(), glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc));
    }

    onTranslateController(id, data);

    if (currentRoute.has_value()){
      auto interactState = currentRoute.value() -> getInteract();
      setRouterGameState(RouteState{
        .paused = interactState.paused,
        .inGameMode = interactState.inGameMode,
        .showMouse = interactState.showMouse,
      });
      setPaused(interactState.paused);
    }


    if (levelShortcutToLoad.has_value()){
      goToLevel(levelShortcutToLoad.value());
      levelShortcutToLoad = std::nullopt;
    }

    // CORE LOGIC ///////////////////////// 
    // This has some mixed logic with ui stuff and some non core stuff
    std::vector<EntityUpdate> entityUpdates;
    //////// needs multiviewport work ///////////////////////////////

    if (isInGameMode()){
      onVehicleFrame(vehicles, getControlParamsByPort(movement, 0));

      ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());
      if (controlledPlayer.playerId.has_value() && !isPlayerControlDisabled(getDefaultPlayerIndex())){
        std::vector<MovementActivePlayer> activePlayers;

        for (auto& player : getPlayers()){
          activePlayers.push_back(MovementActivePlayer {
            .activeId = player.playerId.value(),
            .playerPort = player.viewport,
          });
        }

        auto uiUpdate = onMovementFrame(movementEntities, movement, isGunZoomed, disableTpsMesh, entityUpdates, activePlayers);
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

    auto cameraPos = gameapi -> getCameraTransform(getDefaultPlayerIndex());

    /// GENERAL UPDATES
    {
      tickCutscenes2();
      if (isInGameMode()){
        handleEntitiesOnRails(id, gameapi -> rootSceneId());
        handleDirector(director);
        //handleEntitiesRace();
        onFrameWater(water, isPaused());
        if (!hasOption("no-ai")){
          static bool showAi = getArgEnabled("ai-debug");
          onFrameAi(aiData, showAi);
        }
        onFrameDaynight();
        onGametypesFrame(gametypeSystem);
  
        onTagsFrame();
        handleOrbViews(orbData);
  
        doStateControllerAnimations(validateAnimationControllerAnimations, disableAnimation);
  
        /// temp code 
        // TODO multiviewport sound
        ensureAmbientSound(cameraPos.position, sceneManagement.changedLevelFrame, sceneManagement.managedScene.has_value());
      }
      debugOnFrame();
    }

    // UI UPDATES //////////////////
    {
      drawAllCurves(id);
      if (isInGameMode()){
        for (auto& player : getPlayers()){
          auto playerPosition = getActivePlayerPosition(player.viewport);
          if (playerPosition.has_value()){
            drawWaypoints(waypoints, playerPosition.value());
          }
        }
        drawWaterOverlay(isInGameMode(), getGlobalState().isFreeCam, cameraPos.position);
  
        setUiGemCount(GemCount {
          .currentCount = numberOfCrystals(std::nullopt),
          .totalCount = totalCrystals(std::nullopt),
        });
      }
    
      //std::optional<glm::vec2> mainUiCursorCoord = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
      std::optional<glm::vec2> mainUiCursorCoord;
      uiData.uiCallbacks = handleDrawMainUi(uiStateContext, uiDataPtr -> uiContext, getGlobalState().selectedId, std::nullopt, mainUiCursorCoord);
      modassert(uiDataPtr, "uiDataPtr NULL");
    
      onInGameUiFrame(uiStateContext, inGameUi, uiDataPtr->uiContext, std::nullopt, ndiCoord);
    
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
      }
    }

    // ARCADE ///////////////
    if (isInGameMode()){
      updateArcade();
      drawArcade();  
    }

    // POST UPDATE STATEKEEPING //////////////////////
    for (auto &update : entityUpdates){
      if (update.pos.has_value()){
        gameapi -> setGameObjectPosition(update.id, update.pos.value(), true, Hint { .hint = update.posHint });  
      }
      if (update.rot.has_value()){
        gameapi -> setGameObjectRot(update.id, update.rot.value(), true, Hint { .hint = update.rotHint });
      }
    }
    
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

    auto& allPlayers = getPlayers();
    for (auto& player : allPlayers){
      auto activePlayer = player.playerId;
      auto thirdPersonCamera = getCameraForThirdPerson(player.viewport);

      if (activePlayer.has_value()){
        auto id = activePlayer.value();
        if (movementEntities.movementEntities.find(id) == movementEntities.movementEntities.end()){
          continue;
        }
        MovementEntity& movementEntity = movementEntities.movementEntities.at(id);

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

            auto thirdPerson = lookThirdPersonCalc(vehicles.vehicles.at(controllable.value() -> vehicle.value()).state.managedCamera, controllable.value() -> vehicle.value(), false);
            glm::vec3 screenShake = thirdPerson.rotation * player.shakeOffset;

            gameapi -> setGameObjectPosition(thirdPersonCamera.value(), thirdPerson.position + screenShake, true, Hint { .hint = "[gamelogic] lateUpdate - set vehicle camera" });
            gameapi -> setGameObjectRot(thirdPersonCamera.value(), thirdPerson.rotation, true, Hint { .hint = "[gamelogic] lateUpdate - set vehicle camera" });

            return;
          }

          if (movementEntity.managedCamera.thirdPersonMode){
            auto thirdPersonInfo = lookThirdPersonCalc(movementEntity.managedCamera, id, false);
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
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    if (key == "save-gun"){
      ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());
      saveGunTransform(getWeaponState(weapons, controlledPlayer.playerId.value()).weaponValues);
    }

    if (key == "reset"){
      pushHistory({ "mainmenu" }, true);
      return;
    }
    if (key == "reload-config:levels"){
      sceneManagement.levels = loadLevels();
      return;
    }

    if (key == "selected"){
      auto gameObjId = anycast<objid>(value); 
      modassert(gameObjId, "selected value invalid");
      if (!gameapi -> getGameObjNameForId(*gameObjId).has_value()){
        return;
      }
      handleDialogInteract(*gameObjId);
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
      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt, false);
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
      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, position, false);

      auto gem = getSingleAttr(itemAcquiredMessage -> itemId, "gem-label");
      if (gem.has_value()){
        pickupCrystal(gem.value());
      }else{
        modassert(false, "no label for gem");
      }

      saveData();
    }


    if (key == "play-material-sound"){
      auto soundPosition = anycast<MessagePlaySound>(value);
      modassert(soundPosition != NULL, "sound position not given");
      playMaterialSound(soundData, gameapi -> rootSceneId(), soundPosition -> position, soundPosition -> material);
    }

    if (key == "arcade"){
      auto attrValue = anycast<MessageWithId>(value); 
      modassert(attrValue, "message invalid arcade");
      zoomIntoArcade(attrValue -> id, getPlayerIndex(attrValue -> playerId.value()).value());
    }

    gametypesOnMessage(gametypeSystem, key, value);
    onAiOnMessage(aiData, key, value);

    if (key == "weather"){
      onWeatherMessage(weather, value, gameapi -> rootSceneId());
    }

    onTagsMessage(key, value);

    onDebugMessage(key, value);
  };

  binding.onCollisionEnter = [](objid id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal, float force) -> void {
    auto gameobj1Exists = gameapi -> gameobjExists(obj1); // this check shouldn't be necessary, is bug
    auto gameobj2Exists = gameapi -> gameobjExists(obj2);
    modassert(gameobj1Exists && gameobj2Exists, "collision enter: objs do not exist");

    auto direction = orientationFromPos(glm::vec3(0.f, 0.f, 0.f), normal) * glm::vec3(0.f, 0.f, -10.f);
    drawDebugRaycast(pos, pos + direction, -1);

    handleCollision(obj1, obj2, "switch-enter", "switch-enter-key", "enter");
    handleCollisionDamage(obj1, obj2);
    handleKillplaneCollision(obj1, obj2);
    handleGravityHoleCollision(obj1, obj2);
    handleMomentumCollision(obj1, obj2, pos, normal, force);
    handleCollisionBouncepad(obj1, obj2, normal);
    handleInventoryOnCollision(obj1, obj2);
    handleSpawnCollision(director, obj1, obj2);

    handleLevelEndCollision(obj1, obj2);
    handleCollisionTeleport(obj1, obj2);
    handlePowerupCollision(obj1, obj2);
    handleGemCollision(obj1, obj2);
    
    handleTriggerZone(obj1, obj2);
    handleOnTriggerEnter(obj1, obj2);

    handleSurfaceCollision(obj1, obj2);

    onCollisionEnterWater(water, obj1, obj2);
    onCollisionEnterSound(soundData, gameapi -> rootSceneId(), obj1, obj2, pos);
  };

  binding.onCollisionExit = [](objid id, void* data, int32_t obj1, int32_t obj2) -> void {
    auto gameobj1Exists = gameapi -> gameobjExists(obj1);
    auto gameobj2Exists = gameapi -> gameobjExists(obj2);
    modassert(gameobj1Exists && gameobj2Exists, "collision exit: objs do not exist");
    handleCollision(obj1, obj2, "switch-exit", "switch-exit-key", "exit");
    handleRemoveKillplaneCollision(obj1, obj2);
    onCollisionExitWater(water, obj1, obj2);
    removeSurfaceModifier(obj1, obj2);
    handleOnTriggerExit(obj1, obj2);
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
      onMainUiScroll(uiStateContext, uiDataPtr->uiContext, scrollCallback.amount);
    }
    onInGameUiScrollCallback(inGameUi, scrollCallback.amount);
    onMovementScrollCallback(movement, scrollCallback.amount, scrollCallback.playerPort);
  };

  binding.onObjectAdded = [](int32_t _, void* data, int32_t idAdded) -> void {
    modlog("objchange onObjectAdded", gameapi -> getGameObjNameForId(idAdded).value());

    onAddControllableEntity(aiData, movementEntities, idAdded);
    handleOnAddedTags(idAdded);

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

void setupGameCompileFn(std::unordered_map<std::string, std::string>& args){
  auto ballgameCompile = getCompileMapForBallGame();
  setCompileFn(ballgameCompile);
}