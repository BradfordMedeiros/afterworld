#include "./scene_ai.h"

extern AiData aiData;

void raiseTurret(objid id, bool raiseUp){
  Agent& agent = getAgent(aiData, id);
  if (isAgentTurret(agent)){
    auto isGunRaised = isGunRaisedTurret(agent);
    setGunTurret(agent, !isGunRaised);
  }
}

void wakeUpTv(objid id, bool active){
  Agent& agent = getAgent(aiData, id);
  if (isAgentTv(agent)){
    setTvActive(agent, active);
  }
}