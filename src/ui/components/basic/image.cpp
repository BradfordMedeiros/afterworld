#include "./image.h"

Component imageComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::string* imageName = typeFromProps<std::string>(props, valueSymbol);
    modassert(imageName, "no imageName defined, must have valueSymbol");
    float height = floatFromProp(props, heightSymbol, 1.f);
    float width = floatFromProp(props, widthSymbol, 1.f);

    BoundingBox2D boundingBox {
      .x = 0.f,
      .y = 0.f,
      .width = width,
      .height = height,
    };
    drawTools.drawRect(0.f, 0.f, width, height, false, glm::vec4(1.f, 1.f, 1.f, 0.9f), true, std::nullopt, *imageName, std::nullopt, std::nullopt);

    if (true){
      drawDebugBoundingBox(drawTools, boundingBox, glm::vec4(0.f, 0.f, 0.f, 1.f));
    }
    return boundingBox;
  }
};
