#ifndef MOD_AFTERWORLD_AI_BASIC_AGENT
#define MOD_AFTERWORLD_AI_BASIC_AGENT

#include "./common.h"
#include "../../weapons/weaponcore.h"


Agent createBasicAgent(objid id);
void detectWorldInfoBasicAgent(WorldInfo& worldInfo, Agent& agent);
std::vector<Goal> getGoalsForBasicAgent(WorldInfo& worldInfo, Agent& agent);
void doGoalBasicAgent(WorldInfo& worldInfo, Goal& goal, Agent& agent);

void onAiBasicAgentHealthChange(Agent& agent, objid targetId, float remainingHealth);

#endif