#include "./debug.h"

extern CustomApiBindings* gameapi;


Component createRow(std::vector<std::string> values){
  std::vector<Component> elements;
  for (int i = 0 ; i < values.size(); i++){
   // ListComponentData& listItemData = listItems.at(i);
   // auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = values.at(i) },
        //PropPair { .symbol = onclickSymbol, .value = onClick },
        //PropPair { .symbol = colorSymbol, .value = selectedIndex == i ? glm::vec4(0.f, 0.f, 1.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f) },
        //PropPair { .symbol = paddingSymbol, .value = itemPadding },
      },
    };
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    elements.push_back(listItemWithProps);
  }
  Layout layout {
    .tint = glm::vec4(0.f, 0.f, 0.f, 0.6f),
    .showBackpanel = true,
    .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.f),
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = LAYOUT_HORIZONTAL2,
    .layoutFlowHorizontal = UILayoutFlowNone2,
    .layoutFlowVertical = UILayoutFlowNone2,
    .alignHorizontal = UILayoutFlowNegative2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.02f,
    .minspacing = 0.f,
    .padding = 0.02f,
    .children = elements,
  };

  Props listLayoutProps {
    .props = {
      { .symbol = layoutSymbol, .value = layout },
    },
  };
  return withPropsCopy(layoutComponent, listLayoutProps);
}

Component createGrid(std::vector<Component> rows){
  Layout layout {
    .tint = glm::vec4(0.f, 0.f, 0.f, 0.6f),
    .showBackpanel = true,
    .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.f),
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = LAYOUT_VERTICAL2,
    .layoutFlowHorizontal = UILayoutFlowNone2,
    .layoutFlowVertical = UILayoutFlowNone2,
    .alignHorizontal = UILayoutFlowNegative2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.02f,
    .minspacing = 0.f,
    .padding = 0.02f,
    .children = rows,
  };

  Props listLayoutProps {
    .props = {
      { .symbol = layoutSymbol, .value = layout },
    },
  };
  return withPropsCopy(layoutComponent, listLayoutProps); 
}

Component createGrid(std::vector<std::vector<std::string>> gridConfig){
  std::vector<Component> rows;
  for (int y = 0; y < gridConfig.size(); y++){
    std::vector<std::string> values;
    for (int x = 0; x < gridConfig.at(y).size(); x++){
      values.push_back(gridConfig.at(y).at(x));
    }
    rows.push_back(createRow(values));
  }
  auto grid = createGrid(rows);
  return grid;
}

Component debugComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
	  auto config = typeFromProps<DebugConfig>(props, valueSymbol);
    modassert(config, "debugConfig must be defined for debugComponent");

    std::vector<std::vector<std::string>> values;
    for (int i = 0; i < config -> data.size(); i++){
      auto& row = config -> data.at(i);
      std::vector<std::string> rowValue;
      for (auto &col : row){
        auto strVal = std::get_if<std::string>(&col);
        auto debugItem = std::get_if<DebugItem>(&col);
        modassert(strVal || debugItem, "invalid type to config");
        if(strVal){
          rowValue.push_back(*strVal);
        }else if (debugItem){
          rowValue.push_back(debugItem -> text);
        }
      }
      values.push_back(rowValue);
    }

    auto grid = createGrid(values);
    auto listBoundingBox = grid.draw(drawTools, props);
    return listBoundingBox;
  },
};
