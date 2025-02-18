#ifndef MOD_AFTERWORLD_AI_TURRET
#define MOD_AFTERWORLD_AI_TURRET

#include "./common.h"

void setGunTurret(Agent& agent, bool isGunRaised);
bool isGunRaisedTurret(Agent& agent);
bool isAgentTurret(Agent& agent);

extern AiAgent turretAgent;

#endif