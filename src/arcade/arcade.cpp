#include "./arcade.h"

std::unordered_map<objid, ArcadeInstance> arcadeInstances;

void addArcadeType(objid id, std::string type, std::optional<objid> textureId){
	modassert(arcadeInstances.find(id) == arcadeInstances.end(), "arcade instance already exists");

	if (type == "tennis"){
		arcadeInstances[id] = ArcadeInstance {
			.type = ARCADE_TENNIS,
			.interface = &tennisGame,
			.data = tennisGame.createInstance(),
			.textureId = textureId,
		};
	}else if (type == "invaders"){
		arcadeInstances[id] = ArcadeInstance {
			.type = ARCADE_INVADERS,
			.interface = &invadersGame,
			.data = invadersGame.createInstance(),
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
