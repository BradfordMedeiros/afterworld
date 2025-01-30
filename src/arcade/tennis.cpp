#include "./tennis.h"

extern CustomApiBindings* gameapi;

float offsetPaddle = 0.1f;
float paddleWidth = 0.015f;
float paddleHeight = 0.3f;
float ballSize = 0.05f;
float paddleXLeft = -1.f + offsetPaddle;
float paddleXRight = 1.f - offsetPaddle;
glm::vec2 initialBallVelocity(0.5f, 0.25f);
Tennis createTennis(){
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
	};
}

Tennis tennis = createTennis();


void onMouseTennis(){

}


void resetTennisBall(){
	tennis.ballPosition = glm::vec2(0.f, 0.f);
	tennis.ballVelocity = initialBallVelocity;
}
void onKeyTennis(int key, int scancode, int action, int mod){
	modlog("tennis", std::to_string(key) + " " + std::to_string(action));
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
		resetTennisBall();
	}
}

bool tennisPaddleCanMoveUp(){
	return tennis.leftPaddlePosition < (1.f - (paddleHeight * 0.5f));
}
bool tennisPaddleCanMoveDown(){
	return tennis.leftPaddlePosition > (-1.f + (paddleHeight * 0.5f));
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
bool tennisPaddleHitLeft(){
	return tennisHitsPaddle(glm::vec2(paddleXLeft, tennis.leftPaddlePosition), glm::vec2(paddleWidth, paddleHeight), tennis.ballPosition);
}

bool tennisPaddleHitRight(){
	return tennisHitsPaddle(glm::vec2(paddleXRight, tennis.rightPaddlePosition), glm::vec2(paddleWidth, paddleHeight), tennis.ballPosition);
}

glm::vec2 tennisReflect(glm::vec2 ballVelocity){
	ballVelocity.x *= -1;
 	ballVelocity *= 1.1;
 	return ballVelocity;
}

void updateTennis(){
	float paddleMovementSpeed = 2.f;

	auto paddleHitRight = tennisPaddleHitRight();
	auto paddleHitLeft = tennisPaddleHitLeft();
	if (paddleHitRight){
		if (tennis.ballVelocity.x > 0){
			tennis.ballVelocity = tennisReflect(tennis.ballVelocity);
		}
	}
	if (paddleHitLeft){
		if (tennis.ballVelocity.x < 0){
			tennis.ballVelocity = tennisReflect(tennis.ballVelocity);
		}
	}

	if ((tennis.ballPosition.y + (0.5f * ballSize)) > 1.f){
		if (tennis.ballVelocity.y > 0.f){
			tennis.ballVelocity.y *= -1;
		}
	}
	if ((tennis.ballPosition.y - (0.5f * ballSize)) < -1.f){
		if (tennis.ballVelocity.y < 0.f){
			tennis.ballVelocity.y *= -1;
		}
	}

	tennis.ballPosition.x += tennis.ballVelocity.x * gameapi -> timeElapsed();
	tennis.ballPosition.y += tennis.ballVelocity.y * gameapi -> timeElapsed();

	if (tennis.controllingRightPaddle){
		if (tennis.pressingUp && tennisPaddleCanMoveUp()){
			tennis.rightPaddlePosition += paddleMovementSpeed * gameapi -> timeElapsed();
		}
		if (tennis.pressingDown && tennisPaddleCanMoveDown()){
			tennis.rightPaddlePosition -= paddleMovementSpeed * gameapi -> timeElapsed();
		}		
	}else{
		if (tennis.pressingUp && tennisPaddleCanMoveUp()){
			tennis.leftPaddlePosition += paddleMovementSpeed * gameapi -> timeElapsed();
		}
		if (tennis.pressingDown && tennisPaddleCanMoveDown()){
			tennis.leftPaddlePosition -= paddleMovementSpeed * gameapi -> timeElapsed();
		}		
	}

	bool playerLeftScores = tennis.ballPosition.x > 1.f;
	bool playerRightScores = tennis.ballPosition.x < -1.f;
	if (playerLeftScores || playerRightScores){
		resetTennisBall();
		if(playerLeftScores){
			tennis.scorePlayerOne++;
		}else if(playerRightScores){
			tennis.scorePlayerTwo++;
		}
	}
}

void drawTennis(){
	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt /*textureId*/, true, std::nullopt, "./res/textures/testgradient.png", std::nullopt);

  drawRightText(std::to_string(tennis.scorePlayerOne) + " - " + std::to_string(tennis.scorePlayerTwo), 0.f, 0.8f, 0.04f, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt);

	gameapi -> drawRect(tennis.ballPosition.x, tennis.ballPosition.y, ballSize, ballSize, false, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt /*textureId*/, true, std::nullopt, std::nullopt, std::nullopt);

	gameapi -> drawRect(paddleXLeft, tennis.leftPaddlePosition, paddleWidth, paddleHeight, false, glm::vec4(1.f, 1.f, 0.f, 1.f), std::nullopt /*textureId*/, true, std::nullopt, std::nullopt, std::nullopt);
	gameapi -> drawRect(paddleXRight, tennis.rightPaddlePosition, paddleWidth, paddleHeight, false, glm::vec4(1.f, 1.f, 0.f, 1.f), std::nullopt /*textureId*/, true, std::nullopt, std::nullopt, std::nullopt);

}

