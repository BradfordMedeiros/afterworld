#include "./playing.h"

Component playingComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    PlayingOptions* playingOptions = typeFromProps<PlayingOptions>(props, valueSymbol);
    modassert(playingOptions, "playing options not defined");

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


    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};
