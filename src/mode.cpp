#include "./mode.h"

extern CustomApiBindings* gameapi;

extern GameTypes gametypeSystem;
extern std::optional<std::string> activeLevel;
extern Vehicles vehicles;

void setCanExitVehicle(bool canExit);
void enterVehicleRaw(int playerIndex, objid vehicleId, objid id);
void goToLevel(std::string levelShortName);
bool isReloadKey(int button);

void setBallLevelComplete(){
	changeGameType(gametypeSystem, NULL, NULL);
	markLevelComplete(activeLevel.value(), true);
	goToLevel("ballselect");
}

void createBallObj(objid sceneId, glm::vec3 position){
  GameobjAttributes attr { 
  	.attr = {
  		{ "vehicle", "ball" },
			{ "texture", "./res/textures/wood.jpg" },
			{ "tint", glm::vec4(1.f, 1.f, 1.f, 0.1f) },
			{ "physics_restitution", 0.5f },
			{ "mesh", "../gameresources/build/primitives/sphere.gltf" },
			{ "physics_mass" , 10.f },
			{ "physics_type", "dynamic" },
			{ "physics_shape", "shape_sphere" },
			{ "physics", "enabled" },
			{ "position", position},
			{ "layer", "transparency" },
  	} 
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto ball = gameapi -> makeObjectAttr(sceneId, std::string("ball1"), attr, submodelAttributes);
  modassert(ball.has_value(), "ball was not created");

  GameobjAttributes emitterAttr { 
  	.attr = {
 			{ "effekseer", "./res/particles/spirit.efkefc" },
 			{ "state", "enabled" },
  	} 
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributesEmitter;
  auto ballSpirit = gameapi -> makeObjectAttr(sceneId, std::string("+ballSpirit"), emitterAttr, submodelAttributesEmitter);
  modassert(ballSpirit.has_value(), "ballSpirit was not created");

  gameapi -> makeParent(ballSpirit.value(), ball.value());
}

std::optional<objid> getBallId(){
	  auto vehicles = getVehicleIds();
 	  modassert(vehicles.size() == 1, "invalid expected vehicle size");
	  return vehicles.at(0);
}

void changeUi(bool showBallUi){
		if (showBallUi){
			setShowBallOptions(
				BallComponentOptions {
					//.winMessage = "you win congrats",
					.powerupTexture = "../gameresources/build/textures/ballgame/jump.png",
				}
			);
		}else{
			setShowBallOptions(std::nullopt);
		}
}

void showTimeElapsed(std::optional<float> startTime){
		if (!showBallOptions().has_value()){
			return;
		}
		auto ballOptions = showBallOptions().value();
		ballOptions.startTime = startTime;
		setShowBallOptions(ballOptions);
}

void setPowerupTexture(std::string texture){
	auto ballOptions = showBallOptions();
	if (!ballOptions.has_value()){
		return;
	}
	if (ballOptions.has_value()){
		auto newBallOptions = ballOptions.value();
		newBallOptions.powerupTexture = texture;;
		setShowBallOptions(newBallOptions);
	}
}

std::optional<BallPowerup> getPowerup(){
	auto activePlayer = getActivePlayerId(0);
	auto vehicleIds = getVehicleIds();
	if (vehicleIds.size() == 0){
		return std::nullopt;
	}
	return getBallPowerup(vehicles, vehicleIds.at(0));
} 


void startBallMode(objid sceneId){
	auto playerSpawnId = findObjByShortName("playerspawn", std::nullopt);
	auto position = gameapi -> getGameObjectPos(playerSpawnId.value(), true, "[gamelogic] ball - get playerspawn position");

	createBallObj(sceneId, position);

	BallModeOptions modeOptions {};

	changeGameType(gametypeSystem, "ball", &modeOptions);
}

void endBallMode(){
	setCanExitVehicle(true);
	setShowBallOptions(std::nullopt);
	changeGameType(gametypeSystem, NULL, NULL);
}

GameTypeInfo getBallMode(){
	GameTypeInfo ballMode = GameTypeInfo {
	  .gametypeName = "ball",
	  .events = { "ball-game" },
	  .createGametype = [](void* data) -> std::any {
			BallModeOptions* modeOptions = static_cast<BallModeOptions*>(data);
			{
				auto activePlayer = getActivePlayerId(0);
				auto vehicles = getVehicleIds();
				modassert(vehicles.size() == 1, "invalid expected vehicle size");
				modassert(activePlayer.has_value(), "ball set active player no active player");
				std::cout << "vehicles: " << print(vehicles) << "[" << gameapi -> getGameObjNameForId(vehicles.at(0)).value()  << "]" << ", playerid: " << print(activePlayer) << ", [" << gameapi -> getGameObjNameForId(activePlayer.value()).value()  <<  "]" << std::endl;
				std::cout << "vehicles: " << gameapi -> getGameObjNameForId(gameapi -> getActiveCamera(std::nullopt).value()).value()  << std::endl;
				enterVehicleRaw(0, vehicles.at(0), activePlayer.value());
				setCanExitVehicle(false);
			}

		  auto ballId = getBallId().value();
	  	auto pos = gameapi -> getGameObjectPos(ballId, true, "[gamelogic] get ball position for start");
	  	modeOptions -> initialBallPos = pos;
	  	modeOptions -> ballId = ballId;
 	   	showTimeElapsed(gameapi -> timeSeconds(true));

			changeUi(true);

	    return *modeOptions; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");

	  	std::string* message = std::any_cast<std::string>(&value);
	  	modassert(message, "invalid type ball-mode");
	  	std::cout << "from ball mode: " << event << ", " << *message << std::endl;

	  	if (*message == "complete"){
	  		setBallLevelComplete();
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
	  	std::cout << "ball mode: " << key << ", " << action << std::endl;
	  },
	  .onFrame = [](std::any& gametype) -> void {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");
	  	std::cout << "ball onframe" << std::endl;
	  	if (ballMode -> ballId.has_value()){
	  		auto powerup = getPowerup();
	  		if (powerup.has_value()){
		  		std::cout << "powerup: " << print(powerup.value()) << std::endl;
	  			if (powerup.value() == BIG_JUMP){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/jump.png");
	 		  		return;
	  			}else if (powerup.value() == LAUNCH_FORWARD){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/dash.png");
	 		  		return;
	  			}else if (powerup.value() == LOW_GRAVITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/bounce.png");
	 		  		return;
	  			}else if (powerup.value() == REVERSE_GRAVITY){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/bounce.normal.png");
	 		  		return;
	  			}else if (powerup.value() == TELEPORT){
	 		  		setPowerupTexture("../gameresources/build/textures/ballgame/teleport.png");
	 		  		return;
	  			}else{
	  				modassert(false, "invalid powerup size ball.cpp");
	  			}
	  		}
	  	}

  		setPowerupTexture("../gameresources/build/textures/ballgame/none.png");
	  },
	};
	return ballMode;
}

