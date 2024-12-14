#include "./loading.h"

extern CustomApiBindings* gameapi;

Component loadingComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	int time = static_cast<int>(gameapi -> timeSeconds(true));
  	auto numDots = time % 3;

  	std::string text = "loading";
  	for (int i = 0; i < numDots; i++){
  		text += ".";
  	}
		drawCenteredTextReal(drawTools, text, 0.f, 0.f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);

    BoundingBox2D boundingBox { .x = 0, .y = 0, .width = 0.f, .height = 0.f };
    return boundingBox;
  },
};

