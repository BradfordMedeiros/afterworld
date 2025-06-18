#include "./rhythm.h"

extern CustomApiBindings* gameapi;
extern ArcadeApi arcadeApi;

struct Rhythm {
	
};

std::any createRhythm(objid id){
	arcadeApi.ensureTexturesLoaded(id, 
	{ 
		paths::RHYTHM_LEFT,
		paths::RHYTHM_RIGHT,
		paths::RHYTHM_UP,
		paths::RHYTHM_DOWN,
	});
	return Rhythm {};
}

void rmRhythmInstance(std::any& any){
}

void updateRhythm(std::any& any){
}


enum RhythmType { RHYTHM_LEFT, RHYTHM_RIGHT, RHYTHM_UP, RHYTHM_DOWN };
struct RhythmControl {
	RhythmType type;
	float time;
};
std::vector<RhythmControl> elementsToRender(float time){
	std::vector<RhythmControl> elements {};
	for (int i = 0; i < 50; i++){
		RhythmType type = RHYTHM_LEFT;
		if (i % 4 == 0){
			type = RHYTHM_LEFT;
		}else if (i % 4 == 1){
			type = RHYTHM_RIGHT;
		}else if (i % 4 == 2){
			type = RHYTHM_UP;
		}else if (i % 4 == 3){
			type = RHYTHM_DOWN;
		}
		elements.push_back(RhythmControl {
			.type = type,
			.time = 0.f,
		});
	}
	return elements;
}

float getArrowCenter(int column, float columnWidth){
	float offset = ((column - 1) * columnWidth) - (columnWidth * 0.5f);
	return offset;
}

struct ArrowRender {
	glm::vec2 position;
	const char* texture;
};
std::vector<ArrowRender> getArrowRendering(float time, float columnWidth){
	std::vector<ArrowRender> arrows;

	for (auto &element : elementsToRender(time)){
		int columnNumber = 0;
		if (element.type == RHYTHM_LEFT){
			columnNumber = 0;
		}else if (element.type == RHYTHM_RIGHT){
			columnNumber = 1;
		}else if (element.type == RHYTHM_UP){
			columnNumber = 2;
		}else if (element.type == RHYTHM_DOWN){
			columnNumber = 3;
		}else{
			modassert(false, "invalid type rhythm column");
		}
		auto positionX = getArrowCenter(columnNumber, columnWidth);
		auto positionY = element.time;

		const char* texture = NULL;
		if (element.type == RHYTHM_LEFT){
			texture = paths::RHYTHM_LEFT;
		}else if (element.type == RHYTHM_RIGHT){
			texture = paths::RHYTHM_RIGHT;
		}else if (element.type == RHYTHM_UP){
			texture = paths::RHYTHM_UP;
		}else if (element.type == RHYTHM_DOWN){
			texture = paths::RHYTHM_DOWN;
		}else{
			modassert(false, "invalid type rhythm");
		}

		arrows.push_back(ArrowRender {
			.position = glm::vec2(positionX, positionY),
			.texture = texture,
		});


	}

	return arrows;
}


void drawRhythm(std::any& any, std::optional<objid> textureId){
	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(.1f, .1f, .1f, 1.f), textureId, true, std::nullopt, "./res/textures/hexglow.png", std::nullopt);
	gameapi -> drawRect(0.f, 0.8f, 2.f, 0.05f, false, glm::vec4(0.f, 0.f, 1.f, 0.2f), textureId, true, std::nullopt, std::nullopt, std::nullopt);

	float columnWidth = 2.f / 4.f;

	std::vector<glm::vec4> colors {
		glm::vec4(1.f, 0.f, 0.f, 0.2f),
		glm::vec4(1.f, 1.f, 0.f, 0.2f),
		glm::vec4(1.f, 0.f, 1.f, 0.2f),
		glm::vec4(0.f, 1.f, 0.f, 0.2f),
	};
	for (int i = 0; i < 4; i++){
		auto offset = getArrowCenter(i, columnWidth);
		gameapi -> drawRect(offset, 0.f, columnWidth, 0.2f, false, colors.at(i), textureId, true, std::nullopt, std::nullopt, std::nullopt);
	}

	auto time = gameapi -> timeSeconds(false);
	float arrowSize = 0.1f;
	for (auto &element : getArrowRendering(time, columnWidth)){
		gameapi -> drawRect(element.position.x, element.position.y, arrowSize, arrowSize, false, glm::vec4(1.f, 1.f, 1.f, 1.f), textureId, true, std::nullopt, element.texture, std::nullopt);
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

