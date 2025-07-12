#include "./crawler.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;

struct CrawlerAiState {
};

std::any createCrawlerAgent(objid id){
	return CrawlerAiState {};
}

void detectWorldInfoCrawlerAgent(WorldInfo& worldInfo, Agent& agent){
  auto visibleTargets = checkVisibleTargets(worldInfo, agent.id);
  if (visibleTargets.size() > 0){
    setAgentTargetId(worldInfo, agent, visibleTargets.at(0).id);
  }
}
std::vector<Goal> getGoalsForCrawlerAgent(WorldInfo& worldInfo, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int moveToTargetGoal = getSymbol("move-to-target");

  std::vector<Goal> goals = {};

  goals.push_back(
    Goal {
      .goaltype = idleGoal,
      .goalData = NULL,
      .score = [&agent](std::any& targetPosition) -> int { 
        return 5;
      }
    }
  );

  auto targetId = getAgentTargetId(worldInfo, agent);
  if (targetId.has_value()){
    auto targetPosition = gameapi -> getGameObjectPos(targetId.value(), true, "[gamelogic] getGoalsForCrawlerAgent targetPosition");
    goals.push_back(
      Goal {
        .goaltype = moveToTargetGoal,
        .goalData = targetPosition,
        .score = [&agent](std::any& targetPosition) -> int { 
          return 10;
        }
      }
    );
  }

	return goals;
}
void doGoalCrawlerAgent(WorldInfo& worldInfo, Goal& goal, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int moveToTargetGoal = getSymbol("move-to-target");

  if (goal.goaltype == idleGoal){
    // do nothing
  }else if (goal.goaltype == moveToTargetGoal){
    auto agentPos = gameapi -> getGameObjectPos(agent.id, true, "[gamelogic] doGoalCrawlerAgent get agentPos");

    auto targetId = getAgentTargetId(worldInfo, agent);
    auto targetPosition = gameapi -> getGameObjectPos(targetId.value(), true, "[gamelogic] doGoalCrawlerAgent targetPosition");

    aiInterface.move(agent.id, targetPosition,  1.f);

    if (glm::distance(agentPos, targetPosition) < 1.5f){
      aiInterface.doDamage(agent.id, 10000.f); 
    }
  }
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