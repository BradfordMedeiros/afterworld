#include "./slider.h"

void drawRight(DrawingTools& drawTools, float x, float y, float width, float height, glm::vec4 color, objid mappingId){
  drawTools.drawRect(x + (width * 0.5f), y, width, height, false, color, std::nullopt, true, mappingId /*radioButton.mappingId */, std::nullopt, mappingId);
}


Component sliderInner  {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    float xoffset = floatFromProp(props, xoffsetSymbol, 0.f);
    float yoffset = floatFromProp(props, yoffsetSymbol, 0.f);
    glm::vec4* borderColor = typeFromProps<glm::vec4>(props, borderColorSymbol);
    glm::vec4* barColor = typeFromProps<glm::vec4>(props, barColorSymbol);
    Slider* slider = typeFromProps<Slider>(props, sliderSymbol);
    modassert(slider, "invalid slider prop for sliderSymbol");

    float width = 0.2f;
    float height = 0.05f;
    float x = xoffset + (width * 0.5f);
    float y = yoffset;
    float left = x;
    float right = x + width;
     

    auto barMappingId = uniqueMenuItemMappingId();
    drawRight(drawTools, x, y, width * glm::min(1.f, glm::max(slider -> percentage, 0.f)), height, barColor ? *barColor : glm::vec4(0.4f, 0.4f, 0.4f, .8f), barMappingId);

    auto mappingId = uniqueMenuItemMappingId();
    drawRight(drawTools, x, y, width, height, glm::vec4(0.f, 0.f, 0.f, .0f), mappingId);


    auto onSlide = slider -> onSlide;
    drawTools.registerCallbackFnsHandler(mappingId, [onSlide, right, left](HandlerCallbackFn& handler) -> void {
      float actualLeft = handler.trackedLocationData.position.x - (handler.trackedLocationData.size.x * 0.5f);
      float actualRight = handler.trackedLocationData.position.x + (handler.trackedLocationData.size.x * 0.5f);

      std::cout << "slider tracked location: " << print(handler.trackedLocationData) << std::endl;

      float percentage = (getGlobalState().xNdc - actualLeft) / (actualRight - actualLeft);
      std::cout << "slider : percentage: " << percentage << std::endl;
      onSlide(percentage);
    });

    BoundingBox2D boundingBox {
      .x = x + (width * 0.5f),
      .y = y,
      .width = width,
      .height = height,
    };
    if (borderColor){
      drawDebugBoundingBox(drawTools, boundingBox, *borderColor);
    }
    return boundingBox;
  },
};
 
Component slider = wrapWithLabel(sliderInner);

