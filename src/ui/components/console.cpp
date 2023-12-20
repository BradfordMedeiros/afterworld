#include "./console.h"

const int CONSOLE_WIDTH = 0.5f;
const int CONSOLE_HEIGHT = 0.5f;

Component consoleComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	std::vector<Component> elements = {};

    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string("test value") },
        PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
        PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
        PropPair { .symbol = paddingSymbol, .value = 0.01f },
        //PropPair { .symbol = onclickSymbol, .value = onClick },
      },
    };
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    elements.push_back(listItemWithProps);

  	Layout layout {
  	  .tint = glm::vec4(0.f, 0.f, 1.f, 0.1f),
  	  .showBackpanel = true,
  	  .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.1f),
  	  .minwidth = CONSOLE_WIDTH,
  	  .minheight = CONSOLE_HEIGHT,
  	  .layoutType = LAYOUT_VERTICAL2,
  	  .layoutFlowHorizontal = UILayoutFlowNone2,
  	  .layoutFlowVertical = UILayoutFlowNegative2,
  	  .alignHorizontal = UILayoutFlowNegative2,
  	  .alignVertical = UILayoutFlowPositive2,
  	  .spacing = 0.f,
  	  .minspacing = 0.f,
  	  .padding = 0.f,
  	  .children = elements,
  	};
  	Props listLayoutProps {
  	  .props = {
  	    { .symbol = layoutSymbol, .value = layout },
  	  },
  	};
  	auto layoutScenegraph = withPropsCopy(layoutComponent, listLayoutProps);
  	//return layoutScenegraph.draw(drawTools, props);


    return BoundingBox2D {
  		.x = 0,
  		.y = 0,
  		.width = CONSOLE_WIDTH,
  		.height = CONSOLE_HEIGHT,
    };
  },
};