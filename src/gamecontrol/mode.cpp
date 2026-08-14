#include "./mode.h"

extern GameTypes gametypeSystem;

void inputOverride();
void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn);

std::string modeTypeString(GameMode& gameMode){
  auto gamemodeFps = std::get_if<GameModeFps>(&gameMode);
  if (gamemodeFps){
    return "fps";
  }
  auto gamemodeBall = std::get_if<GameModeBall>(&gameMode);
  if (gamemodeBall){
    return "ball";
  }
  auto gamemodeVideo = std::get_if<GameModeVideo>(&gameMode);
  if (gamemodeVideo){
    return "video";
  }

  auto gamemodeBoot = std::get_if<GameModeBoot>(&gameMode);
  if (gamemodeBoot){
    return "boot";
  }

  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
  if (gamemodeNone){
    return "none";
  }
  return "[unknown]";
}


void startMode(GameMode& gameMode, objid sceneId){
  std::cout << "mode: start: " << modeTypeString(gameMode) << std::endl;

  auto& allPlayers = getPlayers();
  for (auto& player : allPlayers){
    setTempCamera(std::nullopt, player.viewport);
  }

  inputOverride();
  setCanExitVehicle(true);

  auto gamemodeFps = std::get_if<GameModeFps>(&gameMode);
  auto gamemodeBall = std::get_if<GameModeBall>(&gameMode);
  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
  auto gamemodeVideo = std::get_if<GameModeVideo>(&gameMode);
  auto gamemodeBoot = std::get_if<GameModeBoot>(&gameMode);

  if (gamemodeFps){
    changeUiMode(FpsModeUi{});
    startFpsMode(sceneId, gamemodeFps -> player, gamemodeFps -> makePlayer);
  }else if (gamemodeBall){
    startBallMode(sceneId);
  }else if (gamemodeNone){
  	// do nothing
    changeUiMode(UiModeNone{});
  }else if (gamemodeVideo){
    startVideoMode(sceneId);
  }else if (gamemodeBoot){
    startBootMode(sceneId);
  }else {
    modassert(false, "startMode invalid game mode");
  }
}

void stopMode(GameMode& gameMode){
  std::cout << "mode: stop" << std::endl;

  auto gamemodeFps = std::get_if<GameModeFps>(&gameMode);
  if (gamemodeFps){
    stopFpsMode();
  }

  auto gamemodeBall = std::get_if<GameModeBall>(&gameMode);
  if (gamemodeBall){
    endBallMode();
  }

  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
  if (gamemodeNone){
  }

  auto gamemodeVideo = std::get_if<GameModeVideo>(&gameMode);
  if (gamemodeVideo){
    // do nothing
  }

  auto gamemodeBoot = std::get_if<GameModeBoot>(&gameMode);
  if (gamemodeBoot){
     stopBootMode();
  }

  inputOverride();
  setPauseMenuOverride(std::nullopt);
  changeUiMode(UiModeNone{});
  changeGameTypeNone(gametypeSystem);
  getGlobalState().userRequestedPause = false;

  auto& allPlayers = getPlayers();
  for (auto& player : allPlayers){
    setTempCamera(std::nullopt, player.viewport);
  }
}

bool isModeNone(GameMode& gameMode){
  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
  return gamemodeNone != NULL;
}

void onModeCollision(GameMode& gameMode, objid obj1, objid obj2){
  auto gamemodeBall = std::get_if<GameModeBall>(&gameMode);
  if (gamemodeBall){
    handleBallModeCollision(obj1, obj2);
  }
}