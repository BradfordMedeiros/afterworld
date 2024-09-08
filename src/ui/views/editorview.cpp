#include "./editorview.h"

Component editorViewComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    EditorViewOptions* editorOptions = typeFromProps<EditorViewOptions>(props, valueSymbol);
  
    Props worldPlayProps {
       .props = {
         PropPair { .symbol = xoffsetSymbol, .value = 0.f },
         PropPair { .symbol = yoffsetSymbol, .value = -1.f },
         PropPair { .symbol = valueSymbol, .value = editorOptions -> worldPlayInterface },
       }
    };
    worldplay.draw(drawTools, worldPlayProps);

    return { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
  },
};

