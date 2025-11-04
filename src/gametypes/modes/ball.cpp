#include "./ball.h"

extern CustomApiBindings* gameapi;

GameTypeInfo getBallMode(){
	GameTypeInfo ballMode = GameTypeInfo {
	  .gametypeName = "ball",
	  .events = { },
	  .createGametype = [](void* data) -> std::any {
		int scoreLimit = 3;
		std::vector<std::string> teamNames = { "red", "blue" };
		std::vector<int> scores = { 0, 0 };
		modassert(teamNames.size() == scores.size(), "team names is not score size");

		BallModeOptions* modeOptions = static_cast<BallModeOptions*>(data);
		std::cout << "ball mode: " << modeOptions -> testNumber << std::endl;
		modeOptions -> setPlayerControl();
	    //return DeathmatchMode { 
	    //	.scoreLimit = scoreLimit,
	    //	.teamNames = teamNames,
	    //	.scores = scores,
	    //};

  		//gameapi -> schedule(0, true, 10000, NULL, [](void*) -> void {
		//  auto activePlayer = getActivePlayerId(0);
		//  auto vehicles = getVehicleIds();
		//  std::cout << "vehicles: " << print(vehicles) << ", playerid: " << print(activePlayer) << std::endl;
		//  enterVehicle(0, vehicles.at(0), activePlayer.value());
  		//});
	    return std::nullopt; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	   	return false;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string {
	    return std::string("ball mode");
	  },
	  .getScoreInfo = [](std::any& gametype, float startTime) -> std::optional<GametypeData> { return std::nullopt; },
	};
	return ballMode;
}
