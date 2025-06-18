#include "./tennis.h"

extern CustomApiBindings* gameapi;
extern ArcadeApi arcadeApi;

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

	std::vector<objid> sounds;
};

float offsetPaddle = 0.1f;
float paddleWidth = 0.015f;
float paddleHeight = 0.3f;
float ballSize = 0.05f;
float paddleXLeft = -1.f + offsetPaddle;
float paddleXRight = 1.f - offsetPaddle;
glm::vec2 initialBallVelocity(0.5f, 0.25f);

std::any createTennis(objid id){
	auto sounds = arcadeApi.ensureSoundsLoaded(id, { paths::TENNIS_HIT_SOUND });
	arcadeApi.ensureTexturesLoaded(id, 
	{ 
		paths::TENNIS_BALL_IMAGE,
	});

	return Tennis {
		.ballPosition = glm::vec2(0.f, 0.f),
		.ballVelocity = initialBallVelocity,
		.leftPaddlePosition = 0.f,
		.rightPaddlePosition = 0.f,
		.scorePlayerOne = 0,
		.scorePlayerTwo = 0,
		.gameTime = 0.f,
		.isPlaying = false,
		.pressingUp = false,
		.pressingDown = false,
		.controllingRightPaddle = false,
		.sounds = sounds,
	};
}

void rmTennisInstance(std::any& any){
	arcadeApi.releaseSounds(101);
}

void resetTennisBall(Tennis& tennis){
	tennis.ballPosition = glm::vec2(0.f, 0.f);
	tennis.ballVelocity = initialBallVelocity;
}

void onKeyTennis(std::any& any, int key, int scancode, int action, int mod){
	modlog("tennis", std::to_string(key) + " " + std::to_string(action));

  Tennis* tennisPtr = anycast<Tennis>(any);
  Tennis& tennis = *tennisPtr;

	if (key == 265){ // up arrow
		if (action == 1){
			tennis.pressingUp = true;
		}else if (action == 0){
			tennis.pressingUp = false;
		}

	}
	if (key == 264){	// down arrow
		if (action == 1){
			tennis.pressingDown = true;
		}else if (action == 0){
			tennis.pressingDown = false;
		}
	}

	if (key == 340 && action == 1){
		tennis.controllingRightPaddle = !tennis.controllingRightPaddle;
	}

	if (key == 82 && action == 1){
		resetTennisBall(tennis);
	}
}

bool tennisPaddleCanMoveUp(float paddlePos){
	return paddlePos < (1.f - (paddleHeight * 0.5f));
}

bool tennisPaddleCanMoveDown(float paddlePos){
	return paddlePos > (-1.f + (paddleHeight * 0.5f));
}

bool tennisHitsPaddle(glm::vec2 paddlePos, glm::vec2 paddleSize, glm::vec2 ballPosition){
	float paddleLeftSide = paddlePos.x - (0.5f * paddleSize.x);
	float paddleRightSide = paddlePos.x + (0.5f * paddleSize.x);
	float paddleTopSide = paddlePos.y + (0.5f * paddleSize.y);
	float paddleBottomSide = paddlePos.y - (0.5f * paddleSize.y);
	bool hitsPaddle =  (
		((ballPosition.x + (0.5f * ballSize)) > paddleLeftSide) && 
		((ballPosition.x - (0.5f * ballSize)) < paddleRightSide) && 
		((ballPosition.y + (0.5f * ballSize)) > paddleBottomSide) && 
		((ballPosition.y - (0.5f * ballSize)) < paddleTopSide));
	if (!hitsPaddle){
		return false;
	}
	return true;
}


bool tennisPaddleHitLeft(Tennis& tennis){
	return tennisHitsPaddle(glm::vec2(paddleXLeft, tennis.leftPaddlePosition), glm::vec2(paddleWidth, paddleHeight), tennis.ballPosition);
}

bool tennisPaddleHitRight(Tennis& tennis){
	return tennisHitsPaddle(glm::vec2(paddleXRight, tennis.rightPaddlePosition), glm::vec2(paddleWidth, paddleHeight), tennis.ballPosition);
}

glm::vec2 tennisReflect(glm::vec2 ballVelocity){
	ballVelocity.x *= -1;
 	ballVelocity *= 1.1;
 	return ballVelocity;
}


void updateTennis(std::any& any){
  Tennis* tennisPtr = anycast<Tennis>(any);
  Tennis& tennis = *tennisPtr;

	float paddleMovementSpeed = 2.f;

	auto paddleHitRight = tennisPaddleHitRight(tennis);
	auto paddleHitLeft = tennisPaddleHitLeft(tennis);
	if (paddleHitRight){
		if (tennis.ballVelocity.x > 0){
			tennis.ballVelocity = tennisReflect(tennis.ballVelocity);
			arcadeApi.playSound(tennis.sounds.at(0));
		}
	}
	if (paddleHitLeft){
		if (tennis.ballVelocity.x < 0){
			tennis.ballVelocity = tennisReflect(tennis.ballVelocity);
			arcadeApi.playSound(tennis.sounds.at(0));
		}
	}

	if ((tennis.ballPosition.y + (0.5f * ballSize)) > 1.f){
		if (tennis.ballVelocity.y > 0.f){
			tennis.ballVelocity.y *= -1;
			arcadeApi.playSound(tennis.sounds.at(0));
		}
	}
	if ((tennis.ballPosition.y - (0.5f * ballSize)) < -1.f){
		if (tennis.ballVelocity.y < 0.f){
			tennis.ballVelocity.y *= -1;
			arcadeApi.playSound(tennis.sounds.at(0));
		}
	}

	tennis.ballPosition.x += tennis.ballVelocity.x * gameapi -> timeElapsed();
	tennis.ballPosition.y += tennis.ballVelocity.y * gameapi -> timeElapsed();

	auto moveRightPaddleUp = tennis.controllingRightPaddle && tennis.pressingUp;
	auto moveRightPaddleDown = tennis.controllingRightPaddle && tennis.pressingDown;
	auto moveLeftPaddleUp = !tennis.controllingRightPaddle && tennis.pressingUp;
	auto moveLeftPaddleDown = !tennis.controllingRightPaddle && tennis.pressingDown;


	// ai code
	if (tennis.ballPosition.y > tennis.rightPaddlePosition){
		moveRightPaddleUp = true;
	}else if (tennis.ballPosition.y < tennis.rightPaddlePosition){
		moveRightPaddleDown = true;
	}
	//


	if(moveRightPaddleUp && tennisPaddleCanMoveUp(tennis.rightPaddlePosition)){
		tennis.rightPaddlePosition += paddleMovementSpeed * gameapi -> timeElapsed();
	}
	if (moveRightPaddleDown && tennisPaddleCanMoveDown(tennis.rightPaddlePosition)){
		tennis.rightPaddlePosition -= paddleMovementSpeed * gameapi -> timeElapsed();
	}
	if (moveLeftPaddleUp && tennisPaddleCanMoveUp(tennis.leftPaddlePosition)){
		tennis.leftPaddlePosition += paddleMovementSpeed * gameapi -> timeElapsed();
	}
	if (moveLeftPaddleDown && tennisPaddleCanMoveDown(tennis.leftPaddlePosition)){
		tennis.leftPaddlePosition -= paddleMovementSpeed * gameapi -> timeElapsed();
	}

	bool playerLeftScores = tennis.ballPosition.x > 1.f;
	bool playerRightScores = tennis.ballPosition.x < -1.f;
	if (playerLeftScores || playerRightScores){
		resetTennisBall(tennis);
		if(playerLeftScores){
			tennis.scorePlayerOne++;
		}else if(playerRightScores){
			tennis.scorePlayerTwo++;
		}
	}
}

void drawTennis(std::any& any, std::optional<objid> textureId){
  Tennis* tennisPtr = anycast<Tennis>(any);
  Tennis& tennis = *tennisPtr;

	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(.1f, .1f, .1f, 1.f), textureId, true, std::nullopt, "./res/textures/hexglow.png", std::nullopt);

  drawRightText(std::to_string(tennis.scorePlayerOne) + " - " + std::to_string(tennis.scorePlayerTwo), 0.f, 0.8f, 0.04f, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, std::nullopt);

	gameapi -> drawRect(tennis.ballPosition.x, tennis.ballPosition.y, ballSize, ballSize, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, paths::TENNIS_BALL_IMAGE, std::nullopt);

	gameapi -> drawRect(paddleXLeft, tennis.leftPaddlePosition, paddleWidth, paddleHeight, false, glm::vec4(0.f, 0.f, 1.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	gameapi -> drawRect(paddleXRight, tennis.rightPaddlePosition, paddleWidth, paddleHeight, false, glm::vec4(0.f, 0.f, 1.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

}


void onTennisMouseMove(std::any&, double xPos, double yPos, float xNdc, float yNdc){

}


void onTennisMouseClick(std::any&, int button, int action, int mods){

}

void OnTennisMessage(std::any&){

}

ArcadeInterface tennisGame {
	.createInstance = createTennis,
	.rmInstance = rmTennisInstance,
	.update = updateTennis,
	.draw = drawTennis,
	.onKey = onKeyTennis,
	.onMouseMove = onTennisMouseMove,
	.onMouseClick = onTennisMouseClick,
	.onMessage = OnTennisMessage,
};

