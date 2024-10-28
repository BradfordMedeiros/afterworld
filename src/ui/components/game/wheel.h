#ifndef MOD_AFTERWORLD_COMPONENTS_WHEEL
#define MOD_AFTERWORLD_COMPONENTS_WHEEL

#include "../common.h"
#include "../../../global.h"

#include "../basic/button.h"

struct WheelConfig {
	int numElementsInWheel;
	float wheelRadius;
	int selectedIndex;
	int offset;
	std::vector<std::string> wheelContents;
	std::function<float()> getRotationOffset;
	std::function<void(int)> onClick;
};

extern Component wheelComponent;

#endif

