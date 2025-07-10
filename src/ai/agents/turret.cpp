#include "./turret.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;


struct TurretAiState {
  float health;
  float initialHealth;
	float lastAttackTime;
  bool changedGun;
  bool isGunRaised;
};

std::any createTurretAgent(objid id){
  auto initialHealth = getSingleFloatAttr(id, "health").value();
	return TurretAiState {
    .health = initialHealth,
    .initialHealth = initialHealth,
    .lastAttackTime = 0.f,
    .changedGun = false,
    .isGunRaised = true,
  };
}

void detectWorldInfoTurretAgent(WorldInfo& worldInfo, Agent& agent){
  auto visibleTargets = checkVisibleTargets(worldInfo, agent.id);
  if (visibleTargets.size() > 0){
    setAgentTargetId(worldInfo, agent.id, visibleTargets.at(0).id);
  }

}

std::vector<Goal> getGoalsForTurretAgent(WorldInfo& worldInfo, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int attackTargetGoal = getSymbol("attack-target");

  std::vector<Goal> goals = {};

  TurretAiState* turretState = anycast<TurretAiState>(agent.agentData);
  modassert(turretState, "attackState invalid");

  auto targetPosition = getAgentTargetPos(worldInfo, agent.id);
  bool hasTarget = false;
  if (targetPosition.has_value()){
    auto distanceToTarget = glm::distance(targetPosition.value(), gameapi -> getGameObjectPos(agent.id, true, "[gamelogic] getGoalsForTurretAgent"));
    if (distanceToTarget < 20  && turretState -> isGunRaised){
      hasTarget = true;
    }
  }

  if (hasTarget){
    goals.push_back(
      Goal {
        .goaltype = attackTargetGoal,
        .goalData = NULL,
        .score = [&agent](std::any&) -> int { 
          return 10;
        }
      }
    );
  }else{
  	goals.push_back(
  	  Goal {
  	    .goaltype = idleGoal,
  	    .goalData = NULL,
  	    .score = [&agent](std::any&) -> int { 
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

  if (!turretState -> changedGun){
    aiInterface.changeGun(agent.id, "scrapgun");
    turretState -> changedGun = true;
    return;
  }

  if (goal.goaltype == idleGoal){
    // do nothing
  }else if (goal.goaltype == attackTargetGoal){
  	float currentTime = gameapi -> timeSeconds(false);
  	if (currentTime - turretState -> lastAttackTime > 1.f){
  	  modlog("ai basic", "attack");
  	  turretState -> lastAttackTime = currentTime;

      auto targetPosition = getAgentTargetPos(worldInfo, agent.id).value();
  		auto agentPos = gameapi -> getGameObjectPos(agent.id, true, "[gamelogic] doGoalTurretAgent");
  		glm::vec3 targetPosSameY = glm::vec3(targetPosition.x, agentPos.y, targetPosition.z);
    	auto towardTarget = gameapi -> orientationFromPos(agentPos, targetPosSameY);
      
      if ((turretState -> health / turretState -> initialHealth) < 0.6f){
        setGunTurret(agent, false);
        return;
      }

    	aiInterface.look(agent.id, towardTarget);
   	  aiInterface.fireGun(agent.id);
  	}
  }
}


void onAiTurretAgentHealthChange(Agent& agent, objid targetId, float remainingHealth){
  if (targetId == agent.id){
    TurretAiState* turretState = anycast<TurretAiState>(agent.agentData);
    modassert(turretState, "attackState invalid");
    turretState -> health = remainingHealth;
    modlog("turret health", std::to_string(remainingHealth));
  }
}

void setGunTurret(Agent& agent, bool isGunRaised){
  TurretAiState* turretState = anycast<TurretAiState>(agent.agentData);
  modassert(turretState, "attackState invalid");
  bool oldIsGunRaised = turretState -> isGunRaised;
  turretState -> isGunRaised = isGunRaised;
  if (oldIsGunRaised != turretState -> isGunRaised){
    aiInterface.playAnimation(agent.id, isGunRaised ? "raise" : "lower", FORWARDS);
  }
}

bool isGunRaisedTurret(Agent& agent){
  TurretAiState* turretState = anycast<TurretAiState>(agent.agentData);
  modassert(turretState, "attackState invalid");
  return turretState -> isGunRaised;  
}

bool isAgentTurret(Agent& agent){
  TurretAiState* turretState = anycast<TurretAiState>(agent.agentData);
  return turretState != NULL;
}

AiAgent turretAgent{
  .createAgent = createTurretAgent,
  .detect = detectWorldInfoTurretAgent,
  .getGoals = getGoalsForTurretAgent,
  .doGoal = doGoalTurretAgent,
  .onHealthChange = onAiTurretAgentHealthChange,
};