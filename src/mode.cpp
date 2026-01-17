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