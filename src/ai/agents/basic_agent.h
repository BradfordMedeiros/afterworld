#ifndef MOD_AFTERWORLD_AI_BASIC_AGENT
#define MOD_AFTERWORLD_AI_BASIC_AGENT

#include "./common.h"

Agent createBasicAgent(objid id);
void detectWorldInfoBasicAgent(WorldInfo& worldInfo, Agent& agent);
std::vector<Goal> getGoalsForBasicAgent(WorldInfo& worldInfo, Agent& agent);
void doGoalBasicAgent(Goal& goal, Agent& agent);

#endif