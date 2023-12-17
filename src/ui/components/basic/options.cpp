#include "./options.h"

Props optionsProps(Options& options, float itemPadding){
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
      PropPair { .symbol = horizontalSymbol,   .value = true },
      PropPair { .symbol = selectedSymbol, options.selectedIndex },
      PropPair { .symbol = itemPaddingSymbol,      .value = itemPadding },
    },
  };
  return levelProps;
}

const int optionsSymbol = getSymbol("options");

Component options {
  .draw = [](DrawingTools& drawTools, Props& props){	
    auto options = typeFromProps<Options>(props, optionsSymbol);
    auto itemPadding = floatFromProp(props, itemPaddingSymbol, 0.f);
    modassert(options, "options - options not defined");
  	return withPropsCopy(listComponent,  optionsProps(*options, itemPadding)).draw(drawTools, props);
  },
};
