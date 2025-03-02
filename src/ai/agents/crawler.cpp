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
    auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id) /* bad basically a small leak */ ); 
    updateState(worldInfo, symbol, visibleTargets.at(0).position, {}, STATE_VEC3, agent.id);
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

  auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id));
  auto targetPosition = getState<glm::vec3>(worldInfo, symbol);

  if (targetPosition.has_value()){
    goals.push_back(
      Goal {
        .goaltype = moveToTargetGoal,
        .goalData = targetPosition.value(),
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
    auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id));
    auto agentPos = gameapi -> getGameObjectPos(agent.id, true);
    auto targetPosition = getState<glm::vec3>(worldInfo, symbol).value();

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