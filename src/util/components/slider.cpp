#include "./slider.h"

void drawRight(DrawingTools& drawTools, float x, float y, float width, float height, glm::vec4 color, objid mappingId){
  drawTools.drawRect(x + (width * 0.5f), y, width, height, false, color, std::nullopt, true, mappingId /*radioButton.mappingId */, std::nullopt);
}

const int xoffsetSymbol = getSymbol("xoffset");
const int yoffsetSymbol = getSymbol("yoffset");

Component createSlider(Slider& slider){
  Component sliderSelector  {
    .draw = [&slider](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
      float xoffset = floatFromProp(props, xoffsetSymbol, 0.f);
      float yoffset = floatFromProp(props, yoffsetSymbol, 0.f);

      float width = 0.2f;
      float height = 0.05f;
      float x = xoffset + (width * 0.5f);
      float y = yoffset;
  
      float left = x;
      float right = x + width;
  
      if (slider.update){
        float percentage = (getGlobalState().xNdc - left) / (right - left);
        slider.percentage = percentage;
        slider.update = false;
      }
      std::cout << "radio 1: " << slider.percentage << ", left = " << left << ", right = " << right << ", xndc = " << getGlobalState().xNdc << std::endl;
  
      drawRight(drawTools, x, y, width, height, glm::vec4(0.f, 0.f, 0.f, .8f), slider.mappingId);
      drawRight(drawTools, x, y, width * glm::min(1.f, glm::max(slider.percentage, 0.f)), height, glm::vec4(0.4f, 0.4f, 0.4f, .8f), slider.mappingId );
  
      BoundingBox2D boundingBox {
        .x = x + (width * 0.5f),
        .y = y,
        .width = width,
        .height = height,
      };
  
      drawDebugBoundingBox(drawTools, boundingBox);
      return boundingBox;
    },
    .imMouseSelect = [&slider](std::optional<objid> mappingIdSelected) -> void {
      if (mappingIdSelected.has_value() && mappingIdSelected.value() == slider.mappingId){
        slider.update = true;
      }
    }  
  };
  return sliderSelector;
}

