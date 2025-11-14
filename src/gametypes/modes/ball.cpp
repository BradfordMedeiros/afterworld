#include "./ball.h"

bool isReloadKey(int button);
bool isTeleportButton(int button);

extern CustomApiBindings* gameapi;

GameTypeInfo getBallMode(){
	GameTypeInfo ballMode = GameTypeInfo {
	  .gametypeName = "ball",
	  .events = { "ball-game" },
	  .createGametype = [](void* data) -> std::any {
			int scoreLimit = 3;
			std::vector<std::string> teamNames = { "red", "blue" };
			std::vector<int> scores = { 0, 0 };
			modassert(teamNames.size() == scores.size(), "team names is not score size");

			BallModeOptions* modeOptions = static_cast<BallModeOptions*>(data);
			std::cout << "ball mode: " << modeOptions -> testNumber << std::endl;
			modeOptions -> setPlayerControl([gameapi]() -> void {
				// This is lame, but whatever
				gameapi -> sendNotifyMessage("ball-game", std::string("start"));
			});
			modeOptions -> changeUi(true);

	    return *modeOptions; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");

	  	std::string* message = std::any_cast<std::string>(&value);
	  	modassert(message, "invalid type ball-mode");
	  	std::cout << "from ball mode: " << event << ", " << *message << std::endl;
	  	if (*message == "start"){
	  	   auto ballId = ballMode -> getBallId().value();
	  	   auto pos = gameapi -> getGameObjectPos(ballId, true, "[gamelogic] get ball position for start");
	  	   ballMode -> initialBallPos = pos;
	  	   ballMode -> ballId = ballId;
 	   	   ballMode -> showTimeElapsed(gameapi -> timeSeconds(true));
	  	}
	  	if (*message == "complete"){
	  		ballMode -> setLevelFinished();
	  	}
	  	if (*message == "reset"){
	  		modassert(ballMode -> initialBallPos.has_value(), "no initial ball position");
	  		gameapi -> setGameObjectPosition(ballMode -> ballId.value(), ballMode -> initialBallPos.value(), true, Hint { .hint = "[gamelogic] - ball set pos" });
	  	}
	   	return false;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string {
	    return std::string("ball mode");
	  },
	  .getScoreInfo = [](std::any& gametype, float startTime) -> std::optional<GametypeData> { return std::nullopt; },
	  .onKey = [](std::any& gametype, int key, int scancode, int action, int mods) -> void {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");
	  	if(isReloadKey(key) && action == 0){
	  		gameapi -> sendNotifyMessage("ball-game", std::string("reset"));
	  	}
	  	if (isTeleportButton(key) && action == 0){
	  		gameapi -> sendNotifyMessage("ball-game", std::string("complete"));
	  	}
	  	std::cout << "ball mode: " << key << ", " << action << std::endl;
	  },
	  .onFrame = [](std::any& gametype) -> void {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");
	  	std::cout << "ball onframe" << std::endl;
	  	if (ballMode -> ballId.has_value()){
	  		auto powerup = ballMode -> getPowerup();
	  		if (powerup.has_value()){
		  		std::cout << "powerup: " << print(powerup.value()) << std::endl;
	  			if (powerup.value() == BIG_JUMP){
	 		  		ballMode -> setPowerupTexture("../gameresources/build/textures/ballgame/jump.png");
	 		  		return;
	  			}else if (powerup.value() == LAUNCH_FORWARD){
	 		  		ballMode -> setPowerupTexture("../gameresources/build/textures/ballgame/dash.png");
	 		  		return;
	  			}else if (powerup.value() == LOW_GRAVITY){
	 		  		ballMode -> setPowerupTexture("../gameresources/build/textures/ballgame/bounce.png");
	 		  		return;
	  			}else if (powerup.value() == REVERSE_GRAVITY){
	 		  		ballMode -> setPowerupTexture("../gameresources/build/textures/ballgame/bounce.normal.png");
	 		  		return;
	  			}else if (powerup.value() == TELEPORT){
	 		  		ballMode -> setPowerupTexture("../gameresources/build/textures/ballgame/teleport.png");
	 		  		return;
	  			}else{
	  				modassert(false, "invalid powerup size ball.cpp");
	  			}
	  		}
	  	}

  		ballMode -> setPowerupTexture("../gameresources/build/textures/ballgame/none.png");


	  },
	};
	return ballMode;
}

void createBallObj(objid sceneId){
  GameobjAttributes attr { 
  	.attr = {
  		{ "vehicle", "ball" },
		{ "texture", "./res/textures/wood.jpg" },
		{ "tint", glm::vec4(1.f, 1.f, 1.f, 0.7f) },
		{ "physics_restitution", 0.5f },
		{ "mesh", "../gameresources/build/primitives/sphere.gltf" },
		{ "physics_mass" , 10.f },
		{ "physics_type", "dynamic" },
		{ "physics_shape", "shape_sphere" },
		{ "physics", "enabled" },
		{ "layer", "transparency" },
		{ "position", glm::vec3(5.68f, -4.98f, 13.6f) }
  	} 
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto ball = gameapi -> makeObjectAttr(sceneId, std::string("ball1"), attr, submodelAttributes);
  modassert(ball.has_value(), "ball was not created");
}


void createLevelObj(objid sceneId, std::string ballLevel){
  GameobjAttributes attr { 
  	.attr = {
		{ "mesh", ballLevel },
		{ "scale", glm::vec3(0.2f, 0.2f, 0.2f) },
		{ "position", glm::vec3(68.32f, -22.1f, 166.6f) },
  	} 
  };

  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  GameobjAttributes submodelAttr { 
  	.attr = {
		{ "physics_restitution", 1.f },
		{ "physics_shape", "shape_exact" },
		{ "physics", "enabled" },
  	} 
  };
  submodelAttributes["obj/entity0_brush9"] = submodelAttr;

  auto level = gameapi -> makeObjectAttr(sceneId, std::string("obj"), attr, submodelAttributes);
  modassert(level.has_value(), "level was not created");

}