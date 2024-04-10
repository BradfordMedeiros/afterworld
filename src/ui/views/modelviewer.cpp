#include "./modelviewer.h"

const float modelViewerButtonPadding = 0.02f;
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

    std::function<void()> onPrevModel = []() -> void {
    	std::cout << "on previous model" << std::endl;
    };


    Props prevButtonProps { .props = { 
    	PropPair { .symbol = valueSymbol, .value = std::string("PREV") },
    	PropPair { .symbol = onclickSymbol, .value = onPrevModel },
     	PropPair { .symbol = paddingSymbol, .value = modelViewerButtonPadding },
    }};
    auto previousModelButton = withPropsCopy(button, prevButtonProps);


    std::function<void()> onNextModel = []() -> void {
    	std::cout << "on next model" << std::endl;
    };
    Props nextButtonProps { .props = { 
    	PropPair { .symbol = valueSymbol, .value = std::string("NEXT") },
    	PropPair { .symbol = onclickSymbol, .value = onNextModel },
     	PropPair { .symbol = paddingSymbol, .value = modelViewerButtonPadding },
    }};
    auto nextModelButton = withPropsCopy(button, nextButtonProps);

    std::vector<Component> children = { previousModelButton, currentModelLabel, nextModelButton };
    Layout layout {
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.2f),
      .showBackpanel = true,
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
