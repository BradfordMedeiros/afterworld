#include "./wheel.h"

extern CustomApiBindings* gameapi;

struct WheelConfig {
	int numElementsInWheel;
	int numElementsToShow;
	float wheelRadius;
	int selectedIndex;
	int offset;
	std::vector<std::string> wheelContents;
	std::function<float()> getRotationOffset;
	std::function<void(int)> onClick;
};

WheelConfig wheelConfig {
	.numElementsInWheel = 10,
	.numElementsToShow = 5,
	.wheelRadius = 0.5f,
	.selectedIndex = 3,
	.offset = 2,
	.wheelContents = {
		"a1 Target Hunt",
		"a2 Maze",
		"a3 Race Stalker",
		"a4 Placeholder Game",
		"a5 Target Hunt 2",
		"a6 Vortex Vault",
		"b1 Target Hunt",
		"b2 Maze",
		"b3 Race Stalker",
		"b4 Placeholder Game",
		"b5 Target Hunt 2",
		"b6 Vortex Vault",
		"c1 Target Hunt",
		"c2 Maze",
		"c3 Race Stalker",
		"c4 Placeholder Game",
		"c5 Target Hunt 2",
		"c6 Vortex Vault",
		"d1 Target Hunt",
		"d2 Maze",
		"d3 Race Stalker",
		"d4 Placeholder Game",
		"d5 Target Hunt 2",
		"d6 Vortex Vault",
	},
	.getRotationOffset = []() -> float {
	 	auto timeOffset = gameapi -> timeSeconds(false) * 0.2f;
	 	return timeOffset;
	},
	.onClick = [](int) -> void {
		wheelConfig.selectedIndex++;
	},
};

Component wheelComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	for (int i = 0; i < wheelConfig.numElementsInWheel; i++){
  		if ((i + 1) > wheelConfig.numElementsToShow){
  			break;
			}
  		float value = wheelConfig.getRotationOffset() + i * (2 * M_PI) / wheelConfig.numElementsInWheel;
  		float midpointX = glm::cos(value) * wheelConfig.wheelRadius;
  		float midpointY = glm::sin(value) * wheelConfig.wheelRadius;

  		auto index = i + wheelConfig.offset;
	    auto textContent = wheelConfig.wheelContents.at(index % wheelConfig.wheelContents.size());
		  drawCenteredTextReal(drawTools, textContent, midpointX, midpointY, 0.02f, (index == wheelConfig.selectedIndex) ? glm::vec4(0.f, 0.f, 1.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 0.8f), std::nullopt);
  	}

  	return BoundingBox2D {
  		.x = 0.f,
  		.y = 0.f,
  		.width = 1.f,
  		.height = 1.f,
  	};
  },
};
