#include "./deathmatch.h"

extern CustomApiBindings* gameapi;


struct DeathmatchMode {
	int scoreLimit;
	std::vector<std::string> teamNames;
	std::vector<int> scores;
};

std::string* getWinner(DeathmatchMode& mode){
	for (int i = 0; i < mode.scores.size(); i++){
		auto score = mode.scores.at(i);
		if (score >= mode.scoreLimit){
			return &mode.teamNames.at(i);
		}
	}
	return NULL;
}

std::optional<int> lookupTeam(DeathmatchMode& deathmatchMode, NoHealthMessage& message){
	//std::optional<std::string> getStrAttr(GameobjAttributes& objAttr, std::string key);
	auto team = message.team;
	if (team.has_value()){
		auto teamName = team.value();
		for (int i = 0; i < deathmatchMode.teamNames.size(); i++){
			if (deathmatchMode.teamNames.at(i) == teamName){
				return i;
			}
		}
		return NULL;
	}
	return std::nullopt;
}

std::string generateScoreStr(DeathmatchMode& mode){
	std::string scoreStr = "";
	for (int i = 0; i < mode.teamNames.size(); i++){
		scoreStr += std::string("(") + mode.teamNames.at(i) + " - " + std::to_string(mode.scores.at(i)) +  " )";
	}
	return scoreStr;
}

GameTypeInfo getDeathmatchMode(){
	GameTypeInfo deathmatchMode = GameTypeInfo {
	  .gametypeName = "deathmatch",
	  .events = { "nohealth"  },
	  .createGametype = []() -> std::any {
			int scoreLimit = 3;
			std::vector<std::string> teamNames = { "red", "blue" };
			std::vector<int> scores = { 0, 0 };
			modassert(teamNames.size() == scores.size(), "team names is not score size");
	    return DeathmatchMode { 
	    	.scoreLimit = scoreLimit,
	    	.teamNames = teamNames,
	    	.scores = scores,
	    }; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	    DeathmatchMode* deathmatchMode = std::any_cast<DeathmatchMode>(&gametype);
	    modassert(deathmatchMode, "deatchmatchMode mode null");
	    if (event == "nohealth"){
      	auto nohealthMessage = anycast<NoHealthMessage>(value);
      	modassert(nohealthMessage, "deathmatch nohealth target id null");
	      auto teamId = lookupTeam(*deathmatchMode, *nohealthMessage);
	      if (teamId.has_value()){
	      	deathmatchMode -> scores.at(teamId.value())++;
	      }
	    }
	   	return false;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string {
	    DeathmatchMode* deathmatchMode = std::any_cast<DeathmatchMode>(&gametype);
	    modassert(deathmatchMode, "deathModeMode mode null");
	  	auto winner = getWinner(*deathmatchMode);

	    return std::string("deathmatch mode: ") + (winner ? (*winner + " won!") : "in progress") + generateScoreStr(*deathmatchMode);
	  }
	};
	return deathmatchMode;
}
