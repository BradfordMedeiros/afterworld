#include "./ball.h"

extern CustomApiBindings* gameapi;

Component ballComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    BallComponentOptions* ballOptions = typeFromProps<BallComponentOptions>(props, valueSymbol);
    modassert(ballOptions, "ballOptions null");
  
    if (ballOptions -> levelComplete.has_value()){
      drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.f, 0.f, 0.f, 0.9f), true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
      drawCenteredTextReal(drawTools, "Level Complete", 0.f, 0.f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
      drawCenteredTextReal(drawTools, "Click to Continue", 0.f, -0.1f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
    }
    if (ballOptions -> elapsedTime.has_value()){
      drawCenteredTextReal(drawTools, std::to_string(ballOptions -> elapsedTime.value()()), 0.f, 0.4f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
    }

    if (ballOptions -> powerupTexture.has_value()){
      drawTools.drawRect(0.8f, 0.8f, 0.2f, 0.2f, false, glm::vec4(1.f, 1.f, 1.f, 0.9f), true, std::nullopt, ballOptions -> powerupTexture.value(), std::nullopt, std::nullopt);
    }

    return BoundingBox2D {
    	.x = 0,
    	.y = 0,
    	.width = 0.f,
    	.height = 0.f,
    };
  },
};
