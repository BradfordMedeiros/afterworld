#include "./debug.h"

extern CustomApiBindings* gameapi;

struct RowItem {
  std::string value;
  std::optional<std::function<void()>> fn;
};

Component createRow(std::vector<RowItem> values){
  std::vector<Component> elements;
  for (int i = 0 ; i < values.size(); i++){
   // ListComponentData& listItemData = listItems.at(i);
    std::function<void()> onClick = []() -> void {};
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = values.at(i).value },
      },
    };
    if (values.at(i).fn.has_value()){
      listItemProps.props.push_back(PropPair { .symbol = onclickSymbol, .value = values.at(i).fn.value() });
    }
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
    .layoutFlowHorizontal = UILayoutFlowPositive2,
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

Component createGrid(std::vector<std::vector<RowItem>> gridConfig){
  std::vector<Component> rows;
  for (int y = 0; y < gridConfig.size(); y++){
    std::vector<RowItem> values;
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

    std::vector<std::vector<RowItem>> values;
    for (int i = 0; i < config -> data.size(); i++){
      auto& row = config -> data.at(i);
      std::vector<RowItem> rowValue;
      for (auto &col : row){
        auto strVal = std::get_if<std::string>(&col);
        auto debugItem = std::get_if<DebugItem>(&col);
        modassert(strVal || debugItem, "invalid type to config");
        if(strVal){
          rowValue.push_back(RowItem { .value = *strVal });
        }else if (debugItem){
          rowValue.push_back(RowItem { .value = debugItem -> text, .fn = debugItem -> onClick });
        }
      }
      values.push_back(rowValue);
    }

    auto grid = createGrid(values);
    auto listBoundingBox = grid.draw(drawTools, props);
    return listBoundingBox;
  },
};
