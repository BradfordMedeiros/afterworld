#ifndef MOD_AFTERWORLD_ARCADE_TENNIS
#define MOD_AFTERWORLD_ARCADE_TENNIS

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"

struct Tennis {
	glm::vec2 ballPosition;
	glm::vec2 ballVelocity;
	float leftPaddlePosition;
	float rightPaddlePosition;
	int scorePlayerOne;
	int scorePlayerTwo;
	float gameTime;
	bool isPlaying;

	bool pressingUp;
	bool pressingDown;
	bool controllingRightPaddle;
};

Tennis createTennis();
void onMouseTennis();
void onKeyTennis(int key, int scancode, int action, int mod);

void updateTennis();
void drawTennis();

#endif