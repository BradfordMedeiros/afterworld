#include "./mode.h"

void startMode(GameMode& gameMode, objid sceneId){
  auto gamemodeFps = std::get_if<GameModeFps>(&gameMode);
  auto gamemodeBall = std::get_if<GameModeBall>(&gameMode);
  auto gamemodeIntro = std::get_if<GameModeIntro>(&gameMode);
  auto gamemodeNone = std::get_if<GameModeNone>(&gameMode);
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
  }
}

void stopMode(GameMode& gameMode){
  changeUiMode(UiModeNone{});

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

