#ifndef MOD_AFTERWORLD_AI_TURRET
#define MOD_AFTERWORLD_AI_TURRET

#include "./common.h"

Agent createTurretAgent(objid id);
std::vector<Goal> getGoalsForTurretAgent();

#endif