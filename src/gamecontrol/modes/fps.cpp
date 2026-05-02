#include "./fps.h"

extern CustomApiBindings* gameapi;

extern Director director;
extern GameTypes gametypeSystem;

//    .showGameHud = []() -> bool { return getGlobalState().showGameHud && !isPlayerControlDisabled(getDefaultPlayerIndex()); },
void inputOverride(bool paused, bool showMouse);

GameTypeInfo getFpsMode(){
  GameTypeInfo ballMode = GameTypeInfo {
    .gametypeName = "fps",
    .createGametype = [](void* data) -> std::any {
      return NULL; 
    },
    .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> void {
    },
    .onKey = [](std::any& gametype, int key, int scancode, int action, int mods) -> void {},
    .onFrame = [](std::any& gametype) -> void {
      if (allPlayersDead()){
          inputOverride(false, true);
          changeUiMode(GameOverUi{});
      }
    },
  };
  return ballMode;
}



void startFpsMode(objid sceneId, std::optional<std::string> player, bool makePlayer){
  GameTypeInfo fpsMode = getFpsMode();
  changeGameType(gametypeSystem, fpsMode, "fps", NULL);

  std::vector<objid> playerIds;

  if (makePlayer){
    auto playerLocationObj = gameapi -> getObjectsByAttr("playerspawn", std::nullopt, std::nullopt).at(0);
    glm::vec3 position = gameapi -> getGameObjectPos(playerLocationObj, true, "[gamelogic] startLevel get player spawnpoint");
    int numberOfPlayers = getNumberOfPlayers();
    for (int i  = 0; i < numberOfPlayers; i++){
      auto initialPosition = position + glm::vec3(1.f * i, 0.f, 0.f); // TODO - this is bad, this should just have a few initial spawnpoints
      auto playerId = createPrefab(sceneId, "../afterworld/scenes/prefabs/enemy/player.rawscene",  initialPosition, {});    
      playerIds.push_back(playerId);
    }
  }

  spawnFromAllSpawnpoints(director.managedSpawnpoints, "onload");

  if (playerIds.size() == 0 && player.has_value()){
    // this doesn't work...it's looking for the player both times and order is indeterminant so sometimes it will fail setting the active player
    auto playerId = findObjByShortName(player.value(), sceneId);
    modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
    modlog("router", std::string("setting active player: playerId id = ") + std::to_string(playerId.value()));
    setEntityForPlayerIndex(playerId.value(), 0);
  }

  if (playerIds.size() > 0){
    for (int i = 0; i < playerIds.size(); i++){
      auto prefabId = playerIds.at(i);
      auto playerId = findBodyPart(prefabId, player.value().c_str());
      modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
      modlog("router", std::string("setting active player: playerId id = ") + std::to_string(playerId.value()));
      setEntityForPlayerIndex(playerId, i);
    }
  }
}

void stopFpsMode(){

}