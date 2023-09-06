#include "./uiwindow.h"


Component createUiWindow(Component& component, int symbol){
  Component componentUiWindow {
    .draw = [&component, symbol](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      auto onClickX = fnFromProp(props, onclickSymbol);

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
        
      auto boundingBox = simpleVerticalLayout(allComponents).draw(drawTools, props);
      if (onClickX.has_value()){
        drawWindowX(drawTools, boundingBox, onClickX.value());
      }

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


