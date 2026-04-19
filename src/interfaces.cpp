#include "./interfaces.h"

std::vector<objid> ensureSoundLoadedBySceneId(objid id, objid sceneId, std::vector<std::string>& soundsToLoad);
void unloadManagedSounds(objid id);
void ensureManagedTexturesLoaded(objid id, objid sceneId, std::vector<std::string> textures);
void unloadManagedTexturesLoaded(objid id);

extern MovementEntityData movementEntities;

ArcadeApi createArcadeApi(){
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
      playGameplayClipById(clipId, std::nullopt, std::nullopt, false);
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
  return arcadeApi;
}

AIInterface aiInterface {
  .move = [](objid agentId, glm::vec3 targetPosition, float speed) -> void {
    setEntityTargetLocation(movementEntities, agentId, MovementRequest {
      .position = targetPosition,
      .speed = speed * 0.6f,
    });
  },
  .stopMoving = [](objid agentId) -> void {
    setEntityTargetLocation(movementEntities, agentId, std::nullopt);
  },
  .look = [](objid agentId, glm::quat direction) -> void {
    setEntityTargetRotation(movementEntities, agentId, direction);
  },
  .fireGun = [](objid agentId) -> void {
    fireGun(weapons, agentId);
  },
  .changeGun = [](objid agentId, const char* gun) -> void {
    maybeChangeGun(getWeaponState(weapons, agentId), gun,  agentId /*inventory */);
  },
  .changeTraits = [](objid agentId, const char* profile) -> void {
    changeMovementEntityType(movementEntities, agentId, profile);
  },
  .playAnimation = [](objid agentId, const char* animation, AnimationType animationType){
    gameapi -> playAnimation(agentId, animation, animationType, std::nullopt, 0, false, std::nullopt);
  },
  .doDamage = doDamageMessage,
};


extern DebugPrintType printType;
extern std::optional<ZoomOptions> zoomOptions;
extern std::unordered_map<objid, Inventory> scopenameToInventory;
extern GameTypes gametypeSystem;
extern Director director;
extern AiData aiData;
extern bool disableTpsMesh;
extern double downTime;

std::optional<objid> activeSceneForSelected();
void goToLevel(std::string levelShortName);
void setGlobalModeValues(bool isEditorMode);
void setNoClipMode(bool enable);
void setPausedMode(bool shouldBePaused);
void goToMenu();
void doToggleShowEditor();

void pauseOnMenu(){
  setPausedMode(true); 
}
void resumeOnMenu(){
  setPausedMode(false); 
}
UiContext getUiContext(){
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
   .showScreenspaceGrid = []() -> bool { return getGlobalState().systemConfig.showScreenspaceGrid; },
   .showGameHud = []() -> bool { return getGlobalState().showGameHud && !isPlayerControlDisabled(getDefaultPlayerIndex()); },
   .showGameOver = []() -> bool { return getGlobalState().showGameOver; },
   .showPause = []() -> bool { 
        return getGlobalState().routeState.paused && !getGlobalState().systemConfig.showConsole;
    },
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
      return getGlobalState().systemConfig.showKeyboard;
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
      .pause = pauseOnMenu,
      .resume = resumeOnMenu,
    },
    .worldPlayInterface = WorldPlayInterface {
      .isGameMode = []() -> bool { return getGlobalState().routeState.inGameMode; },
      .isPaused = isPaused,
      .enterGameMode = []() -> void {},
      .exitGameMode = []() -> void {},
      .pause = pauseOnMenu,
      .resume = resumeOnMenu,
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
        doToggleShowEditor();
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