#ifndef MOD_AFTERWORLD_AI_BASIC_AGENT
#define MOD_AFTERWORLD_AI_BASIC_AGENT

#include "./common.h"

Agent createBasicAgent(objid id);
std::vector<Goal> getGoalsForBasicAgent(WorldInfo& worldInfo, Agent& agent);

#endif