#include "./checkbox.h"

Props checkboxOptions(){
  Props props {
    .props = {
      PropPair { .symbol = valueSymbol, .value = std::string("checkbox") },
      PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f) },
      PropPair { .symbol = paddingSymbol,      .value = 0.02f },
    },
  };
  return props;
}

Component checkbox {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	auto checkboxProps = checkboxOptions();
  	return withPropsCopy(listItem,  checkboxProps).draw(drawTools, props);
  },
};

