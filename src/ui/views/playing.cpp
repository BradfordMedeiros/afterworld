#include "./playing.h"

Component playingComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    PlayingOptions* playingOptions = typeFromProps<PlayingOptions>(props, valueSymbol);
    modassert(playingOptions, "playing options not defined");

    auto ballOptions = playingOptions -> ballMode;
    if (ballOptions.has_value()){
  	  Props ballProps { 
  	    .props = {
  	      PropPair { .symbol = valueSymbol, .value = playingOptions -> ballMode.value() },
  	    },
  	  };
  	  ballComponent.draw(drawTools, ballProps);    
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

	  if (playingOptions -> terminalConfig.has_value()){
    	Props terminalProps { 
    	  .props = { PropPair { .symbol = valueSymbol, .value = playingOptions -> terminalConfig.value() }},
    	};
	    terminalComponent.draw(drawTools, terminalProps);    
	  }

    if (playingOptions -> menuOptions.has_value()){
      Props defaultProps { 
        .props = {
          PropPair { .symbol = valueSymbol, .value = playingOptions -> menuOptions.value() }
        }
      };
      mainMenu2.draw(drawTools, defaultProps);      
    }

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};
