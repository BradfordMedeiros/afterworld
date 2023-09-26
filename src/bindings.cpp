#include "./bindings.h"

CustomApiBindings* gameapi = NULL;

std::vector<std::string> defaultScenes = { };
std::vector<std::string> managedTags = { "game-level" };

struct ManagedCamera {
  std::optional<objid> defaultCamera;
  objid id;
};
std::stack<ManagedCamera> cameras = {};


struct GameState {
  std::optional<std::string> loadedLevel;
  std::vector<Level> levels;
  bool menuLoaded;

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
  auto optCameraId = gameapi -> getGameObjectByName(">maincamera", sceneId, false);
  if (optCameraId.has_value()){
    cameras = {};
    gameapi -> sendNotifyMessage("request:change-control", optCameraId.value());
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
void goToMenu(GameState& gameState){
  exitGameMode();
  if (gameState.loadedLevel.has_value()){
    gameState.loadedLevel = std::nullopt;
    unloadAllManagedScenes();
  }
  if (!gameState.menuLoaded){
    gameapi -> loadScene(
      "../afterworld/scenes/menu.rawscene", {}, std::nullopt, managedTags);
    gameState.menuLoaded = true;
  }
  cameras = {};
}

double downTime = 0;
void setPausedMode(bool shouldBePaused){
  setPaused(shouldBePaused);
  if (!shouldBePaused){
    if (getCurrentPath() == "paused"){
      pushHistory("playing");
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
   .showScreenspaceGrid = []() -> bool { return getGlobalState().showScreenspaceGrid; },
   .levels = LevelUIInterface {
      .goToLevel = [&gameState](Level& level) -> void {
        goToLevel(gameState, level.scene);
        enterGameMode();
        pushHistory("playing");
      },
      .getLevels = [&gameState]() -> std::vector<Level> {
        return gameState.levels; 
      },
      .goToMenu = [&gameState]() -> void {
        goToMenu(gameState);
        pushHistory("mainmenu");
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
        std::cout << "save scene placeholder" << std::endl;
      }
    },
    .listScenes = []() -> std::vector<std::string> { return gameapi -> listResources("scenefiles"); },
    .loadScene = [&gameState](std::string scene) -> void {
      std::cout << "load scene placeholder: " << scene << std::endl;
      goToLevel(gameState, scene);
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

void handleCollision(objid obj1, objid obj2, std::string attrForValue, std::string attrForKey){
  auto objAttr1 =  gameapi -> getGameObjectAttr(obj1);
  auto switchEnter1 = getAttr(objAttr1, attrForValue);
  auto switchEnter1Key = getStrAttr(objAttr1, attrForKey);
  if (switchEnter1.has_value()){
    //std::cout << "race publishing 1: " << switchEnter1.value() << std::endl;
    gameapi -> sendNotifyMessage(switchEnter1Key.has_value() ? switchEnter1Key.value() : "switch", switchEnter1.value());
  }

  auto objAttr2 =  gameapi -> getGameObjectAttr(obj2);
  auto switchEnter2 = getAttr(objAttr2, attrForValue);
  auto switchEnter2Key = getStrAttr(objAttr2, attrForKey);
  if (switchEnter2.has_value()){
    //std::cout << "race publishing 2: " << switchEnter2.value() << std::endl;
    gameapi -> sendNotifyMessage(switchEnter2Key.has_value() ? switchEnter2Key.value() : "switch", switchEnter2.value());
  }
}

void selectWithBorder(GameState& gameState, glm::vec2 fromPoint, glm::vec2 toPoint, objid id){
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

  std::set<objid> ids;
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
  }
  gameapi -> setSelected(ids);
}

TextData textData {
  .valueText = "default \n textbox",
  .cursorLocation = 2,
  .highlightLength = 0,
  .maxchars = -1,
};

void deleteCharacter(TextData& textData){
  bool deleteSelection = textData.highlightLength > 0;
  auto index = deleteSelection ? (textData.cursorLocation + 1) : textData.cursorLocation;
  if (index == 0){
    return;
  }
  auto prefix = textData.valueText.substr(0, index -1);
  auto suffixIndex = index;
  if (deleteSelection){
    suffixIndex = suffixIndex + textData.highlightLength -1;
  }
  if (suffixIndex >= textData.valueText.size()){
    suffixIndex = textData.valueText.size();
  }
  auto suffix = textData.valueText.substr(suffixIndex, textData.valueText.size());
  auto newString = prefix + suffix;
  textData.valueText = newString;

  if (!deleteSelection){
    textData.cursorLocation--;
  }
  if (textData.cursorLocation < 0){
    textData.cursorLocation = 0;
  }
  textData.highlightLength = 0;
}
void insertCharacter(TextData& textData, char character){
  if (textData.highlightLength > 0){
    deleteCharacter(textData);
  }
  auto newString = insertString(textData.valueText, textData.cursorLocation, character);
  textData.valueText = newString;
  textData.cursorLocation++;
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
    gameState -> selecting = std::nullopt;
    gameState -> uiCallbacks = HandlerFns {
      .handlerFns = {},
      .handlerFns2 = {},
    };
    gameState -> uiContext = {};
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
    return gameState;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    delete gameState;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    auto selectedId = gameapi -> idAtCoord(getGlobalState().xNdc, getGlobalState().yNdc, false);
    getGlobalState().selectedId = selectedId;

    gameState -> uiCallbacks = handleDrawMainUi(gameState -> uiContext, selectedId);

    if (gameState -> dragSelect.has_value() && gameState -> selecting.has_value()){
      selectWithBorder(*gameState, gameState -> selecting.value(), glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc), id);
    }
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

    if (action != 1){
      return;
    }

    if (key == 265){
      // up key
      textData.highlightLength++;
      return;
    }
    if (key == 264){
      // downkey
      textData.highlightLength--;
      if (textData.highlightLength < 0){
        textData.highlightLength = 0;
      }
      return;
    }
    if (key == 263){        // left  key
      textData.cursorLocation--;
      if (textData.cursorLocation < 0){
        textData.cursorLocation = 0;
      }
      activeColor.a += 0.01f;
      return;
    }else if (key == 262){  // right key
      textData.cursorLocation++;
      if (textData.cursorLocation > textData.valueText.size()){
        textData.cursorLocation = textData.valueText.size();
      }
      activeColor.a -= 0.01f;
      return;
    }
    if (key == 257){
      insertCharacter(textData, '\n');
    }else if (key == 261){
      // delete forward
    }else if (key == 259){
      deleteCharacter(textData);
    }else{
      insertCharacter(textData, key);
    }

  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    GameState* gameState = static_cast<GameState*>(data);
    if (key == "reset"){
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

    if (key == "request:change-control"){
      auto gameObjId = anycast<objid>(value); 
      modassert(gameObjId != NULL, "bindings - request change control gameobjid null");
      if (cameras.size() != 0 && *gameObjId == cameras.top().id){
        std::cout << "already on camera: " << *gameObjId << " is " << cameras.top().id << std::endl;
        return;
      }
      auto gameobjName = gameapi -> getGameObjNameForId(*gameObjId);
      if (!gameobjName.has_value()){
        modassert(false, "change control invalid");
        return;
      }

      if (cameras.size() > 0){
        auto managedCamId = cameras.top().defaultCamera;
        if (managedCamId.has_value()){
          gameapi -> removeObjectById(managedCamId.value());
        }
        cameras.pop();
      }
      

      if (gameobjName.value().at(0) != '>'){
        //modassert(false, std::string("is not a camera: ") + gameobjName.value());
        GameobjAttributes attr {
          .stringAttributes = {},
          .numAttributes = {},
          .vecAttr = { .vec3 = { { "position", glm::vec3(0.f, 0.5f, 3.f) }}, .vec4 = {} },
        };
        std::map<std::string, GameobjAttributes> submodelAttributes;
        auto cameraId = gameapi -> makeObjectAttr(gameapi -> listSceneId(*gameObjId), std::string(">") + uniqueNameSuffix(), attr, submodelAttributes);
        gameapi ->  setActiveCamera(cameraId.value(), -1);
        cameras.push(ManagedCamera {
          .defaultCamera = cameraId,
          .id = *gameObjId,
        });
        gameapi -> makeParent(cameraId.value(), *gameObjId);
      }else{
        std::cout << "request change camera: " << gameobjName.value() << std::endl;
        gameapi ->  setActiveCamera(*gameObjId, -1);
        cameras.push(ManagedCamera {
          .defaultCamera = std::nullopt,
          .id = *gameObjId,
        });
      }

      std::cout << "num cameras: " << cameras.size() << std::endl;

      //auto objAttr =  gameapi -> getGameObjectAttr(gameObjId);
      //auto pickup = getStrAttr(objAttr, "pickup");
      return;
    }

    if (key == "request:release-control"){
      auto gameObjId = anycast<objid>(value); 
      modassert(gameObjId != NULL, "request:release-control");
      auto gameobjName = gameapi -> getGameObjNameForId(*gameObjId);
      if (!gameobjName.has_value()){
        return;
      }
      if (cameras.size() != 0 && cameras.top().id == *gameObjId){
        auto managedCamId = cameras.top().defaultCamera;
        if (managedCamId.has_value()){
          gameapi -> removeObjectById(managedCamId.value());
        }
        cameras.pop();
      }

      if (cameras.size() > 0){
        auto previousCamera = cameras.top().id;
        gameapi -> sendNotifyMessage("request:push-control", previousCamera);
      }
    }

    if (key == "alert"){
      auto strValue = anycast<std::string>(value); 
      modassert(strValue != NULL, "alert value invalid");
      sendUiAlert(*strValue);
    }
  };

  binding.onCollisionEnter = [](objid id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal) -> void {
    auto gameobj1 = gameapi -> getGameObjNameForId(obj1); // this check shouldn't be necessary, is bug
    auto gameobj2 = gameapi -> getGameObjNameForId(obj2);
    modassert(gameobj1.has_value() && gameobj2.has_value(), "collision enter: objs do not exist");
    handleCollision(obj1, obj2, "switch-enter", "switch-enter-key");
  };
  binding.onCollisionExit = [](objid id, void* data, int32_t obj1, int32_t obj2) -> void {
    auto gameobj1 = gameapi -> getGameObjNameForId(obj1);
    auto gameobj2 = gameapi -> getGameObjNameForId(obj2);
    modassert(gameobj1.has_value() && gameobj2.has_value(), "collision exit: objs do not exist");
    handleCollision(obj1, obj2, "switch-exit", "switch-exit-key");
  };

  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    //std::cout << "mouse move: xPos = " << xPos << ", yPos = " << yPos << std::endl;
    GameState* gameState = static_cast<GameState*>(data);
    getGlobalState().xNdc = xNdc;
    getGlobalState().yNdc = yNdc;

    //auto selectedId = gameapi -> idAtCoord(gameState -> xNdc, gameState -> yNdc, false);
    //if (selectedId.has_value()){
    //  std::cout << "hovered id is: " << selectedId.value() << std::endl;
    //}
  };

  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    auto idAtCoord = gameapi -> idAtCoord(getGlobalState().xNdc, getGlobalState().yNdc, false);
    onMainUiMousePress(gameState -> uiCallbacks, button, action, idAtCoord);

    if (button == 1){
      if (action == 0){
        gameState -> selecting = std::nullopt;
      }else if (action == 1){
        gameState -> selecting = glm::vec2(getGlobalState().xNdc, getGlobalState().yNdc);
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
  bindings.push_back(hudBinding(api, "native/hud"));
  bindings.push_back(daynightBinding(api, "native/daynight"));
  bindings.push_back(dialogBinding(api, "native/dialog"));
  bindings.push_back(tagsBinding(api, "native/tags"));
  bindings.push_back(debugBinding(api, "native/debug"));
  bindings.push_back(weatherBinding(api, "native/weather"));
  bindings.push_back(inGameUiBinding(api, "native/in-game-ui"));
  bindings.push_back(soundBinding(api, "native/sound"));
  bindings.push_back(waterBinding(api, "native/water"));
  bindings.push_back(gametypesBinding(api, "native/gametypes"));
  return bindings;
} 

