#include "./checkbox.h"

Component checkboxInner {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto isChecked = boolFromProp(props, checkedSymbol, false);
    auto checkedTint = isChecked ? glm::vec4(1.f, 1.f, 1.f, 0.4f) : glm::vec4(0.f, 0.f, 1.f, 0.4f);
    auto measurer = createMeasurer();

    BoundingBox2D checkboxBounding {
      .x = 0.f, 
      .y = 0.f,
      .width = 0.05f,
      .height = 0.075f,
    };
    drawTools.drawRect(0.f, 0.f, 0.05f, 0.075f, false, glm::vec4(0.f, 0.f, 0.f, 1.f), std::nullopt, true, uniqueMenuItemMappingId(), std::nullopt);
    drawTools.drawRect(0.f, 0.f, 0.04f, 0.06f, false, checkedTint, std::nullopt, true, uniqueMenuItemMappingId(), std::nullopt);
    measureBoundingBox(measurer, checkboxBounding);
    return measurerToBox(measurer);
  },
};


Component checkbox {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::vector<Component> components;
    auto strValue = strFromProp(props, valueSymbol, "default checkbox");

    auto innerCheckbox = withProps(checkboxInner, props);
    Props childProps {
      .props = {},
    };
    std::function<void()> onClickTest = []() -> void {};
    Props listItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = strValue },
        PropPair { .symbol = onclickSymbol, .value = onClickTest },
        PropPair { .symbol = paddingSymbol, .value = 0.025f },
      },
    };
    auto listItemWithProps = withPropsCopy(listItem, listItemProps);
    components.push_back(listItemWithProps);
    components.push_back(innerCheckbox);
    auto layout = simpleHorizontalLayout(components);
    return layout.draw(drawTools, childProps);
  },
};

