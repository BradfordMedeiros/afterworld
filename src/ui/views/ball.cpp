#include "./ball.h"

extern CustomApiBindings* gameapi;

Component ballComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    BallComponentOptions* ballOptions = typeFromProps<BallComponentOptions>(props, valueSymbol);
    modassert(ballOptions, "ballOptions null");
  
    if (ballOptions -> winMessage.has_value()){
      drawCenteredTextReal(drawTools, ballOptions -> winMessage.value(), 0.f, 0.f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
    }
    if (ballOptions -> startTime.has_value()){
      auto elapsedTime = gameapi -> timeSeconds(true) - ballOptions -> startTime.value();
      drawCenteredTextReal(drawTools, std::to_string(elapsedTime), 0.f, 0.4f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);
    }
  	//drawCenteredTextReal(drawTools, ballOptions -> text, 0.f, 0.f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);

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
