#include "./tv.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;

struct TvAiState {
};

Agent createTvAgent(objid id){
	return Agent{
    .id = id,
    .enabled = true,
    .type = AGENT_TV,
    .agentData = TvAiState {},
  };
}

void detectWorldInfoTvAgent(WorldInfo& worldInfo, Agent& agent){

}
std::vector<Goal> getGoalsForTvAgent(WorldInfo& worldInfo, Agent& agent){
	return {};
}
void doGoalTvAgent(WorldInfo& worldInfo, Goal& goal, Agent& agent){

}
void onAiTvAgentHealthChange(Agent& agent, objid targetId, float remainingHealth){

}