#ifndef MOD_AFTERWORLD_COMPONENTS_BALL
#define MOD_AFTERWORLD_COMPONENTS_BALL

#include "../components/common.h"

struct BallComponentOptions {
	std::optional<float> startTime;
	std::optional<std::string> winMessage;
	std::optional<std::string> powerupTexture;
};
extern Component ballComponent;

#endif