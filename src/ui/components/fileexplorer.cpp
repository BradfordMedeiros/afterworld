#include "./fileexplorer.h"

Component filexplorerComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol,   .value = std::string("fileexplorer placeholder") },
        PropPair { .symbol = paddingSymbol, .value = 0.02f },
      },
    };
  	return listItem.draw(drawTools, listItemProps);
  },
};

