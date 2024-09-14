#include "./button.h"


Component button {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::function<void()>* onClick = typeFromProps<std::function<void()>>(props, onclickSymbol);
    auto strValue = strFromProp(props, valueSymbol, "button");
    auto paddingAmount = floatFromProp(props, paddingSymbol, 0.f);

    Props buttonProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = strValue },
        PropPair { .symbol = paddingSymbol, .value = paddingAmount },
      },
    };
    if (onClick){
      buttonProps.props.push_back(PropPair { .symbol = onclickSymbol, .value = *onClick });
    }

    Props defaultProps {
      .props = {},
    };
  	auto buttonBox = withPropsCopy(listItem,  buttonProps).draw(drawTools, defaultProps);
    //drawDebugBoundingBox(drawTools, buttonBox, glm::vec4(1.f, 0.f, 1.f, 1.f));
    return buttonBox;
  },
};

