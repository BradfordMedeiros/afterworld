#include "./tv.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;

struct TvAiState {
	bool isActive;
};

std::any createTvAgent(objid id){
	return TvAiState {
		.isActive = false,
	};
}

void detectWorldInfoTvAgent(WorldInfo& worldInfo, Agent& agent){
  auto visibleTargets = checkVisibleTargets(worldInfo, agent.id);
  if (visibleTargets.size() > 0){
    auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id) /* bad basically a small leak */ ); 
    updateState(worldInfo, symbol, visibleTargets.at(0).position, {}, STATE_VEC3, agent.id);
  }
}
std::vector<Goal> getGoalsForTvAgent(WorldInfo& worldInfo, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int attackTargetGoal = getSymbol("attack-target");

  std::vector<Goal> goals = {};

  TvAiState* tvState = anycast<TvAiState>(agent.agentData);
  modassert(tvState, "attackState invalid");

	auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id));
  auto targetPosition = getState<glm::vec3>(worldInfo, symbol);
  auto distanceToTarget = glm::distance(targetPosition.value(), gameapi -> getGameObjectPos(agent.id, true));

  if (targetPosition.has_value() && distanceToTarget < 20 && tvState -> isActive){
    goals.push_back(
      Goal {
        .goaltype = attackTargetGoal,
        .goalData = NULL,
        .score = [&agent](std::any& targetPosition) -> int { 
          return 10;
        }
      }
    );
  }else{
  	goals.push_back(
  	  Goal {
  	    .goaltype = idleGoal,
  	    .goalData = NULL,
  	    .score = [&agent](std::any& targetPosition) -> int { 
  	      return 5;
  	    }
  	  }
  	);	
  }
	return goals;

}
void doGoalTvAgent(WorldInfo& worldInfo, Goal& goal, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int attackTargetGoal = getSymbol("attack-target");

  if (goal.goaltype == idleGoal){
    // do nothing
  }else if (goal.goaltype == attackTargetGoal){
  	float currentTime = gameapi -> timeSeconds(false);

		auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id));
  	auto targetPosition = getState<glm::vec3>(worldInfo, symbol).value();
  	auto agentPos = gameapi -> getGameObjectPos(agent.id, true);
  	glm::vec3 targetPosSameY = glm::vec3(targetPosition.x, agentPos.y, targetPosition.z);
    auto towardTarget = gameapi -> orientationFromPos(agentPos, targetPosSameY);
      
    aiInterface.look(agent.id, towardTarget);
  	
  }
}
void onAiTvAgentHealthChange(Agent& agent, objid targetId, float remainingHealth){

}

void setTvActive(Agent& agent, bool active){
	modlog("ai tv", "setTvActive");
	TvAiState* tvState = anycast<TvAiState>(agent.agentData);
  bool oldIsActive = tvState -> isActive;
  tvState -> isActive = active;
  if (oldIsActive != tvState -> isActive){
  	modlog("ai tv active", active ? "active" : "not-active");
    aiInterface.playAnimation(agent.id, active ? "activate" : "deactivate", FORWARDS);
  }

}

bool isAgentTv(Agent& agent){
  TvAiState* tvState = anycast<TvAiState>(agent.agentData);
  return tvState != NULL;
}

AiAgent tvAgent{
  .createAgent = createTvAgent,
  .detect = detectWorldInfoTvAgent,
  .getGoals = getGoalsForTvAgent,
  .doGoal = doGoalTvAgent,
  .onHealthChange = onAiTvAgentHealthChange,
};