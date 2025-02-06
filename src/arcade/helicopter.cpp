#include "./helicopter.h"

extern CustomApiBindings* gameapi;
extern ArcadeApi arcadeApi;

struct Helicopter {
	bool gameStarted;
	bool flyUp;
	glm::vec2 helicopterLocation;
	glm::vec2 helicopterSize;
	std::vector<objid> sounds;
};
float helicopterDisplayOffset = -0.6f;

std::any createHelicopter(objid id){
	auto sounds = arcadeApi.ensureSoundsLoaded(id, { "../gameresources/sound/rain.wav" });

	arcadeApi.ensureTexturesLoaded(id, { "../gameresources/textures/lava.png" });
	Helicopter helicopter {
		.gameStarted = false,
		.flyUp = false,
		.helicopterLocation = glm::vec2(0.f, 0.f),
		.helicopterSize = glm::vec2(0.1f, 0.1f),
		.sounds = sounds,
	};

	arcadeApi.playSound(sounds.at(0));

	return helicopter;
}

void rmHelicopterInstance(std::any& any){
	arcadeApi.releaseSounds(101);
	arcadeApi.releaseTextures(101);
}

void onKeyHelicopter(std::any& any, int key, int scancode, int action, int mod){
  Helicopter* helicopterPtr = anycast<Helicopter>(any);
  Helicopter& helicopter = *helicopterPtr;

	if (key == 'L'){
		if (action == 1){
			helicopter.flyUp = true;
		}else if (action == 0){
			helicopter.flyUp = false;
		}
	}
}

struct Obstacle {
	glm::vec2 position;
	glm::vec2 size;
};

std::vector<Obstacle> getLocations(glm::vec2 helicopterLocation){
	// every 10 units repeat
	int level = static_cast<int>(helicopterLocation.x) / 5;;
	std::cout << "helicopterLocation: " << helicopterLocation.x << ", level = " << level << std::endl;

	std::vector<Obstacle> locations = {
		Obstacle { .position = glm::vec2(1.f + (5.f * level) + 1.f, 0.f), .size = glm::vec2(0.2f, 0.2f) },
		Obstacle { .position = glm::vec2(1.f + (5.f * level) + 1.5f, 0.4f), .size = glm::vec2(0.2f, 0.2f) },
		Obstacle { .position = glm::vec2(1.f + (5.f * level) + 2.5f, 0.4f), .size = glm::vec2(0.2f, 0.2f) },
		Obstacle { .position = glm::vec2(1.f + (5.f * level) + 3.5f, 0.2f), .size = glm::vec2(0.2f, 0.2f) },
	};
	return locations;
}

// AABB check
bool checkOverlap(glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2){
	auto firstLeft = pos1.x - (0.5f * size1.x);
	auto firstRight = pos1.x + (0.5f * size1.x);
	auto firstUp = pos1.y + (0.5f * size1.y);
	auto firstDown = pos1.y - (0.5f * size1.y);

	auto secLeft = pos2.x - (0.5f * size2.x);
	auto secRight = pos2.x + (0.5f * size2.x);
	auto secUp = pos2.y + (0.5f * size2.y);
	auto secDown = pos2.y - (0.5f * size2.y);

	bool overlapsX = (firstLeft >= secLeft && firstLeft <= secRight) || (firstRight >= secLeft && firstRight <= secRight);
	bool overlapsY = (firstUp >= secDown && firstUp <= secUp) || (firstDown >= secDown && firstDown <= secUp);
	return overlapsX && overlapsY;
}
bool chopperCrashed(Helicopter& heli){
	for (auto &location : getLocations(heli.helicopterLocation)){
		bool collides = checkOverlap(heli.helicopterLocation, heli.helicopterSize, location.position, location.size);
		if (collides){
			//std::cout << print(heli.helicopterLocation) << ", " << print(heli.helicopterSize) << ", " << print(location.position) << ", " << print(location.size) << std::endl;
			return true;
		}
	}
	return false;
}

void updateHelicopter(std::any& any){
  Helicopter* helicopterPtr = anycast<Helicopter>(any);
  Helicopter& helicopter = *helicopterPtr;

  helicopter.helicopterLocation.x += gameapi -> timeElapsed() * 0.5f; 

	if(helicopter.flyUp){
		auto fallAmount = 1 * gameapi -> timeElapsed() * 1.4f;
	  helicopterPtr -> helicopterLocation.y += fallAmount;
	}else{
		auto fallAmount = -1 * gameapi -> timeElapsed() * 0.4f;
	  helicopterPtr -> helicopterLocation.y += fallAmount;
	}

	if (helicopterPtr -> helicopterLocation.y < -0.8f){
		helicopterPtr -> helicopterLocation.y = -0.8f;
	}

  if (chopperCrashed(*helicopterPtr)){
  	helicopterPtr -> gameStarted = false;
  	helicopterPtr -> helicopterLocation = glm::vec2(0.f, 0.f);
  }

}

void drawObstacles(Helicopter& heli, std::optional<objid> textureId){
	float offsetX = -1 * (heli.helicopterLocation.x - helicopterDisplayOffset);
	for (auto &obstacle : getLocations(heli.helicopterLocation)){
		gameapi -> drawRect(obstacle.position.x + offsetX, obstacle.position.y, obstacle.size.x, obstacle.size.y, false, glm::vec4(1.f, 0.f, 0.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}
}

void drawHelicopter(std::any& any, std::optional<objid> textureId){
  Helicopter* helicopterPtr = anycast<Helicopter>(any);

	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.f, 1.f, 0.f, 1.f), textureId, true, std::nullopt, "../gameresources/textures/lava.png", std::nullopt);
	gameapi -> drawRect(helicopterDisplayOffset, helicopterPtr -> helicopterLocation.y, helicopterPtr -> helicopterSize.x, helicopterPtr -> helicopterSize.x, false, glm::vec4(0.f, 1.f, 0.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

	drawObstacles(*helicopterPtr, textureId);
}

void onHelicopterMouseMove(std::any& any, double xPos, double yPos, float xNdc, float yNdc){
}

void onHelicopterMouseClick(std::any&, int button, int action, int mods){

}

ArcadeInterface helicopterGame {
	.createInstance = createHelicopter,
	.rmInstance = rmHelicopterInstance,
	.update = updateHelicopter,
	.draw = drawHelicopter,
	.onKey = onKeyHelicopter,
	.onMouseMove = onHelicopterMouseMove,
	.onMouseClick = onHelicopterMouseClick,
};

