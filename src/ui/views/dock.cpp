#include "./dock.h"

Component dockComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    static std::map<std::string, Component> dockToComponent {
      { "Cameras", dockCameraComponent },
      { "transform", dockTransformComponent },
    };  

    std::vector<Component> elements;

    auto strValue = strFromProp(props, titleSymbol, "Dock");
    std::function<void()> onClick = []() -> void { std::cout << "on click" << std::endl; };
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = strValue },
        PropPair { .symbol = onclickSymbol, .value = onClick },
      },
    };
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    elements.push_back(listItemWithProps);


    modassert(dockToComponent.find(strValue) != dockToComponent.end(), std::string("dock - no component for ") + strValue);
    auto component = dockToComponent.at(strValue);
    elements.push_back(component);

    Layout layout {
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.8f),
      .showBackpanel = false,
      .borderColor = glm::vec4(1.f, 0.f, 0.f, 0.8f),
      .minwidth = 0.5f,
      .minheight = 1.f,
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

    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
      },
    };
    return withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
  },
};



