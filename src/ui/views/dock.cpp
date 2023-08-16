#include "./dock.h"



Layout createDockLayout(std::string& title){
  std::vector<Component> elements;
  std::function<void()> onClick = []() -> void {
    std::cout << "on click" << std::endl;
  };
  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = title },
      PropPair { .symbol = onclickSymbol, .value = onClick },
    },
  };
  auto listItemWithProps = withPropsCopy(listItem, listItemProps);
  elements.push_back(listItemWithProps);

  Layout layout {
    .tint = glm::vec4(0.f, 0.f, 0.f, 0.8f),
    .showBackpanel = true,
    .borderColor = std::nullopt,
    .minwidth = 0.5f,
    .minheight = 1.9f,
    .layoutType = LAYOUT_VERTICAL2,
    .layoutFlowHorizontal = UILayoutFlowNegative2,
    .layoutFlowVertical = UILayoutFlowPositive2,
    .alignHorizontal = UILayoutFlowNone2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = 0.f,
    .children = elements,
  };
  return layout;
}


std::map<std::string, Component> dockToComponent {

};

Component dockComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto strValue = strFromProp(props, titleSymbol, "Dock");

    auto layout = createDockLayout(strValue);
    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    return withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
  },
};



