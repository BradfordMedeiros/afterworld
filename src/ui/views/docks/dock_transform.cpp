#include "./dock_transform.h"

Props transformOptions(){
  std::vector<ListComponentData> levels;
  levels.push_back(ListComponentData {
  	.name = "transform",
  	.onClick = std::nullopt,
  });
  
  Props levelProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = std::string("wow") },
      //PropPair { .symbol = listItemsSymbol, .value = levels },
      //PropPair { .symbol = xoffsetSymbol,   .value = -0.81f },
      //PropPair { .symbol = yoffsetSymbol,   .value = 0.98f },
      //PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
      //PropPair { .symbol = horizontalSymbol,   .value = true },
      //PropPair { .symbol = paddingSymbol,      .value = 0.02f },
    },
  };
  return levelProps;
}


Component dockTransformComponent {
  .draw = [](DrawingTools& drawTools, Props& props){	
  	Props defaultProps { .props = {} };
  	return withPropsCopy(listItem,  transformOptions()).draw(drawTools, props);
  },
};
