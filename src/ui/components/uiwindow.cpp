#include "./uiwindow.h"

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
      auto boundingBox = simpleVerticalLayout(allComponents).draw(drawTools, props);
      if (onClickX.has_value()){
        drawWindowX(drawTools, boundingBox, onClickX.value());
      }
      return boundingBox;
    }
  };
  return componentUiWindow;
}
