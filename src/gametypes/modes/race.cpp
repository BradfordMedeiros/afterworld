#include "./race.h"

extern CustomApiBindings* gameapi;

struct RaceMode {
  double startTime;
  double durationSeconds;
  size_t currentCheckpoint;
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
	  .createGametype = [](void*) -> std::any {
	    return RaceMode {
	    	.startTime = gameapi -> timeSeconds(false),
	    	.durationSeconds = 30.f,
	    	.currentCheckpoint = 0,
	    	.checkpoints = findCheckpoints(),
	    }; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& anyValue) -> bool {
	  	RaceMode* raceMode = std::any_cast<RaceMode>(&gametype);
	  	auto value = anycast<AttributeValue>(anyValue);
	  	modassert(value, "race mode any value invalid");

	    auto floatValue = std::get_if<float>(value);
	    modassert(floatValue, "raceMode unexpected type for raceMode");
	    auto index = static_cast<size_t>(*floatValue);
	    if (raceMode -> currentCheckpoint == index - 1){
	    	raceMode -> currentCheckpoint = glm::min(index, raceMode -> checkpoints.size());
	    }
	    return raceMode -> currentCheckpoint == raceMode -> checkpoints.size();
	  },
	  .getDebugText = [](std::any& gametype) -> std::string {
	    RaceMode* raceMode = std::any_cast<RaceMode>(&gametype);
	    auto timeRemainingVal = timeRemaining(*raceMode);
	    std::string timeRemaining = std::to_string(timeRemainingVal > 0 ? timeRemainingVal : 0.f);
	    auto didWin = raceMode -> currentCheckpoint >= raceMode -> checkpoints.size();
	    modassert(raceMode, "raceMode mode null");
	    std::string checkpointStr = std::to_string(raceMode -> currentCheckpoint) + "/" + std::to_string(raceMode -> checkpoints.size());
	  	return std::string("checkpoint done: ") + checkpointStr + ": time remaining: " + timeRemaining;
	  },
	  .getScoreInfo = [](std::any& gametype, float startTime) -> std::optional<GametypeData> {
	    RaceMode* raceMode = std::any_cast<RaceMode>(&gametype);
	  	modassert(raceMode, "race mode any value invalid");

  		float gametypeLength = 200.f;
  		GametypeData gametypeData {
  		  .gametypeName = "race",
  		  .score1 = raceMode -> currentCheckpoint,
  		  .score2 = static_cast<int>(raceMode -> checkpoints.size()),
  		  .totalScore = static_cast<int>(raceMode -> checkpoints.size()),
  		  .remainingTime = gametypeLength + (startTime - gameapi -> timeSeconds(false)),
  		};
  		return gametypeData;
	  },
	};
	return race;
}
