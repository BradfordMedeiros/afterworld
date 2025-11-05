#include "./ball.h"

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
		modeOptions -> setPlayerControl();
		modeOptions -> changeUi(true);
		modeOptions -> showTimeElapsed(gameapi -> timeSeconds(true));

	    return *modeOptions; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	  	BallModeOptions* ballMode = std::any_cast<BallModeOptions>(&gametype);
	  	modassert(ballMode, "ballMode options");

	  	std::string* message = std::any_cast<std::string>(&value);
	  	modassert(message, "invalid type ball-mode");
	  	std::cout << "from ball modde: " << event << ", " << *message << std::endl;
	  	if (*message == "complete"){
	  		ballMode -> setLevelFinished();
	  	}
	   	return false;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string {
	    return std::string("ball mode");
	  },
	  .getScoreInfo = [](std::any& gametype, float startTime) -> std::optional<GametypeData> { return std::nullopt; },
	};
	return ballMode;
}
