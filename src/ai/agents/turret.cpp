#include "./turret.h"

Agent createTurretAgent(objid id){
	return Agent{
    .id = id,
    .type = AGENT_TURRET,
    .agentData = NULL,
  };
}

void detectWorldInfoTurretAgent(WorldInfo& worldInfo, Agent& agent){
	modassert(false, "detect world info for turret not yet implemented");
}

std::vector<Goal> getGoalsForTurretAgent(WorldInfo& worldInfo, Agent& agent){
	modassert(false, "get goals for turret not yet implemented");
	return {};
}

void doGoalTurretAgent(Goal& goal, Agent& agent){
	modassert(false, "do goal for turret not yet implemented")
}

