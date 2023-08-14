#include "./list.h"

const int valueSymbol = getSymbol("value");
const int onclickSymbol = getSymbol("onclick");

Layout createLayout(std::vector<ListComponentData> listItems){
  std::vector<Component> elements;
  for (int i = 0 ; i < listItems.size(); i++){
    ListComponentData& listItemData = listItems.at(i);
    auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
    Props listItemProps {
      .mappingId = std::nullopt,
      .props = {
        PropPair { .symbol = valueSymbol, .value = listItemData.name },
        PropPair { .symbol = onclickSymbol, .value = onClick },
      },
    };
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    elements.push_back(listItemWithProps);
  }
  Layout layout {
    .tint = glm::vec4(0.f, 0.f, 0.f, 0.8f),
    .showBackpanel = false,
    .borderColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.f),
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = LAYOUT_VERTICAL2,
    .layoutFlowHorizontal = UILayoutFlowNone2,
    .layoutFlowVertical = UILayoutFlowNone2,
    .alignHorizontal = UILayoutFlowNone2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = 0.f,
    .children = elements,
  };
  return layout;
}

const int listItemsSymbol = getSymbol("listitems");
const int layoutSymbol = getSymbol("layout");

Component listComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto listItems = typeFromProps<std::vector<ListComponentData>>(props, listItemsSymbol);
    modassert(listItems, "invalid listItems prop");
    auto layout = createLayout(*listItems);
    Props listLayoutProps {
      .mappingId = std::nullopt,
      .props = {
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    return withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
  },
};
