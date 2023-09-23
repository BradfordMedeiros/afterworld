#include "./button.h"

Component button {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::function<void()>* onClick = typeFromProps<std::function<void()>>(props, onclickSymbol);
    modassert(onClick, "on click not defined in button");
    auto strValue = strFromProp(props, valueSymbol, "button");

    Props buttonProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = strValue },
        PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f) },
        PropPair { .symbol = paddingSymbol,      .value = 0.02f },
        PropPair { .symbol = onclickSymbol, .value = *onClick },
      },
    };

    Props defaultProps {
      .props = {},
    };
  	auto buttonBox = withPropsCopy(listItem,  buttonProps).draw(drawTools, defaultProps);
    //drawDebugBoundingBox(drawTools, buttonBox, glm::vec4(1.f, 0.f, 1.f, 1.f));
    return buttonBox;
  },
};

