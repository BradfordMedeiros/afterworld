#include "./utilview.h"

Component utilViewComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    UtilViewOptions* utilViewOptions = typeFromProps<UtilViewOptions>(props, valueSymbol);

    if (utilViewOptions -> showConsole){
      Props props {
        .props = {
          { .symbol = consoleInterfaceSymbol, .value = utilViewOptions -> consoleInterface },
          { .symbol = autofocusSymbol, .value = utilViewOptions -> consoleKeyName  },
        },
      };
      consoleComponent.draw(drawTools, props);
    }

    if (utilViewOptions -> showKeyboard){
      Props keyboardProps { 
        .props = {
          PropPair { .symbol = xoffsetSymbol, .value = -1.f },
          PropPair { .symbol = yoffsetSymbol, .value = -1.f },
        },
      };
      keyboardComponent.draw(drawTools, keyboardProps);     
    }
    
    {
      Props defaultProps {
        .props = {},
      };
      alertComponent.draw(drawTools, defaultProps);
    }

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};
