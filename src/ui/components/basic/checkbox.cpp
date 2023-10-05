#include "./checkbox.h"

Component checkboxInner {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto isChecked = boolFromProp(props, checkedSymbol, false);
    auto onCheckPtr = typeFromProps<std::function<void(bool)>>(props, onclickSymbol);
    modassert(onCheckPtr, "checkbox onclickSymbol not provided");
    auto onCheck = *onCheckPtr;

    auto measurer = createMeasurer();

    BoundingBox2D checkboxBounding {
      .x = 0.f, 
      .y = 0.f,
      .width = 0.05f,
      .height = 0.075f,
    };
    drawTools.drawRect(0.f, 0.f, 0.05f, 0.075f, false, glm::vec4(1.f, 1.f, 1.f, .4f), std::nullopt, true, uniqueMenuItemMappingId(), std::nullopt, std::nullopt);
    
    auto innerMappingId = uniqueMenuItemMappingId();
    if (isChecked){
      drawTools.drawRect(0.f, 0.f, 0.04f, 0.06f, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt, true, innerMappingId, "./res/scenes/editor/dock/images/checked.png", std::nullopt);
    }else{
      drawTools.drawRect(0.f, 0.f, 0.04f, 0.06f, false, glm::vec4(0.f, 0.f, 0.f, 0.2f), std::nullopt, true, innerMappingId, std::nullopt, std::nullopt);
    }

    auto onCheckValue = [onCheck, isChecked]() -> void {
      onCheck(!isChecked);
    };
    drawTools.registerCallbackFns(innerMappingId, onCheckValue);
    
    measureBoundingBox(measurer, checkboxBounding);
    return measurerToBox(measurer);
  },
};

Component checkbox = wrapWithLabel(checkboxInner);

