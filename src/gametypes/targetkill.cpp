#include "./targetkill.h"

GameTypeInfo getTargetKill(){
	GameTypeInfo targetKill = GameTypeInfo {
	  .gametypeName = "targetkill",
	  .events = { "nohealth" },
	  .createGametype = []() -> GameType { 
	    return TargetKillMode { 
	      .numTargets = 3,
	      .startTime = 1.f,
	      .durationSeconds = 20,
	    }; 
	  },
	  .onEvent = [](GameType& gametype, std::string& event, AttributeValue value) -> bool {
	    TargetKillMode* targetKillMode = std::get_if<TargetKillMode>(&gametype);
	    modassert(targetKillMode, "target kill mode null");
	    targetKillMode -> numTargets--;
	    modlog("gametypes", "on event: " + event + ", value = " + print(value));
	    return targetKillMode -> numTargets == 0;
	  },
	  .getDebugText = [](GameType& gametype) -> std::string { 
	    TargetKillMode* targetKillMode = std::get_if<TargetKillMode>(&gametype);
	    modassert(targetKillMode, "target kill mode null");
	    return std::string("targets remaining: ") + std::to_string(targetKillMode -> numTargets);
	  }
	};
	return targetKill;
}
