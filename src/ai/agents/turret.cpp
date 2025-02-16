#include "./turret.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;


struct TurretAiState {
	float lastAttackTime;
};

Agent createTurretAgent(objid id){
	return Agent{
    .id = id,
    .enabled = true,
    .type = AGENT_TURRET,
    .agentData = TurretAiState {
    	.lastAttackTime = 0.f,
    },
  };
}

void detectWorldInfoTurretAgent(WorldInfo& worldInfo, Agent& agent){
  auto visibleTargets = checkVisibleTargets(worldInfo, agent.id);
  if (visibleTargets.size() > 0){
    auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id) /* bad basically a small leak */ ); 
    updateState(worldInfo, symbol, visibleTargets.at(0).position, {}, STATE_VEC3, agent.id);
  }

}

std::vector<Goal> getGoalsForTurretAgent(WorldInfo& worldInfo, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int attackTargetGoal = getSymbol("attack-target");

  std::vector<Goal> goals = {};

	auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id));
  auto targetPosition = getState<glm::vec3>(worldInfo, symbol);
  auto distanceToTarget = glm::distance(targetPosition.value(), gameapi -> getGameObjectPos(agent.id, true));

  if (targetPosition.has_value() && distanceToTarget < 5 ){
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

void doGoalTurretAgent(WorldInfo& worldInfo, Goal& goal, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int attackTargetGoal = getSymbol("attack-target");

  TurretAiState* turretState = anycast<TurretAiState>(agent.agentData);
  modassert(turretState, "attackState invalid");

  if (goal.goaltype == idleGoal){
    // do nothing
  }else if (goal.goaltype == attackTargetGoal){
  	float currentTime = gameapi -> timeSeconds(false);
  	if (currentTime - turretState -> lastAttackTime > 1.f){
  	  modlog("ai basic", "attack");
  	  turretState -> lastAttackTime = currentTime;

			auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id));
  		auto targetPosition = getState<glm::vec3>(worldInfo, symbol).value();
  		auto agentPos = gameapi -> getGameObjectPos(agent.id, true);
  		glm::vec3 targetPosSameY = glm::vec3(targetPosition.x, agentPos.y, targetPosition.z);
    	auto towardTarget = gameapi -> orientationFromPos(agentPos, targetPosSameY);
    
    	aiInterface.look(agent.id, towardTarget);
   	  aiInterface.fireGun(agent.id);
  	}
  }
}

