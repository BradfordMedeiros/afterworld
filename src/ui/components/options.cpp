#include "./options.h"

Props optionsProps(){
  std::vector<ListComponentData> levels;
  levels.push_back(ListComponentData {
  	.name = "opt1",
  	.onClick = nullClick,
  });
  levels.push_back(ListComponentData {
    .name = "opt2",
    .onClick = nullClick,
  });
  levels.push_back(ListComponentData {
    .name = "opt3",
    .onClick = nullClick,
  });
  
  Props levelProps {
    .props = {
      PropPair { .symbol = listItemsSymbol, .value = levels },
      PropPair { .symbol = xoffsetSymbol,   .value = -0.81f },
      PropPair { .symbol = yoffsetSymbol,   .value = 0.98f },
      PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
      PropPair { .symbol = horizontalSymbol,   .value = true },
      PropPair { .symbol = paddingSymbol,      .value = 0.02f },
    },
  };
  return levelProps;
}

Component options {
  .draw = [](DrawingTools& drawTools, Props& props){	
  	Props defaultProps { .props = {} };
  	return withPropsCopy(listComponent,  optionsProps()).draw(drawTools, props);
  },
};
