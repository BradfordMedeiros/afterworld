#include "./slider.h"

void drawRight(DrawingTools& drawTools, float x, float y, float width, float height, glm::vec4 color, objid mappingId){
  drawTools.drawRect(x + (width * 0.5f), y, width, height, false, color, std::nullopt, true, mappingId /*radioButton.mappingId */, std::nullopt);
}


Component sliderInner  {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    float xoffset = floatFromProp(props, xoffsetSymbol, 0.f);
    float yoffset = floatFromProp(props, yoffsetSymbol, 0.f);
    Slider* slider = typeFromProps<Slider>(props, sliderSymbol);
    modassert(slider, "invalid slider prop");

    float width = 0.2f;
    float height = 0.05f;
    float x = xoffset + (width * 0.5f);
    float y = yoffset;
    float left = x;
    float right = x + width;
     
    auto mappingId = uniqueMenuItemMappingId();
    drawRight(drawTools, x, y, width, height, glm::vec4(0.f, 0.f, 0.f, .8f), mappingId);
    drawRight(drawTools, x, y, width * glm::min(1.f, glm::max(slider -> percentage, 0.f)), height, glm::vec4(0.4f, 0.4f, 0.4f, .8f), mappingId);
    auto onSlide = slider -> onSlide;
    drawTools.registerCallbackFns(mappingId, [onSlide, right, left]() -> void {
      float percentage = (getGlobalState().xNdc - left) / (right - left);
      onSlide(percentage * -1.f);
    });

    BoundingBox2D boundingBox {
      .x = x + (width * 0.5f),
      .y = y,
      .width = width,
      .height = height,
    };
    //drawDebugBoundingBox(drawTools, boundingBox);
    return boundingBox;
  },
};
 
Component slider = wrapWithLabel(sliderInner);

