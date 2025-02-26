#include "./crawler.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;

struct CrawlerAiState {
};

std::any createCrawlerAgent(objid id){
	return CrawlerAiState {};
}

void detectWorldInfoCrawlerAgent(WorldInfo& worldInfo, Agent& agent){

}
std::vector<Goal> getGoalsForCrawlerAgent(WorldInfo& worldInfo, Agent& agent){
	return {};
}
void doGoalCrawlerAgent(WorldInfo& worldInfo, Goal& goal, Agent& agent){

}
void onAiCrawlerAgentHealthChange(Agent& agent, objid targetId, float remainingHealth){

}

AiAgent crawlerAgent{
  .createAgent = createCrawlerAgent,
  .detect = detectWorldInfoCrawlerAgent,
  .getGoals = getGoalsForCrawlerAgent,
  .doGoal = doGoalCrawlerAgent,
  .onHealthChange = onAiCrawlerAgentHealthChange,
};