#include "./bindings.h"

CustomApiBindings* gameapi = NULL;


std::vector<std::string> defaultScenes = {
  "./res/scenes/editor/console.rawscene",
};

std::vector<std::string> levels = {
  "./res/scenes/example.p.rawscene",
  "./res/scenes/features/physics/collisiontypes.p.rawscene",
  "./res/scenes/features/lighting/tint.p.rawscene",
};

struct GameState {
  int selectedLevel;
  std::optional<std::string> loadedLevel;
};
GameState gameState {
  .selectedLevel = 0,
  .loadedLevel = std::nullopt,
};

std::vector<std::string> managedTags = { "game-level" };
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

void goToLevel(std::string sceneName){
  unloadAllManagedScenes();
  gameState.loadedLevel = sceneName;
  auto sceneId = gameapi -> loadScene(sceneName, {}, std::nullopt, managedTags);
  auto optCameraId = gameapi -> getGameObjectByName(">maincamera", sceneId, false);
  if (optCameraId.has_value()){
    gameapi ->  setActiveCamera(optCameraId.value(), -1);
  }
}
void goToMenu(){
  if (gameState.loadedLevel.has_value()){
    gameState.loadedLevel = std::nullopt;
    unloadAllManagedScenes();
  }
  gameapi -> loadScene("../afterworld/scenes/menu.rawscene", {}, std::nullopt, managedTags);
}
void handleLevelUp(){
  if (gameState.loadedLevel.has_value()){
    return;
  }
  auto newIndex = glm::max(0, gameState.selectedLevel - 1);
  gameState.selectedLevel = newIndex;
}
void handleLevelDown(){
  if (gameState.loadedLevel.has_value()){
    return;
  }
  int lastLevelIndex = levels.size() - 1;
  auto newIndex = glm::min(lastLevelIndex, gameState.selectedLevel + 1);
  gameState.selectedLevel = newIndex;
}
void handleSelectLevel(){
  if (gameState.loadedLevel.has_value()){
    return;
  }
  goToLevel(levels.at(gameState.selectedLevel));
}

void drawMenuText(){
  for (int i = 0; i < levels.size(); i++){
    auto level = levels.at(i);
    auto levelText = (i == gameState.selectedLevel) ? (std::string("> ") + level) : level;
    gameapi -> drawText(levelText, -0.9, 0.2 + (i * -0.1f), 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
  }
}

CScriptBinding afterworldMainBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    loadDefaultScenes();
    goToMenu();
    return NULL;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    if (!gameState.loadedLevel.has_value()){
      drawMenuText();
    }
  };
  binding.onKeyCallback = [](int32_t id, int key, int scancode, int action, int mods) -> void {
    auto hasInputKey = gameapi -> unlock("input", id);
    if (!hasInputKey){
      return;
    }
    if (action == 1){
      if (key == 257){  // enter
        handleSelectLevel();
      }else if (key == 259){ // backspace
        goToMenu();
      }
      else if (key == 265){ // up
        handleLevelUp();
      }else if (key == 264){ // down
        handleLevelDown();
      }
    }
  };
  binding.onMessage = [](int32_t id, std::string& key, AttributeValue& value){
    if (key == "reset"){
      goToMenu();
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
  return bindings;
} 










