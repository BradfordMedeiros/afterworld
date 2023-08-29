#include "./button.h"

Component button {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto onClick = fnFromProp(props, onclickSymbol); 
    Props buttonProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string("create") },
        PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f) },
        PropPair { .symbol = paddingSymbol,      .value = 0.02f },
      },
    };

    std::function<void()> onClick2 = onClick.has_value() ? onClick.value() : nullClick.value();

    buttonProps.props.push_back(PropPair {
      .symbol = onclickSymbol,
      .value = onClick2,
    });


    //auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
  

    Props defaultProps {
      .props = {},
    };
  	auto buttonBox = withPropsCopy(listItem,  buttonProps).draw(drawTools, defaultProps);
    //drawDebugBoundingBox(drawTools, buttonBox, glm::vec4(1.f, 0.f, 1.f, 1.f));
    return buttonBox;
  },
};

