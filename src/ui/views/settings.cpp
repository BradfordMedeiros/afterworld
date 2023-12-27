#include "./settings.h"

Component settingsComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    float width = 2.f;
    float height = 2.f;
    BoundingBox2D boundingBox {
      .x = 0.f,
      .y = 0.f,
      .width = width,
      .height = height,
    };
    drawTools.drawRect(0.f, 0.f, width, height, false, glm::vec4(0.f, 0.f, 1.f, 0.9f), std::nullopt, true, std::nullopt, "./res/textures/concepts/settings.png", std::nullopt);

    if (true){
      drawDebugBoundingBox(drawTools, boundingBox, glm::vec4(0.f, 0.f, 0.f, 1.f));
    }
    return boundingBox;
  }
};