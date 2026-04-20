#include "./fps.h"

extern CustomApiBindings* gameapi;

extern Director director;
extern Movement movement;
extern Weapons weapons;
extern AiData aiData;

void startFpsMode(objid sceneId, std::optional<std::string> player, bool makePlayer){
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
    setActivePlayer(movement, weapons, aiData, playerId.value(), 0);
  }

  if (playerIds.size() > 0){
    for (int i = 0; i < playerIds.size(); i++){
      auto prefabId = playerIds.at(i);
      auto playerId = findBodyPart(prefabId, player.value().c_str());
      modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
      modlog("router", std::string("setting active player: playerId id = ") + std::to_string(playerId.value()));
      setActivePlayer(movement, weapons, aiData, playerId, i);
    }
  }
}

void stopFpsMode(){

}