#include "./checkbox.h"

Component checkboxInner {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto isChecked = boolFromProp(props, checkedSymbol, false);
    auto onCheckPtr = typeFromProps<std::function<void(bool)>>(props, onclickSymbol);
    auto STYLE_UI_CHECKBOX_WIDTH = floatFromProp(props, sizeSymbol, 0.05f);

    modassert(onCheckPtr, "checkbox onclickSymbol not provided");
    auto onCheck = *onCheckPtr;

    auto measurer = createMeasurer();

    BoundingBox2D checkboxBounding {
      .x = 0.f, 
      .y = 0.f,
      .width = STYLE_UI_CHECKBOX_WIDTH,
      .height = STYLE_UI_CHECKBOX_WIDTH,
    };
    
    auto innerMappingId = uniqueMenuItemMappingId();
    if (isChecked){
      drawTools.drawRect(0.f, 0.f, STYLE_UI_CHECKBOX_WIDTH * 0.8, STYLE_UI_CHECKBOX_WIDTH * 0.8, false, styles.highlightColor, std::nullopt, true, innerMappingId, "./res/scenes/editor/dock/images/checked.png", std::nullopt, std::nullopt);
    }else{
      drawTools.drawRect(0.f, 0.f, STYLE_UI_CHECKBOX_WIDTH * 0.8, STYLE_UI_CHECKBOX_WIDTH * 0.8, false, styles.thirdColor, std::nullopt, true, innerMappingId, std::nullopt, std::nullopt, std::nullopt);
    }

    auto onCheckValue = [onCheck, isChecked]() -> void {
      onCheck(!isChecked);
    };
    drawTools.registerCallbackFns(innerMappingId, onCheckValue);
    
    measureBoundingBox(measurer, checkboxBounding);

    drawDebugBoundingBox(drawTools, checkboxBounding, styles.debugColor2);
    return measurerToBox(measurer);
  },
};

Component checkbox = wrapWithLabel(checkboxInner, 0.02f);

