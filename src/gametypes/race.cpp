#include "./race.h"

struct RaceMode {
};

GameTypeInfo getRaceMode(){
	GameTypeInfo race = GameTypeInfo {
	  .gametypeName = "race",
	  .events = { },
	  .createGametype = []() -> std::any {
	    return RaceMode { 
	    }; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, AttributeValue value) -> bool {
	    RaceMode* raceMode = std::any_cast<RaceMode>(&gametype);
	    modassert(raceMode, "raceMode mode null");
	    return 0;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string { 
	  	return "race mode";
	  }
	};
	return race;
}
