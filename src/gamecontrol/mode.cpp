#include "./mode.h"

void inputOverride();

void startMode(GameMode& gameMode, objid sceneId){
  inputOverride();

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

  inputOverride();
  changeUiMode(UiModeNone{});
  getGlobalState().userRequestedPause = false;

}

