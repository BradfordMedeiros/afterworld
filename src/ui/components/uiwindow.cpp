#include "./uiwindow.h"


Component createUiWindow(Component& component, int symbol){
  Component componentUiWindow {
    .draw = [&component, symbol](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      auto enable = windowEnabled(symbol);
      if (!enable){
        return BoundingBox2D { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
      }

      std::function<void()> onClickX = [symbol]() -> void {
        windowSetEnabled(symbol, false);
      };

      std::function<void()> onClick = [symbol]() -> void {
          std::cout << "on window drag" << std::endl;
          windowOnDrag(symbol);
      };

      auto titleValue = strFromProp(props, titleSymbol, "Color Selection");
      auto titleTextbox = withPropsCopy(listItem, Props {
        .props = {
          PropPair { .symbol = valueSymbol, .value = titleValue },
          PropPair { .symbol = onclickSymbol, .value = onClick },
        }
      });

      std::vector<Component> allComponents;
      allComponents.push_back(titleTextbox);
      allComponents.push_back(component);
        
      auto windowOffset = windowGetOffset(symbol);
      props.props.push_back(PropPair { xoffsetSymbol, windowOffset.x  });
      props.props.push_back(PropPair { yoffsetSymbol, windowOffset.y  });

      auto boundingBox = simpleVerticalLayout(allComponents).draw(drawTools, props);
      drawWindowX(drawTools, boundingBox, onClickX);
      

      std::cout << "symbol is: " << nameForSymbol(symbol) << std::endl;
      auto oldCoords = windowGetPreDragOffset(symbol);

      BoundingBox2D copyBoundingBox = boundingBox;
      copyBoundingBox.x = oldCoords.x;
      copyBoundingBox.y = oldCoords.y;
      if (windowIsBeingDragged(symbol)){
        drawFillDebugBoundingBox(drawTools, copyBoundingBox);
      }

      return boundingBox;
    }
  };
  return componentUiWindow;
}


