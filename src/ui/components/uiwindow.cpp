#include "./uiwindow.h"

std::function<void()> onWindowDragValue = []() -> void {
  std::cout << "on window drag" << std::endl;
  windowOnDrag(windowColorPickerSymbol);
};


Component createUiWindow(Component& component, std::string& titleValue, std::function<void()>& onClick, std::optional<std::function<void()>>& onClickX){
  auto titleTextbox = withPropsCopy(listItem, Props {
    .props = {
      PropPair { .symbol = valueSymbol, .value = titleValue },
      PropPair { .symbol = onclickSymbol, .value = onClick },
    }
  });

  Component componentUiWindow {
    .draw = [&component, titleTextbox, &onClick, &onClickX](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      std::vector<Component> allComponents;
      allComponents.push_back(titleTextbox);
      allComponents.push_back(component);
        
      //props.props.push_back(PropPair { onWindowDragSymbol, onWindowDragValue });

      auto boundingBox = simpleVerticalLayout(allComponents).draw(drawTools, props);
      if (onClickX.has_value()){
        drawWindowX(drawTools, boundingBox, onClickX.value());
      }

      int* symbol = typeFromProps<int>(props, windowSymbol);
      modassert(symbol, "window symbol is null");
      std::cout << "symbol is: " << nameForSymbol(*symbol) << std::endl;
      auto oldCoords = windowGetPreDragOffset(*symbol);

      BoundingBox2D copyBoundingBox = boundingBox;
      copyBoundingBox.x = oldCoords.x;
      copyBoundingBox.y = oldCoords.y;
      if (windowIsBeingDragged(*symbol)){
        drawFillDebugBoundingBox(drawTools, copyBoundingBox);
      }

      return boundingBox;
    }
  };
  return componentUiWindow;
}


