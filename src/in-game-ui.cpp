#include "./in-game-ui.h"

extern CustomApiBindings* gameapi;


std::string ingameUiTextureName(objid id){
	return std::string("gentexture-ingame-ui-texture-test");
}
void createInGamesUiInstance(objid id){
	std::cout << "create in game texture" << std::endl;
	std::string texture = ingameUiTextureName(id);
	auto uiTexture = gameapi -> createTexture(texture, 512, 512, id);
	gameapi -> clearTexture(uiTexture, true, std::nullopt, "../gameresources/textures/controls/up-down.png");

 	GameobjAttributes attr {
 	   .stringAttributes = {
 	     { "texture", texture },
 	   },
 	   .numAttributes = {},
 	   .vecAttr = {
 	     .vec3 = {},
 	     .vec4 = {},
 	   },
 	};
 	gameapi -> setGameObjectAttr(id, attr);  
};

void freeInGameUiInstance(objid id){
	gameapi -> freeTexture(ingameUiTextureName(id), id);
}

struct PosAndRot {
	glm::vec3 position;
	glm::quat rotation;
};
PosAndRot uiPosAndRot(){
 	return PosAndRot {
 		.position = glm::vec3(0.f, 0.f, 0.f),
 		.rotation = glm::identity<glm::quat>(),
 	};
}

void zoomIntoGameUi(){
	auto posAndRot = uiPosAndRot();
	setTempViewpoint(posAndRot.position, posAndRot.rotation);
}