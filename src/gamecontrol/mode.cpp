#include "./mode.h"

extern GameTypes gametypeSystem;

void inputOverride();

void startMode(GameMode& gameMode, objid sceneId){
  auto& allPlayers = getPlayers();
  for (auto& player : allPlayers){
    setTempCamera(std::nullopt, player.viewport);
  }

  inputOverride();

  auto gamemodeFps = std::get_if<GameModeFps>(&gameMode);
  auto gamemodeBall = std::get_if<GameModeBall>(&gameMode);
  auto gamemodeIntro = std::get_if<GameModeIntro>(&gameMode);
  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
  auto gamemodeVideo = std::get_if<GameModeVideo>(&gameMode);
  if (gamemodeFps){
    changeUiMode(FpsModeUi{});
    startFpsMode(sceneId, gamemodeFps -> player, gamemodeFps -> makePlayer);
  }else if (gamemodeBall){
    changeUiMode(BallModeUi{});
    startBallMode(sceneId);
  }else if (gamemodeIntro){
    changeUiMode(UiModeNone{});
    startIntroMode(sceneId);
  }else if (gamemodeNone){
  	// do nothing
    changeUiMode(UiModeNone{});
  }else if (gamemodeVideo){
    startVideoMode(sceneId);
  }else {
    modassert(false, "startMode invalid game mode");
  }
}

void stopMode(GameMode& gameMode){
  auto gamemodeFps = std::get_if<GameModeFps>(&gameMode);
  if (gamemodeFps){
    stopFpsMode();
  }

  auto gamemodeBall = std::get_if<GameModeBall>(&gameMode);
  if (gamemodeBall){
    endBallMode();
  }

  auto gamemodeIntro = std::get_if<GameModeIntro>(&gameMode);
  if (gamemodeIntro){
    endIntroMode();
  }

  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
  if (gamemodeNone){
  }

  auto gamemodeVideo = std::get_if<GameModeVideo>(&gameMode);
  if (gamemodeVideo){
    // do nothing
  }

  inputOverride();
  changeUiMode(UiModeNone{});
  changeGameTypeNone(gametypeSystem);
  getGlobalState().userRequestedPause = false;

  auto& allPlayers = getPlayers();
  for (auto& player : allPlayers){
    setTempCamera(std::nullopt, player.viewport);
  }
}

