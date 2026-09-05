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
    if (terminal.has_value()){
      Props terminalProps { 
        .props = { PropPair { .symbol = valueSymbol, .value = terminal.value() }},
      };
      terminalComponent.draw(drawTools, terminalProps);    
    }

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};
