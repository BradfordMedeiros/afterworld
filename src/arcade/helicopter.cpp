#include "./helicopter.h"

extern CustomApiBindings* gameapi;

struct Helicopter {
	bool gameStarted;
	bool flyUp;
	glm::vec2 helicopterLocation;
};

std::any createHelicopter(){
	Helicopter helicopter {
		.gameStarted = false,
		.flyUp = false,
		.helicopterLocation = glm::vec2(0.f, 0.f),
	};

	return helicopter;
}

void rmHelicopterInstance(std::any& any){

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

bool chopperCrashed(Helicopter& heli){
	return false;
}

void updateHelicopter(std::any& any){
  Helicopter* helicopterPtr = anycast<Helicopter>(any);
  Helicopter& helicopter = *helicopterPtr;

  helicopter.helicopterLocation.x += gameapi -> timeElapsed() * 0.1f; 

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
  }

}

void drawBackground(Helicopter& heli, std::optional<objid> textureId){

	float depthFactor = 0.5f;
	float offsetX = -1 * heli.helicopterLocation.x * depthFactor;

	std::vector<glm::vec2> locations = {
		glm::vec2(-0.5f, 0.f),
		glm::vec2(0.1f, 0.f),
		glm::vec2(0.6f, 0.f),
	};
	for (auto &pos : locations){
		gameapi -> drawRect(pos.x + offsetX, -0.6f, 0.2f, 0.2f, false, glm::vec4(0.f, 1.f, 0.f, 0.4f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}
}

void drawHelicopter(std::any& any, std::optional<objid> textureId){
  Helicopter* helicopterPtr = anycast<Helicopter>(any);

	// draw buildings
	// let's just grid is 10x10
	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.f, 1.f, 0.f, 1.f), textureId, true, std::nullopt, "./res/textures/testgradient.png", std::nullopt);
	gameapi -> drawRect(-0.6f, helicopterPtr -> helicopterLocation.y, 0.1f, 0.1f, false, glm::vec4(0.f, 1.f, 0.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

	drawBackground(*helicopterPtr, textureId);

	for (int i = 0; i < 10; i++){
		gameapi -> drawRect(i * 0.1f + 0.05f, 0.f, 0.1f, 0.1f, false, glm::vec4(i * 0.1f, i * 0.1f, i * 0.1f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}
}

void onHelicopterMouseMove(std::any& any, double xPos, double yPos, float xNdc, float yNdc){
}

ArcadeInterface helicopterGame {
	.createInstance = createHelicopter,
	.rmInstance = rmHelicopterInstance,
	.update = updateHelicopter,
	.draw = drawHelicopter,
	.onKey = onKeyHelicopter,
	.onMouseMove = onHelicopterMouseMove,
};

