#include "./platformer.h"

extern CustomApiBindings* gameapi;

struct PlatformerPlatform {
	glm::vec2 position;
	glm::vec2 size;
};

struct PlatformerCoin {
	glm::vec2 position;
	bool collected;
};

enum PlatformerState { PLATFORMER_TITLE, PLATFORMER_PLAYING, PLATFORMER_WON, PLATFORMER_GAME_OVER };

struct Platformer {
	PlatformerState state;
	glm::vec2 playerPosition;
	glm::vec2 playerVelocity;
	bool moveLeft;
	bool moveRight;
	bool jumpPressed;
	bool grounded;
	int lives;
	int score;
	float cameraLeft;
	std::vector<PlatformerPlatform> platforms;
	std::vector<PlatformerCoin> coins;
};

const float platformerPlayerWidth = 0.14f;
const float platformerPlayerHeight = 0.18f;
const float platformerWorldWidth = 18.f;
const float platformerViewWidth = 2.f;

std::vector<PlatformerPlatform> createPlatformerPlatforms(){
	return {
		{ glm::vec2(1.5f, -0.92f), glm::vec2(5.f, 0.16f) },
		{ glm::vec2(5.4f, -0.92f), glm::vec2(2.f, 0.16f) },
		{ glm::vec2(7.5f, -0.62f), glm::vec2(1.2f, 0.16f) },
		{ glm::vec2(9.4f, -0.92f), glm::vec2(2.f, 0.16f) },
		{ glm::vec2(11.6f, -0.62f), glm::vec2(1.4f, 0.16f) },
		{ glm::vec2(13.3f, -0.92f), glm::vec2(1.8f, 0.16f) },
		{ glm::vec2(15.7f, -0.68f), glm::vec2(1.2f, 0.16f) },
		{ glm::vec2(17.2f, -0.92f), glm::vec2(2.f, 0.16f) },
	};
}

std::vector<PlatformerCoin> createPlatformerCoins(){
	return {
		{ glm::vec2(2.2f, -0.48f), false },
		{ glm::vec2(5.4f, -0.42f), false },
		{ glm::vec2(7.5f, -0.18f), false },
		{ glm::vec2(9.5f, -0.48f), false },
		{ glm::vec2(11.6f, 0.02f), false },
		{ glm::vec2(13.3f, -0.42f), false },
		{ glm::vec2(15.7f, -0.24f), false },
	};
}

void resetPlatformer(Platformer& game){
	game.state = PLATFORMER_TITLE;
	game.playerPosition = glm::vec2(0.25f, -0.72f);
	game.playerVelocity = glm::vec2(0.f, 0.f);
	game.moveLeft = false;
	game.moveRight = false;
	game.jumpPressed = false;
	game.grounded = true;
	game.lives = 3;
	game.score = 0;
	game.cameraLeft = 0.f;
	game.platforms = createPlatformerPlatforms();
	game.coins = createPlatformerCoins();
}

std::any createPlatformer(objid){
	Platformer game;
	resetPlatformer(game);
	return game;
}

void losePlatformerLife(Platformer& game){
	game.lives--;
	game.playerPosition = glm::vec2(0.25f, -0.72f);
	game.playerVelocity = glm::vec2(0.f, 0.f);
	game.cameraLeft = 0.f;
	game.grounded = true;
	game.moveLeft = false;
	game.moveRight = false;
	game.jumpPressed = false;
	if (game.lives <= 0){
		game.state = PLATFORMER_GAME_OVER;
	}
}

bool platformerOverlaps(float center1, float size1, float center2, float size2){
	return std::abs(center1 - center2) < (size1 + size2) * 0.5f;
}

void updatePlatformer(std::any& any){
	Platformer& game = *anycast<Platformer>(any);
	if (game.state != PLATFORMER_PLAYING){
		return;
	}

	const float elapsed = std::min(static_cast<float>(gameapi -> timeElapsed()), 0.05f);
	const float moveSpeed = 2.7f;
	const float gravity = 6.f;
	const float jumpSpeed = 2.75f;

	game.playerVelocity.x = game.moveRight == game.moveLeft ? 0.f : (game.moveRight ? moveSpeed : -moveSpeed);
	float nextX = game.playerPosition.x + game.playerVelocity.x * elapsed;
	nextX = std::max(platformerPlayerWidth * 0.5f, std::min(nextX, platformerWorldWidth - platformerPlayerWidth * 0.5f));
	for (const auto& platform : game.platforms){
		if (platformerOverlaps(game.playerPosition.y, platformerPlayerHeight, platform.position.y, platform.size.y)
			&& platformerOverlaps(nextX, platformerPlayerWidth, platform.position.x, platform.size.x)){
			nextX = game.playerPosition.x;
			break;
		}
	}
	game.playerPosition.x = nextX;

	if (game.jumpPressed && game.grounded){
		game.playerVelocity.y = jumpSpeed;
		game.grounded = false;
	}
	game.jumpPressed = false;

	float nextY = game.playerPosition.y + game.playerVelocity.y * elapsed - 0.5f * gravity * elapsed * elapsed;
	game.playerVelocity.y -= gravity * elapsed;
	game.grounded = false;
	for (const auto& platform : game.platforms){
		if (!platformerOverlaps(game.playerPosition.x, platformerPlayerWidth, platform.position.x, platform.size.x)){
			continue;
		}

		const float platformTop = platform.position.y + platform.size.y * 0.5f;
		const float platformBottom = platform.position.y - platform.size.y * 0.5f;
		const float previousBottom = game.playerPosition.y - platformerPlayerHeight * 0.5f;
		const float previousTop = game.playerPosition.y + platformerPlayerHeight * 0.5f;
		const float nextBottom = nextY - platformerPlayerHeight * 0.5f;
		const float nextTop = nextY + platformerPlayerHeight * 0.5f;

		if (game.playerVelocity.y <= 0.f && previousBottom >= platformTop && nextBottom <= platformTop){
			nextY = platformTop + platformerPlayerHeight * 0.5f;
			game.playerVelocity.y = 0.f;
			game.grounded = true;
		}else if (game.playerVelocity.y > 0.f && previousTop <= platformBottom && nextTop >= platformBottom){
			nextY = platformBottom - platformerPlayerHeight * 0.5f;
			game.playerVelocity.y = 0.f;
		}
	}
	game.playerPosition.y = nextY;

	if (game.playerPosition.y < -1.5f){
		losePlatformerLife(game);
		return;
	}

	for (auto& coin : game.coins){
		if (!coin.collected
			&& platformerOverlaps(game.playerPosition.x, platformerPlayerWidth, coin.position.x, 0.13f)
			&& platformerOverlaps(game.playerPosition.y, platformerPlayerHeight, coin.position.y, 0.16f)){
			coin.collected = true;
			game.score += 100;
		}
	}

	if (game.playerPosition.x >= 17.15f){
		game.state = PLATFORMER_WON;
	}

	game.cameraLeft = std::max(0.f, std::min(game.playerPosition.x - 0.7f, platformerWorldWidth - platformerViewWidth));
}

float platformerScreenX(const Platformer& game, float worldX){
	return worldX - game.cameraLeft - platformerViewWidth * 0.5f;
}

void drawPlatformer(std::any& any, std::optional<objid> textureId){
	const Platformer& game = *anycast<Platformer>(any);
	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.28f, 0.69f, 0.91f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

	for (const auto& platform : game.platforms){
		const float screenX = platformerScreenX(game, platform.position.x);
		if (screenX + platform.size.x * 0.5f < -1.f || screenX - platform.size.x * 0.5f > 1.f){
			continue;
		}
		gameapi -> drawRect(screenX, platform.position.y, platform.size.x, platform.size.y, false, glm::vec4(0.48f, 0.28f, 0.13f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
		gameapi -> drawRect(screenX, platform.position.y + platform.size.y * 0.36f, platform.size.x, platform.size.y * 0.28f, false, glm::vec4(0.25f, 0.75f, 0.25f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}

	for (const auto& coin : game.coins){
		if (!coin.collected){
			gameapi -> drawRect(platformerScreenX(game, coin.position.x), coin.position.y, 0.1f, 0.14f, false, glm::vec4(1.f, 0.82f, 0.12f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
		}
	}

	const float goalX = platformerScreenX(game, 17.15f);
	gameapi -> drawRect(goalX, -0.27f, 0.035f, 1.1f, false, glm::vec4(0.95f, 0.95f, 0.9f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	gameapi -> drawRect(goalX + 0.13f, 0.15f, 0.25f, 0.18f, false, glm::vec4(1.f, 0.28f, 0.28f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

	const float playerX = platformerScreenX(game, game.playerPosition.x);
	gameapi -> drawRect(playerX, game.playerPosition.y, platformerPlayerWidth, platformerPlayerHeight, false, glm::vec4(0.88f, 0.24f, 0.16f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	gameapi -> drawRect(playerX + 0.025f, game.playerPosition.y + 0.025f, 0.025f, 0.025f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

	drawRightText(std::string("COINS ") + std::to_string(game.score / 100) + "   LIVES " + std::to_string(game.lives), -0.95f, 0.91f, 0.045f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, textureId);

	if (game.state == PLATFORMER_TITLE){
		drawCenteredText("PIXEL PATH", 0.f, 0.42f, 0.12f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, textureId);
		drawCenteredText("Press SPACE to start", 0.f, 0.18f, 0.06f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, textureId);
		drawCenteredText("Move with A/D or arrows; jump with SPACE or UP", 0.f, 0.05f, 0.035f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, textureId);
	}else if (game.state == PLATFORMER_WON){
		drawCenteredText("LEVEL COMPLETE!", 0.f, 0.45f, 0.11f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, textureId);
		drawCenteredText("Press SPACE to play again", 0.f, 0.25f, 0.06f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, textureId);
	}else if (game.state == PLATFORMER_GAME_OVER){
		drawCenteredText("GAME OVER", 0.f, 0.45f, 0.11f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, textureId);
		drawCenteredText("Press SPACE to try again", 0.f, 0.25f, 0.06f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, textureId);
	}
}

void onKeyPlatformer(std::any& any, int key, int, int action, int){
	Platformer& game = *anycast<Platformer>(any);
	if (action == 1 && (game.state == PLATFORMER_TITLE || game.state == PLATFORMER_WON || game.state == PLATFORMER_GAME_OVER)){
		if (key == 32 || key == 265 || key == 87){
			resetPlatformer(game);
			game.state = PLATFORMER_PLAYING;
			return;
		}
	}

	if (key == 65 || key == 263){
		game.moveLeft = action == 1;
	}else if (key == 68 || key == 262){
		game.moveRight = action == 1;
	}else if (key == 32 || key == 265 || key == 87){
		if (action == 1){
			game.jumpPressed = true;
		}
	}
}

void onPlatformerMouseClick(std::any& any, int button, int action, int){
	Platformer& game = *anycast<Platformer>(any);
	if (button == 0 && action == 0 && game.state != PLATFORMER_PLAYING){
		resetPlatformer(game);
		game.state = PLATFORMER_PLAYING;
	}
}

void onPlatformerMouseMove(std::any&, double, double, float, float){
}

void onPlatformerMessage(std::any&){
}

ArcadeInterface platformerGame {
	.createInstance = createPlatformer,
	.rmInstance = [](std::any&){},
	.update = updatePlatformer,
	.draw = drawPlatformer,
	.onKey = onKeyPlatformer,
	.onMouseMove = onPlatformerMouseMove,
	.onMouseClick = onPlatformerMouseClick,
	.onMessage = onPlatformerMessage,
};
