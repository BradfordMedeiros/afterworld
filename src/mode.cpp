#include "./mode.h"

extern CustomApiBindings* gameapi;

extern GameTypes gametypeSystem;
extern std::optional<std::string> activeLevel;
extern Vehicles vehicles;

void setCanExitVehicle(bool canExit);
void enterVehicleRaw(int playerIndex, objid vehicleId, objid id);
void goToLevel(std::string levelShortName);

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


void startBallMode(objid sceneId){
	auto playerSpawnId = findObjByShortName("playerspawn", std::nullopt);
	auto position = gameapi -> getGameObjectPos(playerSpawnId.value(), true, "[gamelogic] ball - get playerspawn position");

	createBallObj(sceneId, position);

	BallModeOptions modeOptions {};

	modeOptions.getBallId = []() -> std::optional<objid> {
	  auto vehicles = getVehicleIds();
 	  modassert(vehicles.size() == 1, "invalid expected vehicle size");
	  return vehicles.at(0);
	};
	modeOptions.setPlayerControl = [](std::function<void()> fn) -> void {
  		gameapi -> schedule(0, true, 1, NULL, [fn](void*) -> void {
		  auto activePlayer = getActivePlayerId(0);
		  auto vehicles = getVehicleIds();
		  modassert(vehicles.size() == 1, "invalid expected vehicle size");
		  modassert(activePlayer.has_value(), "ball set active player no active player");
		  std::cout << "vehicles: " << print(vehicles) << "[" << gameapi -> getGameObjNameForId(vehicles.at(0)).value()  << "]" << ", playerid: " << print(activePlayer) << ", [" << gameapi -> getGameObjNameForId(activePlayer.value()).value()  <<  "]" << std::endl;
		  std::cout << "vehicles: " << gameapi -> getGameObjNameForId(gameapi -> getActiveCamera(std::nullopt).value()).value()  << std::endl;
		  enterVehicleRaw(0, vehicles.at(0), activePlayer.value());
		  setCanExitVehicle(false);
		  fn();
  		});
	};
	modeOptions.changeUi = [](bool showBallUi) -> void {
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
	};
	modeOptions.showTimeElapsed = [](std::optional<float> startTime) -> void {
		if (!showBallOptions().has_value()){
			return;
		}
		auto ballOptions = showBallOptions().value();
		ballOptions.startTime = startTime;
		setShowBallOptions(ballOptions);
	};
	modeOptions.setLevelFinished = []() -> void {
		setBallLevelComplete();
	};
	modeOptions.getPowerup = []() -> std::optional<BallPowerup> {
		auto activePlayer = getActivePlayerId(0);
		auto vehicleIds = getVehicleIds();
		if (vehicleIds.size() == 0){
			return std::nullopt;
		}
		return getBallPowerup(vehicles, vehicleIds.at(0));
	};
	modeOptions.setPowerupTexture = [](std::string texture) -> void {
		auto ballOptions = showBallOptions();
		if (!ballOptions.has_value()){
			return;
		}
		if (ballOptions.has_value()){
			auto newBallOptions = ballOptions.value();
			newBallOptions.powerupTexture = texture;;
			setShowBallOptions(newBallOptions);
		}
	};

	changeGameType(gametypeSystem, "ball", &modeOptions);
}

void endBallMode(){
	setCanExitVehicle(true);
	setShowBallOptions(std::nullopt);
	changeGameType(gametypeSystem, NULL, NULL);
}

