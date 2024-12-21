#include "./navigation.h"

Component navigationComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::function<void()>* onClick = typeFromProps<std::function<void()>>(props, onclickSymbol);
    modassert(onClick, "on click symbol not provided to navigationComponent");
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string("BACK") },
        PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 0.2f) },
        PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
        PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
        PropPair { .symbol = onclickSymbol, .value = *onClick },
        PropPair { .symbol = minwidthSymbol, .value = 0.5f },
        PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.1f) },
        PropPair { .symbol = fontsizeSymbol, .value = 0.03f }
      },
    };
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    return listItemWithProps.draw(drawTools, props);
  }
};

Component withNavigation(UiContext& uiContext, Component wrappedComponent){
  Component component {
    .draw = [&uiContext, wrappedComponent](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      Props emptyProps {
        .props = {
          PropPair { .symbol = xoffsetSymbol, .value = -0.95f },
          PropPair { .symbol = yoffsetSymbol, .value = -0.9f },
          PropPair { .symbol = onclickSymbol, .value = uiContext.consoleInterface.routerPop },
        }
      };
      auto boundingBox = wrappedComponent.draw(drawTools, props);
      navigationComponent.draw(drawTools, emptyProps);
      return boundingBox;
    }
  };
  return component;
}