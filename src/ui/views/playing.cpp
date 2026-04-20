#include "./playing.h"

UiMode uiMode = UiModeNone{};

void changeUiMode(UiMode newUiMode){
  uiMode = newUiMode;
}

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

Component playingComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    PlayingOptions* playingOptions = typeFromProps<PlayingOptions>(props, valueSymbol);
    modassert(playingOptions, "playing options not defined");

    auto uiModeNone = std::get_if<UiModeNone>(&uiMode);
    auto uiModeFps = std::get_if<FpsModeUi>(& uiMode);
    auto uiModeBall = std::get_if<BallModeUi>(&uiMode);
    auto uiModeLiveMenu = std::get_if<LiveMenu>(&uiMode);

    if (uiModeNone){
      drawCenteredText(drawTools, "none", 0.f, 0.f, 0.2f, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt);
    }else if (uiModeFps){
      drawCenteredText(drawTools, "fps", 0.f, 0.f, 0.2f, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt);
    }else if (uiModeBall){
      drawCenteredText(drawTools, "ball", 0.f, 0.f, 0.2f, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt);
    }else if (uiModeLiveMenu){
      drawCenteredText(drawTools, "livemenu", 0.f, 0.f, 0.2f, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt);
    }

    std::cout << "show pause: " << playingOptions -> showPause << std::endl;
    if (playingOptions -> showPause){
      auto pauseComponent = withPropsCopy(pauseMenuComponent, pauseMenuProps(std::nullopt));
      auto defaultProps = getDefaultProps();
      return pauseComponent.draw(drawTools, defaultProps);     
    }

    if (playingOptions -> showGameOver){
      auto deadComponent = withPropsCopy(pauseMenuComponent, deadMenuProps(std::nullopt));
      auto defaultProps = getDefaultProps();
      return deadComponent.draw(drawTools, defaultProps);    
    }

    if (playingOptions -> terminalConfig.has_value()){
      Props terminalProps { 
        .props = { PropPair { .symbol = valueSymbol, .value = playingOptions -> terminalConfig.value() }},
      };
      terminalComponent.draw(drawTools, terminalProps);    
    }


    if (uiModeBall){
      Props ballProps { 
        .props = {
          PropPair { .symbol = valueSymbol, .value = uiModeBall -> ballMode },
        },
      };
      ballComponent.draw(drawTools, ballProps);    
      return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
    }else if (uiModeLiveMenu){
      Props defaultProps { 
        .props = {
          PropPair { .symbol = valueSymbol, .value = uiModeLiveMenu -> options }
        }
      };
      mainMenu2.draw(drawTools, defaultProps);      
      return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
    }


    if (playingOptions -> showHud){
  	  auto hudProps = getDefaultProps();
	    hudComponent.draw(drawTools, hudProps);
    }


  	if (playingOptions -> showZoomOverlay.has_value()){
  	  Props zoomProps { 
  	    .props = {
  	      PropPair { .symbol = valueSymbol, .value = playingOptions -> showZoomOverlay.value() },
  	    },
  	  };
  	  zoomComponent.draw(drawTools, zoomProps);    
  	}


	  if (playingOptions -> scoreOptions.has_value()){
	    float scorePadding = 0.02f;
    	Props scoreProps {
    	  .props = {
    	    PropPair { .symbol = valueSymbol, .value = playingOptions -> scoreOptions.value() },
    	    PropPair { .symbol = xoffsetSymbol, .value = 1.f - scorePadding },
    	    PropPair { .symbol = yoffsetSymbol, .value = -1.f + scorePadding },
    	  },
    	};
 	    scoreComponent.draw(drawTools, scoreProps);
	  }

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};
