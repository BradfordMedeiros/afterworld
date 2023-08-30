#include "./dialog.h"

Component simpleVerticalLayout(std::vector<Component>& children){
  Layout layout {
    .tint = glm::vec4(0.f, 0.f, 0.f, 0.5f),
    .showBackpanel = true,
    .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.2f),
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = LAYOUT_VERTICAL2,
    .layoutFlowHorizontal = UILayoutFlowNone2,
    .layoutFlowVertical = UILayoutFlowNone2,
    .alignHorizontal = UILayoutFlowNegative2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = 0.02f,
    .children = children,
  };
  Props listLayoutProps {
    .props = {
      { .symbol = layoutSymbol, .value = layout },
    },
  };
  return withPropsCopy(layoutComponent, listLayoutProps);
}

Component simpleHorizontalLayout(std::vector<Component>& children){
  Layout layout {
    .tint = glm::vec4(0.f, 0.f, 0.f, 0.5f),
    .showBackpanel = true,
    .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.2f),
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = LAYOUT_HORIZONTAL2,
    .layoutFlowHorizontal = UILayoutFlowNone2,
    .layoutFlowVertical = UILayoutFlowNone2,
    .alignHorizontal = UILayoutFlowNegative2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = 0.02f,
    .children = children,
  };
  Props listLayoutProps {
    .props = {
      { .symbol = layoutSymbol, .value = layout },
    },
  };
  return withPropsCopy(layoutComponent, listLayoutProps);
}

Component dialogComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto listItemPtr = typeFromProps<std::vector<ListComponentData>>(props, listItemsSymbol);
    modassert(listItemPtr, "invalid listItems prop");
 
    auto listItems = *listItemPtr;
    std::vector<Component> choiceElements;
    for (int i = 0 ; i < listItems.size(); i++){
      ListComponentData& listItemData = listItems.at(i);
      auto onClick = listItemData.onClick.has_value() ? listItemData.onClick.value() : []() -> void {};
      Props listItemProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = listItemData.name },
          PropPair { .symbol = onclickSymbol, .value = onClick },
          PropPair { .symbol = tintSymbol, .value = glm::vec4(0.2f, 0.2f, 0.2f, 0.8f) },
        },
      };
      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      choiceElements.push_back(listItemWithProps);
    }

    auto horizontalComponent = simpleHorizontalLayout(choiceElements);

    std::vector<Component> elements;

    auto titleTextbox = withPropsCopy(textbox, Props {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string("title placeholder") },
      }
    });

    auto detailTextbox = withPropsCopy(textbox, Props {
      .props = { 
        PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 0.f) },
        PropPair { .symbol = valueSymbol, .value = std::string("detail placeholder") },
      },
    });
    elements.push_back(titleTextbox);
    elements.push_back(detailTextbox);
    elements.push_back(horizontalComponent);

    auto listBoundingBox = simpleVerticalLayout(elements).draw(drawTools, props);
    //drawDebugBoundingBox(drawTools, listBoundingBox, glm::vec4(0.f, 1.f, 0.f, 1.f));
    return listBoundingBox;
  },
};
