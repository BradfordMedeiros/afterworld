#include "./list.h"


Layout createLayout(std::vector<ListComponentData> listItems, bool horizontal, UILayoutFlowType2 flowVertical, UILayoutFlowType2 flowHorizontal){
  std::vector<Component> elements;
  for (int i = 0 ; i < listItems.size(); i++){
    ListComponentData& listItemData = listItems.at(i);
    auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = listItemData.name },
        PropPair { .symbol = onclickSymbol, .value = onClick },
      },
    };
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    elements.push_back(listItemWithProps);
  }
  Layout layout {
    .tint = glm::vec4(1.f, 1.f, 1.f, 0.4f),
    .showBackpanel = true,
    .borderColor = std::nullopt,
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = horizontal ? LAYOUT_HORIZONTAL2 : LAYOUT_VERTICAL2,
    .layoutFlowHorizontal = flowHorizontal,
    .layoutFlowVertical = flowVertical,
    .alignHorizontal = UILayoutFlowNone2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = 0.f,
    .children = elements,
  };
  return layout;
}

Component listComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto listItems = typeFromProps<std::vector<ListComponentData>>(props, listItemsSymbol);
    modassert(listItems, "invalid listItems prop");
    auto horizontal = boolFromProp(props, horizontalSymbol, false);
    auto flowVerticalProp = typeFromProps<UILayoutFlowType2>(props, flowVertical);
    auto flowVerticalValue = UILayoutFlowNone2;
    if (flowVerticalProp){
      flowVerticalValue = *flowVerticalProp;
    }

    auto flowHorizontalProp = typeFromProps<UILayoutFlowType2>(props, flowHorizontal);
    auto flowHorizontalValue = UILayoutFlowNone2;
    if (flowHorizontalProp){
      flowHorizontalValue = *flowHorizontalProp;
    }

    auto layout = createLayout(*listItems, horizontal, flowVerticalValue, flowHorizontalValue);
    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    return withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
  },
};
