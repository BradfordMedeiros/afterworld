#include "./playing.h"

extern UiMode uiMode;

std::optional<TerminalConfig> terminal;

std::optional<BallModeUi*> getBallModeUI(){
  auto uiModeBall = std::get_if<BallModeUi>(&uiMode);
  if (uiModeBall == NULL){
    return std::nullopt;
  }
  return uiModeBall;
}

std::optional<LiveMenu*> getLiveMenuUi(){
  auto liveMenu = std::get_if<LiveMenu>(&uiMode);
  if (liveMenu == NULL){
    return std::nullopt;
  }
  return liveMenu;
}

void setTerminalConfig(std::optional<TerminalConfig> terminalConfig){
  terminal = terminalConfig;
}
std::optional<TerminalConfig*> getTerminalConfig(){
  if (!terminal.has_value()){
    return std::nullopt;
  }  
  return &terminal.value();
}

Component playingComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto uiModeNone = std::get_if<UiModeNone>(&uiMode);
    auto uiModeFps = std::get_if<FpsModeUi>(& uiMode);
    auto uiModeBall = std::get_if<BallModeUi>(&uiMode);
    auto uiModeLiveMenu = std::get_if<LiveMenu>(&uiMode);
    auto uiModeGameOver = std::get_if<GameOverUi>(&uiMode);

    /*if (uiModeNone){
      drawCenteredText(drawTools, "none", 0.f, 0.f, 0.2f, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt);
    }else if (uiModeFps){
      drawCenteredText(drawTools, "fps", 0.f, 0.f, 0.2f, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt);
    }else if (uiModeBall){
      drawCenteredText(drawTools, "ball", 0.f, 0.f, 0.2f, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt);
    }else if (uiModeLiveMenu){
      drawCenteredText(drawTools, "livemenu", 0.f, 0.f, 0.2f, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt);
    }else if (uiModeGameOver){
      drawCenteredText(drawTools, "gameover", 0.f, 0.f, 0.2f, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt);
    }*/


    if (terminal.has_value()){
      Props terminalProps { 
        .props = { PropPair { .symbol = valueSymbol, .value = terminal.value() }},
      };
      terminalComponent.draw(drawTools, terminalProps);    
    }

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};
