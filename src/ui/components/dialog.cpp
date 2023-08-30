#include "./dialog.h"

Component dialogComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol,   .value = std::string("dialog placeholder") },
        PropPair { .symbol = paddingSymbol, .value = 0.02f },
      },
    };
  	return listItem.draw(drawTools, listItemProps);
  },
};

