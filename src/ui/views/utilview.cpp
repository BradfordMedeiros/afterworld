#include "./utilview.h"


Component utilViewComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    UtilViewOptions* utilViewOptions = typeFromProps<UtilViewOptions>(props, valueSymbol);

    if (utilViewOptions -> showScreenspaceGrid){
      drawScreenspaceGrid(ImGrid{ .numCells = 10 });
    }

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


    if (utilViewOptions -> ndiCursor.has_value()){
      const glm::vec2 cursorSizeNdi(0.02f, 0.02f);
      drawTools.drawRect(utilViewOptions -> ndiCursor.value().x, utilViewOptions -> ndiCursor.value().y, cursorSizeNdi.x, cursorSizeNdi.y, false, glm::vec4(1.f, 1.f, 1.f, 1.f), true, std::nullopt, "./res/textures/crosshairs/crosshair029.png", ShapeOptions { .zIndex = 6 }, std::nullopt);
    }


    if (utilViewOptions -> debugConfig.has_value()){
      Props props {
        .props = {
          PropPair { .symbol = valueSymbol, .value = utilViewOptions -> debugConfig.value() },
          PropPair { .symbol = xoffsetSymbol, .value = -1.f },
        },
      };
      debugComponent.draw(drawTools, props);
    }

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};
