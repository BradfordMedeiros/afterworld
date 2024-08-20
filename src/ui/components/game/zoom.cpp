#include "./zoom.h"

Component zoomComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    auto zoomOptions = typeFromProps<ZoomOptions>(props, valueSymbol);
    modassert(zoomOptions, "zoomOptions must be defined for zoomComponent");

    const glm::vec4 backgroundColor(0.f, 0.f, 0.f, 0.6f);
    //drawTools.drawRect(0.75f, 0.f, 0.5f, 2.f, false, backgroundColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    //drawTools.drawRect(-0.75f, 0.f, 0.5f, 2.f, false, backgroundColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    //drawTools.drawRect(0, 0.75f, 1.f, 0.5f, false, backgroundColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    //drawTools.drawRect(0, -0.75f, 1.f, 0.5f, false, backgroundColor, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);

    drawTools.drawRect(0, 0, 1.f, 1.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, "../gameresources/ui/zoom.png", std::nullopt, std::nullopt);
    drawTools.drawText(std::to_string(zoomOptions -> zoomAmount) + "x " + std::string("zoom"), -0.8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);

  	return BoundingBox2D {
      .x = 0.f,
      .y = 0.f,
      .width = 1.f,
      .height = 1.f,
    };
  },
};
