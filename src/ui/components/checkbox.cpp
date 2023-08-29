#include "./checkbox.h"

Layout createSimpleLayout(std::vector<ListComponentData> listItems, UILayoutFlowType2 flowVertical, UILayoutFlowType2 flowHorizontal, glm::vec4 tint, float minwidth){
  std::vector<Component> elements;
  for (int i = 0 ; i < listItems.size(); i++){
    ListComponentData& listItemData = listItems.at(i);
    auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = listItemData.name },
        PropPair { .symbol = onclickSymbol, .value = onClick },
        PropPair { .symbol = tintSymbol, .value = tint },
      },
    };
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    elements.push_back(listItemWithProps);
  }
  Layout layout {
    .tint = tint,
    .showBackpanel = true,
    .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.2f),
    .minwidth = minwidth,
    .minheight = 0.f,
    .layoutType = LAYOUT_HORIZONTAL2,
    .layoutFlowHorizontal = flowHorizontal,
    .layoutFlowVertical = flowVertical,
    .alignHorizontal = UILayoutFlowNegative2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = 0.f,
    .children = elements,
  };
  return layout;
}

Component simpleList {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto listItems = typeFromProps<std::vector<ListComponentData>>(props, listItemsSymbol);
    modassert(listItems, "invalid listItems prop");
    auto flowVerticalProp = typeFromProps<UILayoutFlowType2>(props, flowVertical);
    auto flowVerticalValue = UILayoutFlowNone2;
    if (flowVerticalProp){
      flowVerticalValue = *flowVerticalProp;
    }

    auto tint = vec4FromProp(props, tintSymbol, glm::vec4(0.f, 0.f, 0.f, 1.f));

    auto flowHorizontalProp = typeFromProps<UILayoutFlowType2>(props, flowHorizontal);
    auto flowHorizontalValue = UILayoutFlowNone2;
    if (flowHorizontalProp){
      flowHorizontalValue = *flowHorizontalProp;
    }

    auto minwidth = floatFromProp(props, minwidthSymbol, 0.f);

    auto layout = createSimpleLayout(*listItems, flowVerticalValue, flowHorizontalValue, tint, minwidth);
    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    auto listBoundingBox = withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
    //drawDebugBoundingBox(drawTools, listBoundingBox, glm::vec4(0.f, 1.f, 0.f, 1.f));
    return listBoundingBox;
  },
};

Component checkbox {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto strValue = strFromProp(props, valueSymbol, "default checkbox");
    auto isChecked = boolFromProp(props, checkedSymbol, false);

    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol,   .value = (strValue + (isChecked ? "(checked)" : "(unchecked)")) },
        PropPair { .symbol = tintSymbol,    .value = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f) },
        PropPair { .symbol = paddingSymbol, .value = 0.02f },
      },
    };


    auto checkedTint = isChecked ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 1.f, 1.f);
    auto measurer = createMeasurer();

    BoundingBox2D checkboxBounding {
      .x = 0.f, 
      .y = 0.f,
      .width = 0.1f,
      .height = 0.1f,
    };
    drawTools.drawRect(0.f, 0.f, 0.1f, 0.1f, false, glm::vec4(0.f, 0.f, 0.f, 1.f), std::nullopt, true, uniqueMenuItemMappingId(), std::nullopt);
    drawTools.drawRect(0.f, 0.f, 0.08f, 0.08f, false, checkedTint, std::nullopt, true, uniqueMenuItemMappingId(), std::nullopt);
    measureBoundingBox(measurer, checkboxBounding);
  	//auto listBoundingBox = listItem.draw(drawTools, listItemProps);
    //measureBoundingBox(measurer, listBoundingBox);
    

    return measurerToBox(measurer);
  },
};

