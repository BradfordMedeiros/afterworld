#include "./options.h"

Props optionsProps(Options& options){
  std::vector<ListComponentData> levels;
  for (auto &option : options.options){
    levels.push_back(ListComponentData {
      .name = option.name,
      .onClick = option.onClick,
    });
  }

  Props levelProps {
    .props = {
      PropPair { .symbol = listItemsSymbol, .value = levels },
      //PropPair { .symbol = xoffsetSymbol,   .value = -0.81f },
      //PropPair { .symbol = yoffsetSymbol,   .value = 0.98f },
      PropPair { .symbol = tintSymbol,      .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
      PropPair { .symbol = horizontalSymbol,   .value = true },
      PropPair { .symbol = selectedSymbol, options.selectedIndex },

      //PropPair { .symbol = paddingSymbol,      .value = 0.02f },
    },
  };
  return levelProps;
}

const int optionsSymbol = getSymbol("options");

Component options {
  .draw = [](DrawingTools& drawTools, Props& props){	
    auto options = typeFromProps<Options>(props, optionsSymbol);
    modassert(options, "options - options not defined");
  	return withPropsCopy(listComponent,  optionsProps(*options)).draw(drawTools, props);
  },
};
