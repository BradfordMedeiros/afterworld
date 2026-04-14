#include "./targetkill.h"

extern CustomApiBindings* gameapi;

struct TargetKillMode {
  int numTargets;
  float startTime;
  float durationSeconds;
};


GameTypeInfo getTargetKill(){
	GameTypeInfo targetKill = GameTypeInfo {
	  .gametypeName = "targetkill",
	  .events = { "nohealth" },
	  .createGametype = [](void*) -> std::any {
	    return TargetKillMode { 
	      .numTargets = 3,
	      .startTime = 1.f,
	      .durationSeconds = 20,
	    }; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, std::any& value) -> bool {
	    TargetKillMode* targetKillMode = std::any_cast<TargetKillMode>(&gametype);
	    modassert(targetKillMode, "target kill mode null");
	    targetKillMode -> numTargets--;
	    return targetKillMode -> numTargets == 0;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string { 
	    TargetKillMode* targetKillMode = std::any_cast<TargetKillMode>(&gametype);
	    modassert(targetKillMode, "target kill mode null");
	    return std::string("targets remaining: ") + std::to_string(targetKillMode -> numTargets);
	  },
	  .getScoreInfo = [](std::any& gametype, float startTime) -> std::optional<GametypeData> {
	    TargetKillMode* targetKillMode = std::any_cast<TargetKillMode>(&gametype);
	    modassert(targetKillMode, "target kill mode null");
  		float gametypeLength = 200.f;
  		GametypeData gametypeData {
  		  .gametypeName = "targetkill",
  		  .score1 = 0,
  		  .score2 = 0,
  		  .totalScore = targetKillMode -> numTargets,
  		  .remainingTime = gametypeLength + (startTime - gameapi -> timeSeconds(false)),
  		};
  		return gametypeData;
	  },
	};
	return targetKill;
}
