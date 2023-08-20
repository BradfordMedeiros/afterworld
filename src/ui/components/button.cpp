#include "./button.h"

Props buttonOptions(){
  Props levelProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = std::string("create") },
      PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f) },
      PropPair { .symbol = paddingSymbol,      .value = 0.02f },
    },
  };
  return levelProps;
}

Component button {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	auto buttonProps = buttonOptions();
  	return withPropsCopy(listItem,  buttonProps).draw(drawTools, props);
  },
};

