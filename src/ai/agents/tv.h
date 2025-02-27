#ifndef MOD_AFTERWORLD_AI_TV
#define MOD_AFTERWORLD_AI_TV

#include "./common.h"

void setTvActive(Agent& agent, bool active);
bool isAgentTv(Agent& agent);

extern AiAgent tvAgent;

#endif