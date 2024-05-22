#include "./bindings.h"


CustomApiBindings* gameapi = NULL;

struct SceneManagement {
  std::vector<Level> levels;
  std::optional<std::string> loadedLevel;
  bool menuLoaded;
  std::optional<objid> modelViewerScene;
  std::optional<objid> particleViewerScene;
};


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
    .menuLoaded = false,
    .modelViewerScene = std::nullopt,
    .particleViewerScene = std::nullopt,
  };
}




/////////////////////////////////////////////////////









std::vector<std::string> managedTags = { "game-level" };

void unloadAllManagedScenes(){
  auto managedScenes = gameapi -> listScenes(managedTags);
  for (auto sceneId : managedScenes){
    gameapi -> unloadScene(sceneId);
  }
}

void goToLevel(SceneManagement& sceneManagement, std::string sceneName){
  unloadAllManagedScenes();
  sceneManagement.menuLoaded = false;
  setPaused(false);
  sceneManagement.loadedLevel = sceneName;
  auto sceneId = gameapi -> loadScene(sceneName, {}, std::nullopt, managedTags);
  setActivePlayer(gameapi -> getGameObjectByName(">maincamera", sceneId, false)); 
  enterGameMode();
  pushHistory("playing", true);
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

void goToMenu(SceneManagement& sceneManagement){
  exitGameMode();
  if (sceneManagement.loadedLevel.has_value()){
    sceneManagement.loadedLevel = std::nullopt;
    unloadAllManagedScenes();
  }
  if (!sceneManagement.menuLoaded){
    gameapi -> loadScene("../afterworld/scenes/menu.rawscene", {}, std::nullopt, managedTags);
    sceneManagement.menuLoaded = true;
  }
  pushHistory("mainmenu", true);
}

double downTime = 0;
void setPausedMode(bool shouldBePaused){
  setPaused(shouldBePaused);
  if (!shouldBePaused){
    if (getCurrentPath() == "paused"){  // probably shouldn't have the ui routing information here
      popHistory();
    }
    downTime = gameapi -> timeSeconds(true);
  }else{
    if (getCurrentPath() == "playing"){
      pushHistory("paused");
    }
    downTime = gameapi -> timeSeconds(true);
  }
}

void togglePauseMode(){
  bool paused = isPaused();
  setPausedMode(!paused);
}


struct GameState {
  SceneManagement sceneManagement;

  std::optional<std::string> dragSelect;
  std::optional<glm::vec2> selecting;

  HandlerFns uiCallbacks;
  UiContext uiContext;
};


UiContext getUiContext(GameState& gameState){
  std::function<void()> pause = [&gameState]() -> void { 
    setPausedMode(true); 
  };
  std::function<void()> resume = []() -> void { 
    setPausedMode(false); 
  };
  UiContext uiContext {
   .isDebugMode = []() -> bool { 
     return getStrWorldState("editor", "debug").value() == "true"; 
   },
   .showEditor = []() -> bool {
    return getGlobalState().showEditor;
   },
   .showConsole = []() -> bool {
     static bool canEnableConsole = queryConsoleCanEnable();
     if (!canEnableConsole){
      return false;
     }
     return getGlobalState().showConsole;
   },
   .showScreenspaceGrid = []() -> bool { return getGlobalState().showScreenspaceGrid; },
   .showGameHud = []() -> bool { return !getGlobalState().paused && getGlobalState().inGameMode; },
   .levels = LevelUIInterface {
      .goToLevel = [&gameState](Level& level) -> void {
        goToLevel(gameState.sceneManagement, level.scene);
      },
      .goToMenu = [&gameState]() -> void {
        goToMenu(gameState.sceneManagement);
      }
    },
    .pauseInterface = PauseInterface {
      .elapsedTime = gameapi -> timeSeconds(true) - downTime,
      .pause = pause,
      .resume = resume,
    },
    .worldPlayInterface = WorldPlayInterface {
      .isGameMode = []() -> bool { return getGlobalState().inGameMode; },
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
      std::cout << "load scene placeholder: " << scene << std::endl;
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
      .setShowEditor = [](bool shouldShowEditor) -> void {
        updateShowEditor(shouldShowEditor);
        queryUpdateShowEditor(shouldShowEditor);
      },
      .setBackground = [](std::string background) -> void {
        gameapi -> sendNotifyMessage("menu-background", background);
      },
      .goToLevel = [&gameState](std::optional<std::string> level) -> void {
        if (!level.has_value()){
          goToMenu(gameState.sceneManagement);
          return;
        }
        auto scene = levelByShortcutName(level.value());
        if (scene.has_value()){
          goToLevel(gameState.sceneManagement, scene.value());
        }else{
          std::cout << "AFTERWORLD: no level found for shortcut: " << level.value() << std::endl;
        }
      },
      .routerPush = [](std::string route, bool replace) -> void {
        pushHistory(route, replace);
      },
      .routerPop = []() -> void {
        popHistory();
      },
    },
  };
  return uiContext;
}


AIInterface aiInterface {
  .move = [](objid agentId, glm::vec3 targetPosition, float speed) -> void {
    setEntityTargetLocation(agentId, MovementRequest {
      .position = targetPosition,
      .speed = speed,
    });
  },
};


void ensureViewLoaded(SceneManagement& sceneManagement, bool loadViewer, const char* scene, std::optional<objid>& sceneId){
  if (!loadViewer && sceneId.has_value()){
    gameapi -> unloadScene(sceneId.value());
    sceneId = std::nullopt;
    return;
  }
  if (loadViewer && !sceneId.has_value()){
    unloadAllManagedScenes();
    sceneManagement.menuLoaded = false;
    sceneId = gameapi -> loadScene(scene, {}, std::nullopt, managedTags);
    auto cameraId = gameapi -> getGameObjectByName(">maincamera", sceneId.value(), false).value();
    setActivePlayer(cameraId);
    return;
  }  
}
void ensureModelViewerLoaded(SceneManagement& sceneManagement, bool loadModelViewer){
  ensureViewLoaded(sceneManagement, loadModelViewer, "../afterworld/scenes/dev/models.rawscene", sceneManagement.modelViewerScene);
}
void ensureParticleViewerLoaded(SceneManagement& sceneManagement, bool loadParticleViewer){
  ensureViewLoaded(sceneManagement, loadParticleViewer, "../afterworld/scenes/dev/particles.rawscene", sceneManagement.particleViewerScene);
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
    goToMenu(gameState -> sceneManagement);
    auto args = gameapi -> getArgs();
    if (args.find("level") != args.end()){
      auto scene = levelByShortcutName(args.at("level"));
      if (scene.has_value()){
        goToLevel(gameState -> sceneManagement, scene.value());
      }else{
        std::cout << "AFTERWORLD: no level found for shortcut: " << args.at("level") << std::endl;
      }
    }
    gameState -> dragSelect = std::nullopt;
    gameState -> uiContext = getUiContext(*gameState);    
    if (args.find("dragselect") != args.end()){
      gameState -> dragSelect = args.at("dragselect");
      modlog("bindings", std::string("drag select value: ") + gameState -> dragSelect.value());
    }
    setPaused(true);
    initSettings();

    registerOnRouteChanged([gameState]() -> void {
      auto currentPath = getCurrentPath();
      ensureModelViewerLoaded(gameState -> sceneManagement, currentPath == "modelviewer");
      ensureParticleViewerLoaded(gameState -> sceneManagement, currentPath == "particleviewer");
      std::cout << "registerOnRouteChanged: , new route: " << getCurrentPath() << std::endl;
    });

    return gameState;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    delete gameState;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    gameapi -> idAtCoordAsync(getGlobalState().xNdc, getGlobalState().yNdc, false, [](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
      getGlobalState().selectedId = selectedId;
      getGlobalState().texCoordUv = texCoordUv;
      std::cout << "tex coord: " << print(texCoordUv) << std::endl;
    });

    gameapi -> idAtCoordAsync(0.f, 0.f, false, [](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
      getGlobalState().texCoordUvView = texCoordUv;
    });

    gameState -> uiCallbacks = handleDrawMainUi(gameState -> uiContext, getGlobalState().selectedId );

    if (gameState -> dragSelect.has_value() && gameState -> selecting.has_value()){
      //selectWithBorder(gameState -> selecting.value(), glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc));
    }
    onPlayerFrame();
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    auto hasInputKey = gameapi -> unlock("input", id);
    if (!hasInputKey){
      return;
    }

    if (action == 1){
      if (isPauseKey(key)){
        togglePauseMode();
      }
      onMainUiKeyPress(gameState -> uiCallbacks, key, scancode, action, mods);
    }
    handleHotkey(key, action);


    if (key == '-' && action == 0){
      setActivePlayerNext();
    }
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    GameState* gameState = static_cast<GameState*>(data);
    if (key == "reset"){
      goToMenu(gameState -> sceneManagement);
      return;
    }
    if (key == "game-over"){
      goToMenu(gameState -> sceneManagement);
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

    if (key == "hud-health"){
      auto floatValue = anycast<float>(value);
      modassert(floatValue != NULL, "hud-health value invalid");
      setHealth(*floatValue);
    }
    if (key == "current-gun"){
      auto currentGunMessage = anycast<CurrentGunMessage>(value); 
      modassert(currentGunMessage != NULL, "current-gun value invalid");
      setUIAmmoCount(AmmoHudInfo {
        .currentAmmo = currentGunMessage -> currentAmmo,
        .totalAmmo = currentGunMessage -> totalAmmo,
      });
    }

  };

  binding.onCollisionEnter = [](objid id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal) -> void {
    auto gameobj1Exists = gameapi -> gameobjExists(obj1); // this check shouldn't be necessary, is bug
    auto gameobj2Exists = gameapi -> gameobjExists(obj2);
    modassert(gameobj1Exists && gameobj2Exists, "collision enter: objs do not exist");
    handleCollision(obj1, obj2, "switch-enter", "switch-enter-key", "enter");
    handleDamageCollision(obj1, obj2);
    inventoryOnCollision(obj1, obj2);
  };
  binding.onCollisionExit = [](objid id, void* data, int32_t obj1, int32_t obj2) -> void {
    auto gameobj1Exists = gameapi -> gameobjExists(obj1);
    auto gameobj2Exists = gameapi -> gameobjExists(obj2);
    modassert(gameobj1Exists && gameobj2Exists, "collision exit: objs do not exist");
    handleCollision(obj1, obj2, "switch-exit", "switch-exit-key", "exit");
  };

  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    //std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    //std::cout << "(xNdc, yNdc)" << xNdc << ", " << yNdc << std::endl;
    getGlobalState().xNdc = xNdc;
    getGlobalState().yNdc = yNdc;
  };

  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    onMainUiMousePress(gameState -> uiCallbacks, button, action, getGlobalState().selectedId);

    std::cout << "on mouse down: button = " << button << ", action = " << action << std::endl;
    if (button == 1){
      if (action == 0){
        gameState -> selecting = std::nullopt;
        getGlobalState().rightMouseDown = false;
      }else if (action == 1){
        gameState -> selecting = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
        getGlobalState().rightMouseDown = true;
        if (false){
          raycastFromCameraAndMoveTo(getActivePlayerId().value());
        }
      }
    }else if (button == 0){
      if (action == 0){
        getGlobalState().leftMouseDown = false;
      }else if (action == 1){
        getGlobalState().leftMouseDown = true;
      }
    }else if (button == 2){
      if (action == 0){
        getGlobalState().middleMouseDown = false;
      }else if (action == 1){
        getGlobalState().middleMouseDown = true;
      }
    }
  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    onMainUiScroll(amount);
  };
  binding.onObjectAdded = [](int32_t _, void* data, int32_t idAdded) -> void {
    onObjectsChanged();
  };
  binding.onObjectRemoved = [](int32_t _, void* data, int32_t idRemoved) -> void {
    onActivePlayerRemoved(idRemoved);
    onObjectsChanged();
  };

  return binding;
}


std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api){
  std::vector<CScriptBinding> bindings;
  gameapi = &api;
  bindings.push_back(afterworldMainBinding(api, "native/main"));
  bindings.push_back(aiBinding(api, "native/ai"));
  bindings.push_back(movementBinding(api, "native/movement"));
  bindings.push_back(menuBinding(api, "native/menu"));
  bindings.push_back(weaponBinding(api, "native/weapon"));
  bindings.push_back(daynightBinding(api, "native/daynight"));
  bindings.push_back(dialogBinding(api, "native/dialog"));
  bindings.push_back(tagsBinding(api, "native/tags"));
  bindings.push_back(debugBinding(api, "native/debug"));
  bindings.push_back(weatherBinding(api, "native/weather"));
  bindings.push_back(soundBinding(api, "native/sound"));
  bindings.push_back(waterBinding(api, "native/water"));
  bindings.push_back(gametypesBinding(api, "native/gametypes"));
  bindings.push_back(modelviewerBinding(api, "native/modelviewer"));
  bindings.push_back(particleviewerBinding(api, "native/particleviewer"));
  return bindings;
} 

