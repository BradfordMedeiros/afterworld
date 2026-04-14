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
    startFpsMode(sceneId, gamemodeFps -> player, gamemodeFps -> makePlayer);
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
  auto gamemodeFps = std::get_if<GameModeFps>(&gameMode);
  if (gamemodeFps){
    stopFpsMode();
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
  
  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
  if (gamemodeNone){
  	return;
  }

  modassert(false, "invalid game mode");
}

