#include "./tv.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;

struct TvAiState {
};

std::any createTvAgent(objid id){
	return TvAiState {};
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

AiAgent tvAgent{
  .createAgent = createTvAgent,
  .detect = detectWorldInfoTvAgent,
  .getGoals = getGoalsForTvAgent,
  .doGoal = doGoalTvAgent,
  .onHealthChange = onAiTvAgentHealthChange,
};