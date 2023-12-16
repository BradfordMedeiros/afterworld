#include "./list.h"


Layout createLayout(std::vector<ListComponentData> listItems, bool horizontal, UILayoutFlowType2 flowVertical, UILayoutFlowType2 flowHorizontal, glm::vec4 tint, float minwidth, float minheight, int selectedIndex, std::optional<float> fontSize, float layoutPadding, float itemPadding){
  std::vector<Component> elements;
  for (int i = 0 ; i < listItems.size(); i++){
    ListComponentData& listItemData = listItems.at(i);
    auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = listItemData.name },
        PropPair { .symbol = onclickSymbol, .value = onClick },
        PropPair { .symbol = colorSymbol, .value = selectedIndex == i ? glm::vec4(0.f, 0.f, 1.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f) },
        PropPair { .symbol = paddingSymbol, .value = itemPadding },

      },
    };
    if (fontSize.has_value()){
      listItemProps.props.push_back(PropPair { .symbol = fontsizeSymbol, .value = fontSize.value() });
    }
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    elements.push_back(listItemWithProps);
  }
  Layout layout {
    .tint = tint,
    .showBackpanel = true,
    .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.2f),
    .minwidth = minwidth,
    .minheight = minheight,
    .layoutType = horizontal ? LAYOUT_HORIZONTAL2 : LAYOUT_VERTICAL2,
    .layoutFlowHorizontal = flowHorizontal,
    .layoutFlowVertical = flowVertical,
    .alignHorizontal = UILayoutFlowNegative2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = layoutPadding,
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

    auto tint = vec4FromProp(props, tintSymbol, glm::vec4(1.f, 0.f, 0.f, 1.f));
    auto selectedIndex = intFromProp(props, selectedSymbol, -1);

    auto flowHorizontalProp = typeFromProps<UILayoutFlowType2>(props, flowHorizontal);
    auto flowHorizontalValue = UILayoutFlowNone2;
    if (flowHorizontalProp){
      flowHorizontalValue = *flowHorizontalProp;
    }

    auto minwidth = floatFromProp(props, minwidthSymbol, 0.f);
    auto minheight = floatFromProp(props, minheightSymbol, 0.f);
    auto fontSize = floatFromProp(props, fontsizeSymbol);
    auto padding = floatFromProp(props, paddingSymbol, 0.f);
    auto itemPadding = floatFromProp(props, itemPaddingSymbol, 0.f);

    auto layout = createLayout(*listItems, horizontal, flowVerticalValue, flowHorizontalValue, tint, minwidth, minheight, selectedIndex, fontSize, padding, itemPadding);
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


Component wrapWithLabel(Component& innerComponent){
  Component wrappedComponent {
    .draw = [&innerComponent](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      auto innerCheckbox = withProps(innerComponent, props);
      Props childProps {
        .props = {},
      };
      auto strValue = strFromProp(props, valueSymbol, "untitled");
      std::function<void()> onClickTest = []() -> void {};
      Props listItemProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = strValue },
          PropPair { .symbol = onclickSymbol, .value = onClickTest },
          PropPair { .symbol = paddingSymbol, .value = 0.025f },
        },
      };
      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      std::vector<Component> components;
      components.push_back(listItemWithProps);
      components.push_back(innerCheckbox);
      auto layout = simpleHorizontalLayout(components);
      return layout.draw(drawTools, childProps);
    },
  };
  return wrappedComponent;
}
