#ifndef MOD_AFTERWORLD_AI_TURRET
#define MOD_AFTERWORLD_AI_TURRET

#include "./common.h"

Agent createTurretAgent(objid id);
void detectWorldInfoTurretAgent(WorldInfo& worldInfo, Agent& agent);
std::vector<Goal> getGoalsForTurretAgent(WorldInfo& worldInfo, Agent& agent);
void doGoalTurretAgent(WorldInfo& worldInfo, Goal& goal, Agent& agent);

#endif