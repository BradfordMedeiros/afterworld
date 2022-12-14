#include "./bindings.h"

CustomApiBindings* gameapi = NULL;

std::vector<std::string> defaultScenes = { "./res/scenes/editor/console.rawscene" };
std::vector<std::string> managedTags = { "game-level" };

struct Level {
  std::string scene;
  std::string name;
};
struct GameState {
  int selectedLevel;
  std::optional<std::string> loadedLevel;
  std::vector<Level> levels;
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
  gameState.loadedLevel = sceneName;
  auto sceneId = gameapi -> loadScene(sceneName, {}, std::nullopt, managedTags);
  auto optCameraId = gameapi -> getGameObjectByName(">maincamera", sceneId, false);
  if (optCameraId.has_value()){
    gameapi ->  setActiveCamera(optCameraId.value(), -1);
  }
}
std::optional<std::string> levelByShortcutName(std::string shortcut){
  auto query = gameapi -> compileSqlQuery("select filepath, shortcut from levels");
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
  if (gameState.loadedLevel.has_value()){
    gameState.loadedLevel = std::nullopt;
    unloadAllManagedScenes();
  }
  gameapi -> loadScene("../afterworld/scenes/menu.rawscene", {}, std::nullopt, managedTags);
}
void handleLevelUp(GameState& gameState){
  if (gameState.loadedLevel.has_value()){
    return;
  }
  auto newIndex = glm::max(0, gameState.selectedLevel - 1);
  gameState.selectedLevel = newIndex;
}
void handleLevelDown(GameState& gameState){
  if (gameState.loadedLevel.has_value()){
    return;
  }
  int lastLevelIndex = gameState.levels.size() - 1;
  auto newIndex = glm::min(lastLevelIndex, gameState.selectedLevel + 1);
  gameState.selectedLevel = newIndex;
}
void handleSelectLevel(GameState& gameState){
  if (gameState.loadedLevel.has_value() || gameState.selectedLevel >= gameState.levels.size()){
    return;
  }
  goToLevel(gameState, gameState.levels.at(gameState.selectedLevel).scene);
}

void drawMenuText(GameState& gameState){
  for (int i = 0; i < gameState.levels.size(); i++){
    auto level = gameState.levels.at(i).name;
    auto levelText = (i == gameState.selectedLevel) ? (std::string("> ") + level) : level;
    gameapi -> drawText(levelText, -0.9, 0.2 + (i * -0.1f), 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
  }
}

void loadConfig(GameState& gameState){
  auto query = gameapi -> compileSqlQuery("select filepath, name from levels");
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

  int maxLevelIndex = levels.size() - 1;
  gameState.selectedLevel = glm::min(gameState.selectedLevel, maxLevelIndex);
}

CScriptBinding afterworldMainBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    GameState* gameState = new GameState;
    gameState -> selectedLevel = 0;
    gameState -> loadedLevel = std::nullopt;
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
    return gameState;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    delete gameState;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    if (!gameState -> loadedLevel.has_value()){
      drawMenuText(*gameState);
    }
  };
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    GameState* gameState = static_cast<GameState*>(data);
    auto hasInputKey = gameapi -> unlock("input", id);
    if (!hasInputKey){
      return;
    }
    if (action == 1){
      if (key == 257){  // enter
        handleSelectLevel(*gameState);
      }else if (key == 259){ // backspace
        goToMenu(*gameState);
      }
      else if (key == 265){ // up
        handleLevelUp(*gameState);
      }else if (key == 264){ // down
        handleLevelDown(*gameState);
      }
    }
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
    GameState* gameState = static_cast<GameState*>(data);
    if (key == "reset"){
      goToMenu(*gameState);
    }
    if (key == "reload-config:levels"){
      loadConfig(*gameState);
    }
  };

  return binding;
}


std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api){
  std::vector<CScriptBinding> bindings;
  gameapi = &api;
  bindings.push_back(afterworldMainBinding(api, "native/main"));
  bindings.push_back(movementBinding(api, "native/movement"));
  bindings.push_back(menuBinding(api, "native/menu"));
  bindings.push_back(menuBinding(api, "native/weapon"));
  return bindings;
} 

