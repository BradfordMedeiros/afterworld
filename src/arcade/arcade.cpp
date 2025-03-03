#include "./arcade.h"

extern ArcadeApi arcadeApi;

extern std::unordered_map<objid, ArcadeInstance> arcadeInstances; // static-state extern

void addArcadeType(objid id, std::string type, std::optional<objid> textureId){
	modassert(arcadeInstances.find(id) == arcadeInstances.end(), "arcade instance already exists");

	if (type == "tennis"){
		arcadeInstances[id] = ArcadeInstance {
			.type = ARCADE_TENNIS,
			.interface = &tennisGame,
			.data = tennisGame.createInstance(id),
			.textureId = textureId,
		};
	}else if (type == "invaders"){
		arcadeInstances[id] = ArcadeInstance {
			.type = ARCADE_INVADERS,
			.interface = &invadersGame,
			.data = invadersGame.createInstance(id),
			.textureId = textureId,
		};
	}else if (type == "helicopter"){
		arcadeInstances[id] = ArcadeInstance {
			.type = ARCADE_HELICOPTER,
			.interface = &helicopterGame,
			.data = helicopterGame.createInstance(id),
			.textureId = textureId,
		};
	}else if (type == "rhythm"){
		arcadeInstances[id] = ArcadeInstance {
			.type = ARCADE_RHYTHM,
			.interface = &rhythmGame,
			.data = rhythmGame.createInstance(id),
			.textureId = textureId,
		};
	}else if (type == "interact"){
		arcadeInstances[id] = ArcadeInstance {
			.type = ARCADE_INTERACT,
			.interface = &interactGame,
			.data = interactGame.createInstance(id),
			.textureId = textureId,
		};
	}else{
		modassert(false, "invalid type");
	}

}

void maybeRemoveArcadeType(objid id){
	if (arcadeInstances.find(id) != arcadeInstances.end()){
		ArcadeInstance& instance = arcadeInstances.at(id);
		instance.interface -> rmInstance(instance.data);
	}
	arcadeInstances.erase(id);
}

std::optional<objid> arcadeTextureId(objid id){
	if (arcadeInstances.find(id) == arcadeInstances.end()){
		return std::nullopt;
	}
	return arcadeInstances.at(id).textureId;
}

void onKeyArcade(int key, int scancode, int action, int mod){
	for (auto &[id, arcade] : arcadeInstances){
		arcade.interface -> onKey(arcade.data, key, scancode, action, mod);
	}
}

void onMouseMoveArcade(double xPos, double yPos, float xNdc, float yNdc){
	for (auto &[id, arcade] : arcadeInstances){
		arcade.interface -> onMouseMove(arcade.data, xPos, yPos, xNdc, yNdc);
	}	
}

void onMouseClickArcade(int button, int action, int mods){
	for (auto &[id, arcade] : arcadeInstances){
		arcade.interface -> onMouseClick(arcade.data, button, action, mods);
	}	
}

void onMessageArcade(std::any& message){
	for (auto &[id, arcade] : arcadeInstances){
		arcade.interface -> onMessage(message);
	}
}

void updateArcade(){
	for (auto &[id, arcade] : arcadeInstances){
		arcade.interface -> update(arcade.data);
	}
}

void drawArcade(){
	for (auto &[id, arcade] : arcadeInstances){
		arcade.interface -> draw(arcade.data, arcade.textureId);
	}
}
