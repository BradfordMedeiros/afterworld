#ifndef MOD_AFTERWORLD_COMPONENTS_ELEVATOR
#define MOD_AFTERWORLD_COMPONENTS_ELEVATOR

#include "../common.h"
#include "../../../global.h"
#include "../basic/layout.h"
#include "../basic/button.h"

struct ElevatorUiOptions {
	std::function<void()> onClickUp;
	std::function<void()> onClickDown;
};

extern Component elevatorComponent;

#endif

