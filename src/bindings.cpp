#include "./bindings.h"

CustomApiBindings* gameapi = NULL;

std::vector<std::string> defaultScenes = { };
std::vector<std::string> managedTags = { "game-level" };

struct GameState {
  std::optional<std::string> loadedLevel;
  std::vector<Level> levels;
  bool menuLoaded;
  std::optional<objid> modelViewerScene;

  std::optional<std::string> dragSelect;
  std::optional<glm::vec2> selecting;


  HandlerFns uiCallbacks;
  UiContext uiContext;
};

void unloadAllManagedScenes(){
  auto managedScenes = gameapi -> listScenes(managedTags);
  for (auto sceneId : managedScenes){
    gameapi -> unloadScene(sceneId);
  }
}
void loadDefaultScenes(){
  for (auto &defaultScene : defaultScenes){
    gameapi -> loadScene(defaultScene, {}, std::nullopt, std::nullopt);
  }
}


void goToLevel(GameState& gameState, std::string sceneName){
  unloadAllManagedScenes();
  gameState.menuLoaded = false;
  setPaused(false);
  gameState.loadedLevel = sceneName;
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
void goToMenu(GameState& gameState){
  exitGameMode();
  if (gameState.loadedLevel.has_value()){
    gameState.loadedLevel = std::nullopt;
    unloadAllManagedScenes();
  }
  if (!gameState.menuLoaded){
    gameapi -> loadScene("../afterworld/scenes/menu.rawscene", {}, std::nullopt, managedTags);
    gameState.menuLoaded = true;
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


void togglePauseMode(GameState& gameState){
  bool paused = isPaused();
  setPausedMode(!paused);
}

std::optional<objid> activeSceneIdOpt(){
  auto selected = gameapi -> selected();
  if (selected.size() == 0){
    return std::nullopt;
  }
  auto selectedId = gameapi -> selected().at(0);
  auto sceneId = gameapi -> listSceneId(selectedId);
  return sceneId;
}

const std::string sceneFolder = "./res/scenes/";


bool queryConsoleCanEnable(){
  auto query = gameapi -> compileSqlQuery("select console from session", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  return result.at(0).at(0) == "true";
}


void queryUpdateShowEditor(bool showEditor){
  auto updateQuery = gameapi -> compileSqlQuery(
    std::string("update session set ") + "editor = " + (showEditor ? "true" : "false"), {}
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
  modassert(validSql, "error executing sql query");
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
        goToLevel(gameState, level.scene);
      },
      .goToMenu = [&gameState]() -> void {
        goToMenu(gameState);
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
        auto sceneId = activeSceneIdOpt();
        modassert(sceneId.has_value(), "save scene - no active scene");
        gameapi -> saveScene(false /*include ids */, sceneId.value(), std::nullopt /* filename */);
      },
    },
    .listScenes = []() -> std::vector<std::string> { return gameapi -> listResources("scenefiles"); },
    .loadScene = [&gameState](std::string scene) -> void {
      std::cout << "load scene placeholder: " << scene << std::endl;
      goToLevel(gameState, scene);
    },
    .newScene = [](std::string sceneName) -> void {
      gameapi -> createScene(sceneFolder + sceneName + ".rawscene");
    },
    .resetScene = []() -> void {
      auto sceneId = activeSceneIdOpt();
      modassert(sceneId.has_value(), "resetScene  - no active scene");
      gameapi -> resetScene(sceneId.value());
    },
    .activeSceneId = activeSceneIdOpt,
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
          goToMenu(gameState);
          return;
        }
        auto scene = levelByShortcutName(level.value());
        if (scene.has_value()){
          goToLevel(gameState, scene.value());
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


void loadConfig(GameState& gameState){
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
  gameState.levels = levels;
}

void handleInteract(objid gameObjId){
  auto objAttr =  gameapi -> getGameObjectAttr(gameObjId);
  auto chatNode = getStrAttr(objAttr, "chatnode");
  if (chatNode.has_value()){
    gameapi -> sendNotifyMessage("dialog:talk", chatNode.value());
  }
  auto triggerSwitch = getStrAttr(objAttr, "trigger-switch");
  if (triggerSwitch.has_value()){
    gameapi -> sendNotifyMessage("switch", triggerSwitch.value());
  }
}

float randomNum(){
  return static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
}

// should work globally but needs lsobj-attr modifications, and probably should create a way to index these
void handleSwitch(std::string switchValue){ 
  //wall:switch:someswitch
  //wall:switch-recording:somerecording
  auto objectsWithSwitch = gameapi -> getObjectsByAttr("switch", switchValue, std::nullopt);

  //std::vector<objid> objectsWithSwitch = {};
  std::cout << "num objects with switch = " << switchValue << ", " << objectsWithSwitch.size() << std::endl;
  for (auto id : objectsWithSwitch){
    std::cout << "handle switch: " << id << std::endl;
    //wall:switch-recording:somerecording
    // supposed to play recording here, setting tint for now to test
    auto objAttr =  gameapi -> getGameObjectAttr(id);
    auto switchRecording = getStrAttr(objAttr, "switch-recording");
    if (switchRecording.has_value()){
      gameapi -> playRecording(id, switchRecording.value(), std::nullopt);
    }
    
    GameobjAttributes attr {
      .stringAttributes = {},
      .numAttributes = {},
      .vecAttr = {
        .vec3 = {},
        .vec4 = {
          { "tint", glm::vec4(randomNum(), randomNum(), randomNum(), 1.f) },
        },
      },
    };
    gameapi -> setGameObjectAttr(id, attr);

  }
}

void handleCollision(objid obj1, objid obj2, std::string attrForValue, std::string attrForKey, std::string removeKey){
  modlog("main collision: ", gameapi -> getGameObjNameForId(obj1).value() + ", " + gameapi -> getGameObjNameForId(obj2).value());
  auto objAttr1 =  gameapi -> getGameObjectAttr(obj1);
  auto switchEnter1 = getAttr(objAttr1, attrForValue);
  auto switchEnter1Key = getStrAttr(objAttr1, attrForKey);
  auto switchRemove1 = getStrAttr(objAttr1, "switch-remove");
  if (switchEnter1.has_value()){
    //std::cout << "race publishing 1: " << switchEnter1.value() << std::endl;
    auto key = switchEnter1Key.has_value() ? switchEnter1Key.value() : "switch";
    std::cout << "handle collision: " << key << ", " << print(switchEnter1.value()) << std::endl;

    gameapi -> sendNotifyMessage(key, switchEnter1.value());
    if (switchRemove1.has_value() && switchRemove1.value() == removeKey){
      gameapi -> removeByGroupId(obj1);
    }
  }

  auto objAttr2 =  gameapi -> getGameObjectAttr(obj2);
  auto switchEnter2 = getAttr(objAttr2, attrForValue);
  auto switchEnter2Key = getStrAttr(objAttr2, attrForKey);
  auto switchRemove2 = getStrAttr(objAttr2, "switch-remove");
  if (switchEnter2.has_value()){
    //std::cout << "race publishing 2: " << switchEnter2.value() << std::endl;
    auto key = switchEnter2Key.has_value() ? switchEnter2Key.value() : "switch";
    std::cout << "handle collision:2 " << key << ", " << print(switchEnter2.value()) << std::endl;
    
    gameapi -> sendNotifyMessage(key, switchEnter2.value());
    if (switchRemove2.has_value() && switchRemove2.value() == removeKey){
      gameapi -> removeByGroupId(obj2);
    }
  }
}
void handleDamageCollision(objid obj1, objid obj2){
  modlog("damage collision: ", gameapi -> getGameObjNameForId(obj1).value() + ", " + gameapi -> getGameObjNameForId(obj2).value());
  
  {
    auto objAttr1 =  gameapi -> getGameObjectAttr(obj1);
    auto damageAmount = getFloatAttr(objAttr1, "touch-damage");
    if (damageAmount.has_value()){
      DamageMessage damageMessage {
        .id = obj2,
        .amount = damageAmount.value(),
      };
      gameapi -> sendNotifyMessage("damage", damageMessage);
      gameapi -> removeByGroupId(obj1);
    }
  }
   
  {
    auto objAttr2 =  gameapi -> getGameObjectAttr(obj2);
    auto damageAmount2 = getFloatAttr(objAttr2, "touch-damage");
    if (damageAmount2.has_value()){
      DamageMessage damageMessage {
        .id = obj1,
        .amount = damageAmount2.value(),
      };
      gameapi -> sendNotifyMessage("damage", damageMessage);
      gameapi -> removeByGroupId(obj2);
    }
  }
}

void selectWithBorder(GameState& gameState, glm::vec2 fromPoint, glm::vec2 toPoint){
  float leftX = fromPoint.x < toPoint.x ? fromPoint.x : toPoint.x;
  float rightX = fromPoint.x > toPoint.x ? fromPoint.x : toPoint.x;

  float topY = fromPoint.y < toPoint.y ? fromPoint.y : toPoint.y;
  float bottomY = fromPoint.y > toPoint.y ? fromPoint.y : toPoint.y;

  float width = rightX - leftX;;
  float height = bottomY - topY;

  //std::cout << "selection: leftX = " << leftX << ", rightX = " << rightX << ", topY = " << topY << ", bottomY = " << bottomY << ", width = " << width << ", height = " << height << std::endl;
  float borderSize = 0.005f;
  float borderWidth = width - borderSize;
  float borderHeight = height - borderSize;

  gameapi -> drawRect(leftX + (width * 0.5f), topY + (height * 0.5f), width, height, false, glm::vec4(0.9f, 0.9f, 0.9f, 0.1f), std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawRect(leftX + (width * 0.5f), topY + (height * 0.5f), borderWidth, borderHeight, false, glm::vec4(0.1f, 0.1f, 0.1f, 0.1f), std::nullopt, true, std::nullopt, std::nullopt);

  // this can be amortized over multiple 
  float uvWidth = toPoint.x - fromPoint.x;
  float uvHeight = toPoint.y - fromPoint.y;

  modassert(false, "select with border needs to use async idatcoordapi");

  std::set<objid> ids;
  /*
  for (int x = 0; x < 50; x++){
    for (int y = 0; y < 50; y++){    
      auto idAtCoord = gameapi -> idAtCoord(fromPoint.x + (x * uvWidth / 50.f), fromPoint.y + (y * uvHeight / 50.f), true);
      if (idAtCoord.has_value()){
        auto selectableValue = getSingleAttr(idAtCoord.value(), "dragselect");
        if (selectableValue.has_value() && selectableValue.value() == gameState.dragSelect.value()){
          ids.insert(idAtCoord.value());
        }
      }
    } 
  }*/

  modlog("dragselect", print(ids));
  gameapi -> setSelected(ids);
}

void raycastAndMoveTo(){
  auto currentTransform = gameapi -> getCameraTransform();
  auto hitpoints = gameapi -> raycast(currentTransform.position, currentTransform.rotation, 100.f);

  if (hitpoints.size() > 0){
    glm::vec3 location = hitpoints.at(0).point;
    setEntityTargetLocation(getActivePlayerId().value(), MovementRequest {
      .position = location,
      .speed = 0.5f,
    });
    showDebugHitmark(hitpoints.at(0), -1);
  }


}

TextData textData {
  .valueText = "default \n textbox",
  .cursorLocation = 2,
  .highlightLength = 0,
  .maxchars = -1,
};


AIInterface aiInterface {
  .move = [](objid agentId, glm::vec3 targetPosition, float speed) -> void {
    setEntityTargetLocation(agentId, MovementRequest {
      .position = targetPosition,
      .speed = speed,
    });
  },
};


void ensureModelViewerLoaded(GameState& gameState, bool loadModelViewer){
  if (!loadModelViewer && gameState.modelViewerScene.has_value()){
    gameapi -> unloadScene(gameState.modelViewerScene.value());
    gameState.modelViewerScene = std::nullopt;
    return;
  }
  if (loadModelViewer && !gameState.modelViewerScene.has_value()){
    unloadAllManagedScenes();
    gameState.menuLoaded = false;
    gameState.modelViewerScene = gameapi -> loadScene("../afterworld/scenes/dev/models.rawscene", {}, std::nullopt, managedTags);
    return;
  }
}


glm::vec4 activeColor(1.f, 0.f, 0.f, 0.5f);
CScriptBinding afterworldMainBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  registerUiSource(textEditorDefault, static_cast<void*>(&textData));
  registerUiSource(color, static_cast<void*>(&activeColor), VEC4);
  
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    GameState* gameState = new GameState;
    gameState -> loadedLevel = std::nullopt;
    gameState -> menuLoaded = false;
    gameState -> modelViewerScene = std::nullopt,
    gameState -> selecting = std::nullopt;
    gameState -> uiCallbacks = HandlerFns {
      .handlerFns = {},
      .handlerFns2 = {},
      .inputFns = {},
    };
    gameState -> uiContext = {};
    initGlobal();
    loadConfig(*gameState);
    loadDefaultScenes();
    goToMenu(*gameState);
    auto args = gameapi -> getArgs();
    if (args.find("level") != args.end()){
      auto scene = levelByShortcutName(args.at("level"));
      if (scene.has_value()){
        goToLevel(*gameState, scene.value());
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
      ensureModelViewerLoaded(*gameState, currentPath == "modelviewer");
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
      selectWithBorder(*gameState, gameState -> selecting.value(), glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc));
    }
    onPlayerFrame();

  };

  binding.onObjectHover = [](objid scriptId, void* data, int32_t index, bool hoverOn) -> void {
    std::cout << "on object hover: " << index << ", hover = " << hoverOn << std::endl;

  };
  binding.onObjectSelected = [](objid scriptId, void* data, int32_t index, glm::vec3 color) -> void {
    std::cout << "on selected: " << index << std::endl;
  };



  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    auto hasInputKey = gameapi -> unlock("input", id);
    if (!hasInputKey){
      return;
    }
    std::cout << "key is: " << key << std::endl;
    if (action == 1){
      if (key == 256 /* escape */ ){
        togglePauseMode(*gameState);
      }
    }

    if (key == 263){        // left  key
      activeColor.a += 0.01f;
    }else if (key == 262){  // right key
      activeColor.a -= 0.01f;
    }


    if (key == 'M' && action == 0){
      spawnFromRandomSpawnpoint("red");
    }else if (key == ',' && action == 0){
      spawnFromAllSpawnpoints("red");
    }else if (key == '.' && action == 0){
      spawnFromAllSpawnpoints("blue");
    }else if (key == '/' && action == 0){
      removeAllSpawnedEntities();
    }else if (key == '-' && action == 0){
      setActivePlayerNext();
    }

    if (action == 1){
      onMainUiKeyPress(gameState -> uiCallbacks, key, scancode, action, mods);
    }

  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    GameState* gameState = static_cast<GameState*>(data);
    if (key == "reset"){
      goToMenu(*gameState);
      return;
    }
    if (key == "game-over"){
      goToMenu(*gameState);
      return;
    }
    if (key == "reload-config:levels"){
      loadConfig(*gameState);
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
      setAmmoCount(AmmoHudInfo {
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
  };
  binding.onCollisionExit = [](objid id, void* data, int32_t obj1, int32_t obj2) -> void {
    auto gameobj1Exists = gameapi -> gameobjExists(obj1);
    auto gameobj2Exists = gameapi -> gameobjExists(obj2);
    modassert(gameobj1Exists && gameobj2Exists, "collision exit: objs do not exist");
    handleCollision(obj1, obj2, "switch-exit", "switch-exit-key", "exit");
  };

  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    //std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    std::cout << "(xNdc, yNdc)" << xNdc << ", " << yNdc << std::endl;
    GameState* gameState = static_cast<GameState*>(data);
    getGlobalState().xNdc = xNdc;
    getGlobalState().yNdc = yNdc;
  };

  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    onMainUiMousePress(gameState -> uiCallbacks, button, action, getGlobalState().selectedId);

    if (button == 1){
      if (action == 0){
        gameState -> selecting = std::nullopt;
      }else if (action == 1){
        gameState -> selecting = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
        raycastAndMoveTo();
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
  bindings.push_back(vehicleBinding(api, "native/vehicle"));
  bindings.push_back(menuBinding(api, "native/menu"));
  bindings.push_back(weaponBinding(api, "native/weapon"));
  bindings.push_back(inventoryBinding(api, "native/inventory"));
  bindings.push_back(daynightBinding(api, "native/daynight"));
  bindings.push_back(dialogBinding(api, "native/dialog"));
  bindings.push_back(tagsBinding(api, "native/tags"));
  bindings.push_back(hotkeysBinding(api, "native/debug"));
  bindings.push_back(weatherBinding(api, "native/weather"));
  bindings.push_back(soundBinding(api, "native/sound"));
  bindings.push_back(waterBinding(api, "native/water"));
  bindings.push_back(gametypesBinding(api, "native/gametypes"));
  return bindings;
} 

