#include "./interact.h"

extern CustomApiBindings* gameapi;
extern ArcadeApi arcadeApi;


struct Interact {
};

std::any createInteract(objid id){
	arcadeApi.ensureTexturesLoaded(id, 
	{ 
		 paths::ARCADE_INTERACT_IMAGE,
	});
	return Interact{};
}

void rmInteractInstance(std::any& any){
}

void updateInteract(std::any& any){
}

int currentAmmo = 0;
void drawInteract(std::any& any, std::optional<objid> textureId){
 	gameapi -> drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.2f, 0.2f, 0.2f, 1.f), textureId, true, std::nullopt, paths::ARCADE_INTERACT_IMAGE, std::nullopt);
  drawCenteredText(std::to_string(currentAmmo), 0.f, 0.f, 0.2f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, textureId);
}

void onKeyInteract(std::any& any, int key, int scancode, int action, int mod){
}

void onInteractMouseMove(std::any& any, double xPos, double yPos, float xNdc, float yNdc){
}

void onInteractMouseClick(std::any& any, int button, int action, int mods){
}

void OnInteractMessage(std::any& any){
  DeliverAmmoMessage* ammoMessage = anycast<DeliverAmmoMessage>(any);
  if (ammoMessage){
  	currentAmmo = ammoMessage -> ammo;
  }
}

ArcadeInterface interactGame {
	.createInstance = createInteract,
	.rmInstance = rmInteractInstance,
	.update = updateInteract,
	.draw = drawInteract,
	.onKey = onKeyInteract,
	.onMouseMove = onInteractMouseMove,
	.onMouseClick = onInteractMouseClick,
	.onMessage = OnInteractMessage,
};

