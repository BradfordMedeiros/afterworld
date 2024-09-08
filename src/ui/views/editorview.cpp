#include "./editorview.h"

glm::vec4 colorPickerColor(0.f, 0.f, 0.f, 1.f);

Component editorViewComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    EditorViewOptions* editorOptions = typeFromProps<EditorViewOptions>(props, valueSymbol);
    auto onNewColor = editorOptions -> onNewColor;

		std::function<void(glm::vec4)> onSlide = [onNewColor](glm::vec4 value) -> void {
		  colorPickerColor = value;
		  if (onNewColor.has_value()){
		    onNewColor.value()(colorPickerColor);
		  }
		};

    auto colorPicker = withPropsCopy(colorPickerComponent, Props {
      .props = { 
        PropPair { onSlideSymbol,  onSlide },
        PropPair { tintSymbol, colorPickerColor },
      }
    });
    auto uiWindowComponent = createUiWindow(
      colorPicker, 
      windowColorPickerSymbol, 
      []() -> void { windowSetEnabled(windowColorPickerSymbol, false); }, 
      *(editorOptions -> colorPickerTitle)
    );
    auto defaultWindowProps = getDefaultProps();
    uiWindowComponent.draw(drawTools, defaultWindowProps);

    {
      Props navbarProps {
        .props = {
          { onclickSymbol, editorOptions -> onClickNavbar },
          { valueSymbol, editorOptions -> navbarType },
        }
      };
      navbarComponent.draw(drawTools, navbarProps);
    }

    Props worldPlayProps {
       .props = {
         PropPair { .symbol = xoffsetSymbol, .value = 0.f },
         PropPair { .symbol = yoffsetSymbol, .value = -1.f },
         PropPair { .symbol = valueSymbol, .value = editorOptions -> worldPlayInterface },
       }
    };
    worldplay.draw(drawTools, worldPlayProps);

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};

