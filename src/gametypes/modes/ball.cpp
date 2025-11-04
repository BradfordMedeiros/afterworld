#include "./ball.h"

GameTypeInfo getBallMode(){
	GameTypeInfo ballMode = GameTypeInfo {
	  .gametypeName = "ball",
	  .events = { },
	  .createGametype = []() -> std::any {
		int scoreLimit = 3;
		std::vector<std::string> teamNames = { "red", "blue" };
		std::vector<int> scores = { 0, 0 };
		modassert(teamNames.size() == scores.size(), "team names is not score size");
	    //return DeathmatchMode { 
	    //	.scoreLimit = scoreLimit,
	    //	.teamNames = teamNames,
	    //	.scores = scores,
	    //};
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
