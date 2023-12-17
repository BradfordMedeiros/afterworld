#include "./uiwindow.h"


const float STYLE_UI_WINDOW_FONTSIZE = 0.02f;
const float STYLE_UI_WINDOW_PADDING = 0.02f;
const glm::vec4 STYLE_UI_WINDOW_TINT(0.f, 0.f, 1.f, 0.1f);

Component createUiWindow(Component& component, int symbol, std::string titleValue, AlignmentParams alignment){
  Component componentUiWindow {
    .draw = [&component, symbol, titleValue, alignment](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
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

      auto titleTextbox = withPropsCopy(listItem, Props {
        .props = {
          PropPair { .symbol = valueSymbol, .value = titleValue },
          PropPair { .symbol = onclickSymbol, .value = onClick },
          PropPair { .symbol = fontsizeSymbol, .value = STYLE_UI_WINDOW_FONTSIZE },
          PropPair { .symbol = paddingSymbol, .value = STYLE_UI_WINDOW_PADDING },
          PropPair { .symbol = tintSymbol, .value = STYLE_UI_WINDOW_TINT },
          PropPair { .symbol = minwidthSymbol, .value = 0.5f - (STYLE_UI_WINDOW_PADDING * 2) },
        }
      });

      std::vector<Component> allComponents;
      allComponents.push_back(titleTextbox);
      allComponents.push_back(component);
        
      auto windowOffset = windowGetOffset(symbol);
      props.props.push_back(PropPair { xoffsetSymbol, windowOffset.x  });
      props.props.push_back(PropPair { yoffsetSymbol, windowOffset.y  });


      auto boundingBox = simpleVerticalLayout(allComponents, glm::vec2(0.f, 0.f), alignment, styles.mainBorderColor).draw(drawTools, props);
      drawWindowX(drawTools, boundingBox, onClickX);

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


