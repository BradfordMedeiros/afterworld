#include "./textbox.h"

Component textbox {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto strValue = strFromProp(props, valueSymbol, "default textbox");
    auto tint = vec4FromProp(props, tintSymbol, glm::vec4(0.f, 0.f, 0.f, 1.f));

    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol,   .value = strValue },
        PropPair { .symbol = paddingSymbol, .value = 0.02f },
        PropPair { .symbol = tintSymbol, .value = tint },
      },
    };

  	return listItem.draw(drawTools, listItemProps);
  },
};

