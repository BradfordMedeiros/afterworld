#include "./mode.h"

extern Director director;
extern Movement movement;
extern Weapons weapons;
extern AiData aiData;

void startMode(GameMode& gameMode, objid sceneId){
  auto gamemodeFps = std::get_if<GameModeFps>(&gameMode);
  auto gamemodeBall = std::get_if<GameModeBall>(&gameMode);
  auto gamemodeIntro = std::get_if<GameModeIntro>(&gameMode);
  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
  if (gamemodeFps){
	  std::vector<objid> playerIds;

    if (gamemodeFps -> makePlayer){
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
    auto prefabId = createPrefab(sceneId, "../afterworld/scenes/prefabs/enemy/player-cheap.rawscene",  position, {});    

    auto playerId = findObjByShortName("maincamera", sceneId);
    modassert(playerId.has_value(), "onSceneRouteChange, no playerId in scene to load");
    setActivePlayer(movement, weapons, aiData, playerId.value(), 0);

    startBallMode(sceneId);
  }else if (gamemodeIntro){
    startIntroMode(sceneId);
  }else if (gamemodeNone){
  	// do nothing
  }
}

void stopMode(GameMode& gameMode){
  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
  if (gamemodeNone){
  	return;
  }

  auto gamemodeFps = std::get_if<GameModeFps>(&gameMode);
  if (gamemodeFps){
  	return;
  }

  auto gamemodeBall = std::get_if<GameModeBall>(&gameMode);
  if (gamemodeBall){
    endBallMode();
  	return;
  }

  auto gamemodeIntro = std::get_if<GameModeIntro>(&gameMode);
  if (gamemodeIntro){
    endIntroMode();
  	return;
  }

  modassert(false, "invalid game mode");
}

