#include "./rhythm.h"

extern CustomApiBindings* gameapi;
extern ArcadeApi arcadeApi;

struct Rhythm {
	
};

std::any createRhythm(objid id){
	return Rhythm {
	};
}

void rmRhythmInstance(std::any& any){
}

void updateRhythm(std::any& any){
}

struct RhythmControl {
	glm::vec2 position;
};



std::vector<RhythmControl> elementsToRender(float time){
	std::vector<RhythmControl> elements {
		RhythmControl {
			.position = glm::vec2(0.2f, 0.2f + (time * 0.1f)),
		},
		RhythmControl {
			.position = glm::vec2(0.2f, 0.6f  + (time * 0.1f)),
		},
	};
	return elements;
}

void drawRhythm(std::any& any, std::optional<objid> textureId){
	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(.1f, .1f, .1f, 1.f), textureId, true, std::nullopt, "./res/textures/hexglow.png", std::nullopt);
	gameapi -> drawRect(0.f, 0.8f, 2.f, 0.05f, false, glm::vec4(0.f, 0.f, 1.f, 0.2f), textureId, true, std::nullopt, std::nullopt, std::nullopt);


	auto time = gameapi -> timeSeconds(false);
	for (auto &element : elementsToRender(time)){
		gameapi -> drawRect(element.position.x, element.position.y, 0.05f, 0.05f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}

}

void onKeyRhythm(std::any& any, int key, int scancode, int action, int mod){
}


void onRhythmMouseMove(std::any&, double xPos, double yPos, float xNdc, float yNdc){

}


void onRhythmMouseClick(std::any&, int button, int action, int mods){

}

void OnRhythmMessage(std::any&){

}

ArcadeInterface rhythmGame {
	.createInstance = createRhythm,
	.rmInstance = rmRhythmInstance,
	.update = updateRhythm,
	.draw = drawRhythm,
	.onKey = onKeyRhythm,
	.onMouseMove = onRhythmMouseMove,
	.onMouseClick = onRhythmMouseClick,
	.onMessage = OnRhythmMessage,
};

