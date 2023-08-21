#include "./list.h"


Layout createLayout(std::vector<ListComponentData> listItems, bool horizontal){
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
    .tint = glm::vec4(1.f, 0.f, 0.f, 0.8f),
    .showBackpanel = true,
    .borderColor = glm::vec4(0.f, 1.f, 1.f, 1.f),
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = horizontal ? LAYOUT_HORIZONTAL2 : LAYOUT_VERTICAL2,
    .layoutFlowHorizontal = UILayoutFlowPositive2,
    .layoutFlowVertical = UILayoutFlowNone2,
    .alignHorizontal = UILayoutFlowPositive2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = 0.02f,
    .children = elements,
  };
  return layout;
}

Component listComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto listItems = typeFromProps<std::vector<ListComponentData>>(props, listItemsSymbol);
    modassert(listItems, "invalid listItems prop");
    auto horizontal = boolFromProp(props, horizontalSymbol, false);

    auto layout = createLayout(*listItems, horizontal);
    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    return withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
  },
};
