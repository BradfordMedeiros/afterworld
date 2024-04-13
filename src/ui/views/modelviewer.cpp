#include "./modelviewer.h"

const float modelViewerButtonPadding = 0.02f;
const int rightButtonSymbol = getSymbol("right-button");
const int leftButtonSymbol = getSymbol("left-button");

Component modelViewerComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string("Model Viewer") },
	     	PropPair { .symbol = paddingSymbol, .value = modelViewerButtonPadding },
	     	PropPair { .symbol = tintSymbol, .value =  styles.secondaryColor },
	     	PropPair { .symbol = colorSymbol, .value =  styles.highlightColor },
      },
    };

    auto currentModelLabel = withPropsCopy(listItem, listItemProps);

    std::function<void()>* leftButton = typeFromProps<std::function<void()>>(props, leftButtonSymbol);
    modassert(leftButton, "left button not defined");
    Props prevButtonProps { .props = { 
    	PropPair { .symbol = valueSymbol, .value = std::string("PREV") },
    	PropPair { .symbol = onclickSymbol, .value = *leftButton },
     	PropPair { .symbol = paddingSymbol, .value = modelViewerButtonPadding },
    }};
    auto previousModelButton = withPropsCopy(button, prevButtonProps);


    std::function<void()>* rightButton = typeFromProps<std::function<void()>>(props, rightButtonSymbol);
    modassert(rightButton, "right button not defined");
    Props nextButtonProps { .props = { 
    	PropPair { .symbol = valueSymbol, .value = std::string("NEXT") },
    	PropPair { .symbol = onclickSymbol, .value = *rightButton },
     	PropPair { .symbol = paddingSymbol, .value = modelViewerButtonPadding },
    }};
    auto nextModelButton = withPropsCopy(button, nextButtonProps);

    std::vector<Component> children = { previousModelButton, currentModelLabel, nextModelButton };
    Layout layout {
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.2f),
      .showBackpanel = false,
      .borderColor = std::nullopt,
      .minwidth = 2.f * (2.f / 3.f),
      .minheight = 2.f,
      .layoutType = LAYOUT_HORIZONTAL2,
      .layoutFlowHorizontal = UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNone2,
      .alignVertical = UILayoutFlowPositive2,
      .spacing = styles.dockElementPadding,
      .minspacing = 0.f,
      .padding = styles.dockElementPadding,
      .children = children,
    };
    Props listLayoutProps {
      .props = {
        { .symbol = yoffsetSymbol, .value = 0.f },
        { .symbol = layoutSymbol, .value = layout },
      },
    };


    return layoutComponent.draw(drawTools, listLayoutProps);
  },
};
