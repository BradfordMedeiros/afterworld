#include "./race.h"

extern CustomApiBindings* gameapi;

struct RaceMode {
  double startTime;
  double durationSeconds;
  int currentCheckpoint;
  std::vector<int> checkpoints;
};

float timeRemaining(RaceMode& mode){
	auto currTime = gameapi -> timeSeconds(false);
	double elapsedTime = currTime - mode.startTime;
	return mode.durationSeconds - elapsedTime;
}

bool hasTimeRemaining(RaceMode& mode){
	return timeRemaining(mode) > 0;
}

std::vector<int> findCheckpoints(){
	// gamemarker-race:2
	auto ids = gameapi -> getObjectsByAttr("gamemarker-race", std::nullopt, std::nullopt);
	std::vector<int> checkpoints = {};
	for (auto id : ids){
		checkpoints.push_back(id);
	}
	return checkpoints;
}

GameTypeInfo getRaceMode(){
	GameTypeInfo race = GameTypeInfo {
	  .gametypeName = "race",
	  .events = { "race-marker" },
	  .createGametype = []() -> std::any {
	    return RaceMode {
	    	.startTime = gameapi -> timeSeconds(false),
	    	.durationSeconds = 30.f,
	    	.currentCheckpoint = 0,
	    	.checkpoints = findCheckpoints(),
	    }; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, AttributeValue value) -> bool {
	    RaceMode* raceMode = std::any_cast<RaceMode>(&gametype);
	    auto strValue = std::get_if<float>(&value);
	    modassert(strValue, "raceMode unexpected type for raceMode");
	    std::cout << "got trigger switch value: " << *strValue << std::endl;
	    modassert(false, "got racemarker");
	    return 0;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string {
	    RaceMode* raceMode = std::any_cast<RaceMode>(&gametype);
	    auto timeRemainingVal = timeRemaining(*raceMode);
	    std::string timeRemaining = std::to_string(timeRemainingVal > 0 ? timeRemainingVal : 0.f);
	    auto didWin = raceMode -> currentCheckpoint >= raceMode -> checkpoints.size();
	    modassert(raceMode, "raceMode mode null");
	    std::string checkpointStr = std::to_string(raceMode -> currentCheckpoint) + "/" + std::to_string(raceMode -> checkpoints.size());
	  	return std::string("checkpoint done: ") + checkpointStr + ": time remaining: " + timeRemaining;
	  }
	};
	return race;
}
