#include "./targetkill.h"

struct TargetKillMode {
  int numTargets;
  float startTime;
  float durationSeconds;
};


GameTypeInfo getTargetKill(){
	GameTypeInfo targetKill = GameTypeInfo {
	  .gametypeName = "targetkill",
	  .events = { "nohealth" },
	  .createGametype = []() -> std::any {
	    return TargetKillMode { 
	      .numTargets = 3,
	      .startTime = 1.f,
	      .durationSeconds = 20,
	    }; 
	  },
	  .onEvent = [](std::any& gametype, std::string& event, AttributeValue value) -> bool {
	    TargetKillMode* targetKillMode = std::any_cast<TargetKillMode>(&gametype);
	    modassert(targetKillMode, "target kill mode null");
	    targetKillMode -> numTargets--;
	    modlog("gametypes", "on event: " + event + ", value = " + print(value));
	    return targetKillMode -> numTargets == 0;
	  },
	  .getDebugText = [](std::any& gametype) -> std::string { 
	    TargetKillMode* targetKillMode = std::any_cast<TargetKillMode>(&gametype);
	    modassert(targetKillMode, "target kill mode null");
	    return std::string("targets remaining: ") + std::to_string(targetKillMode -> numTargets);
	  }
	};
	return targetKill;
}
