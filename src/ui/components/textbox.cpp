#include "./textbox.h"

Component textbox {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto strValue = strFromProp(props, valueSymbol, "default textbox");

    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol,   .value = strValue },
        PropPair { .symbol = paddingSymbol, .value = 0.02f },
      },
    };

  	return listItem.draw(drawTools, listItemProps);
  },
};

