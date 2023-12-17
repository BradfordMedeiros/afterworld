#include "./checkbox.h"

const float STYLE_UI_CHECKBOX_WIDTH = 0.025f;
const float STYLE_UI_CHECKBOX_HEIGHT = 0.0375f;


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
      .width = STYLE_UI_CHECKBOX_WIDTH,
      .height = STYLE_UI_CHECKBOX_HEIGHT,
    };
    drawTools.drawRect(0.f, 0.f, STYLE_UI_CHECKBOX_WIDTH, STYLE_UI_CHECKBOX_HEIGHT, false, glm::vec4(1.f, 1.f, 1.f, .4f), std::nullopt, true, uniqueMenuItemMappingId(), std::nullopt, std::nullopt);
    
    auto innerMappingId = uniqueMenuItemMappingId();
    if (isChecked){
      drawTools.drawRect(0.f, 0.f, STYLE_UI_CHECKBOX_WIDTH * 0.8, STYLE_UI_CHECKBOX_HEIGHT * 0.8, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt, true, innerMappingId, "./res/scenes/editor/dock/images/checked.png", std::nullopt);
    }else{
      drawTools.drawRect(0.f, 0.f, STYLE_UI_CHECKBOX_WIDTH * 0.8, STYLE_UI_CHECKBOX_HEIGHT * 0.8, false, glm::vec4(0.f, 0.f, 0.f, 0.2f), std::nullopt, true, innerMappingId, std::nullopt, std::nullopt);
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

