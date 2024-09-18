#include "./basic_agent.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;


void doAnimationTrigger(objid id, const char* transition);

struct AgentAttackState {
  float lastAttackTime;
  std::optional<GunCore> gunCore;
  float initialHealth;
  bool moveVertical;
  bool aggravated;
  bool scared;
  std::set<objid> visited;
};

Agent createBasicAgent(objid id){
  auto initialHealth = getSingleFloatAttr(id, "health").value();
  auto moveVerticalAttr = getSingleAttr(id, "move-vertical");  // probably shouldn't be in ai system, i think
	return Agent{
    .id = id,
    .enabled = true,
    .type = AGENT_BASIC_AGENT,
    .agentData = AgentAttackState {
      .lastAttackTime = 0.f,
      .gunCore = createGunCoreInstance("pistol", 5, gameapi -> listSceneId(id)),
      .initialHealth = initialHealth,
      .moveVertical = moveVerticalAttr.has_value() && moveVerticalAttr.value() == "true",
      .aggravated = false,
      .scared = false,
      .visited = {},
    },
  };
}

void detectWorldInfoBasicAgent(WorldInfo& worldInfo, Agent& agent){
  auto visibleTargets = checkVisibleTargets(worldInfo, agent.id);
  if (visibleTargets.size() > 0){
    auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id) /* bad basically a small leak */ ); 
    updateState(worldInfo, symbol, visibleTargets.at(0).position, {}, STATE_VEC3, agent.id);
  }
}

std::optional<int> closestPositionIndex(glm::vec3 position, std::vector<EntityPosition>& entityPositions, std::set<objid>& visited){
  if (entityPositions.size() == 0){
    return std::nullopt;
  }
  std::optional<int> minIndex = std::nullopt;
  std::optional<float> minDistance = std::nullopt;
  for (int i = 0; i < entityPositions.size(); i++){
    if (visited.count(entityPositions.at(i).id) > 0){
      continue;
    }
    auto distance = glm::distance(position, entityPositions.at(i).position);
    if (!minIndex.has_value()){
      minIndex = i;
      minDistance = distance;
    }else if (distance < minDistance.value()){
      minIndex = i;
      minDistance = distance;
    }
  }
  if (!minIndex.has_value()){
    return std::nullopt;
  }
  return minIndex.value();
}

std::vector<Goal> getGoalsForBasicAgent(WorldInfo& worldInfo, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int moveToTargetGoal = getSymbol("move-to-target");
  static int attackTargetGoal = getSymbol("attack-target");
  static int getAmmoGoal = getSymbol("get-ammo");
  static int ammoSymbol = getSymbol("ammo");
  static int wanderGoal = getSymbol("wander");

  std::vector<Goal> goals = {};
  AgentAttackState* attackState = anycast<AgentAttackState>(agent.agentData);
  modassert(attackState, "attackState invalid");

  auto agentPos = gameapi -> getGameObjectPos(agent.id, true);

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

  if (attackState -> aggravated && targetPosition.has_value()){
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

  if (attackState -> scared){
    glm::vec3 moveToPosition = targetPosition.value();
    if (attackState -> scared){
      auto diff = moveToPosition - agentPos;
      moveToPosition = agentPos - diff;

      goals.push_back(
        Goal {
          .goaltype = moveToTargetGoal,
          .goalData = moveToPosition,
          .score = [&agent](std::any& targetPosition) -> int { 
            return 200;
          }
        }
      );
    }
  }


  if (attackState -> aggravated){
    goals.push_back(
      Goal {
        .goaltype = attackTargetGoal,
        .goalData = NULL,
        .score = [&agent, &worldInfo, symbol](std::any&) -> int {
            auto targetPosition = getState<glm::vec3>(worldInfo, symbol);
            if (targetPosition.has_value()){
              auto distance = glm::distance(targetPosition.value(), gameapi -> getGameObjectPos(agent.id, true));
              if (distance < 5){
                return 100;
              }
            }
            return 0;
        }
      }
    );    
  }

  if (!attackState -> aggravated && !attackState -> scared){
    static int pointOfInterest = getSymbol("interest-generic");
    auto pointOfInterestPositions = getStateByTag<EntityPosition>(worldInfo, { pointOfInterest });
    auto closestPointOfInterestIndex = closestPositionIndex(agentPos, pointOfInterestPositions, attackState -> visited);
    if (closestPointOfInterestIndex.has_value()){
      auto closestPointOfInterest = pointOfInterestPositions.at(closestPointOfInterestIndex.value());

      goals.push_back(
        Goal {
          .goaltype = wanderGoal,
          .goalData = closestPointOfInterest,
          .score = [&agent, &worldInfo, symbol](std::any&) -> int {
              return 100;
          }
        }
      );  
    }     
  }

  auto ammoPositions = getStateByTag<EntityPosition>(worldInfo, { ammoSymbol });
  if (ammoPositions.size() > 0){
    goals.push_back(
      Goal {
        .goaltype = getAmmoGoal,
        .goalData = NULL,
        .score = [&agent, &worldInfo, symbol, attackState](std::any&) -> int {
            if (attackState -> gunCore.has_value()){
              //modassert(attackState -> gunCore.value().weaponCore, "weapon core is null tho");
              auto currentAmmo = 100; //ammoForGun(attackState -> gunCore.value().weaponCore -> name);
              if (currentAmmo <= 0){
                return 150;
              }
            }
            return 0;
        }
      }
    );    
  }


  return goals;
}


void fireProjectile(objid agentId, AgentAttackState& agentAttackState){
  modlog("basic agent", "firing projectile");
  if (agentAttackState.gunCore.has_value()){
    fireGunAndVisualize(agentAttackState.gunCore.value(), false, true, std::nullopt, std::nullopt, agentId, "default");
  }
}


const bool useMovementSystem = true;
const bool useAiPathing = true;
void moveToTarget(objid agentId, glm::vec3 targetPosition, bool moveVertical, float speed = 1.f){
  auto finalTargetPosition = useAiPathing ? gameapi -> navPosition(agentId, targetPosition) : targetPosition;
  if (!useMovementSystem){  // simple behavior for debug
    auto agentPos = gameapi -> getGameObjectPos(agentId, true);
    auto towardTarget = gameapi -> orientationFromPos(agentPos, finalTargetPosition);
    auto newPos = gameapi -> moveRelative(agentPos, towardTarget, speed * gameapi -> timeElapsed());
    gameapi -> setGameObjectRot(agentId, towardTarget, true);
    gameapi -> setGameObjectPosition(agentId, newPos, true);
    doAnimationTrigger(agentId, "walking");
    return;
  }

  aiInterface.move(agentId, targetPosition, speed);
}
void attackTarget(Agent& agent){
  AgentAttackState* attackState = anycast<AgentAttackState>(agent.agentData);
  modassert(attackState, "attackState invalid");

  float currentTime = gameapi -> timeSeconds(false);
  if (currentTime - attackState -> lastAttackTime > 1.f){
    std::cout << "attack placeholder" << std::endl;
    attackState -> lastAttackTime = currentTime;
    fireProjectile(agent.id, *attackState);
  }
}

void doGoalBasicAgent(WorldInfo& worldInfo, Goal& goal, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int moveToTargetGoal = getSymbol("move-to-target");
  static int attackTargetGoal = getSymbol("attack-target");
  static int getAmmoGoal = getSymbol("get-ammo");
  static int fleeGoal = getSymbol("flee");
  static int wanderGoal = getSymbol("wander");

  AgentAttackState* agentData = anycast<AgentAttackState>(agent.agentData);
  modassert(agentData, "agentData invalid");

  if (goal.goaltype == idleGoal){
    // do nothing
    doAnimationTrigger(agent.id, "not-walking");
  }else if (goal.goaltype == wanderGoal){
    auto targetPosition = anycast<EntityPosition>(goal.goalData);
    modassert(targetPosition, "target pos was null");
    moveToTarget(agent.id, targetPosition -> position, agentData -> moveVertical, 0.7f); 
    auto agentPos = gameapi -> getGameObjectPos(agent.id, true);


    auto distanceToGoal = glm::distance(targetPosition -> position, agentPos);
    if (distanceToGoal < 1.f){ // what if the enemy can't reach the goal?
      modlog("ai basic agent", "agent arrived at goal");
      agentData -> visited.insert(targetPosition -> id); // goal id
      if (agentData -> visited.size() == 3){
        agentData -> visited = {};
      }
    }
  }else if (goal.goaltype == moveToTargetGoal){
    auto targetPosition = anycast<glm::vec3>(goal.goalData);
    modassert(targetPosition, "target pos was null");
    moveToTarget(agent.id, *targetPosition, agentData -> moveVertical);
  }else if (goal.goaltype == attackTargetGoal){
    // not yet implemented
    attackTarget(agent);
    doAnimationTrigger(agent.id, "not-walking");
  }else if (goal.goaltype == getAmmoGoal){
    static int ammoSymbol = getSymbol("ammo");
    auto ammoPositions = getStateByTag<EntityPosition>(worldInfo, { ammoSymbol });
    if (ammoPositions.size() > 0){
      auto ammoPosition = ammoPositions.at(0);
      moveToTarget(agent.id, ammoPosition.position, agentData -> moveVertical);
    }
  }else if (goal.goaltype == fleeGoal){
    auto targetPosition = anycast<glm::vec3>(goal.goalData);
    modassert(targetPosition, "target pos was null");
    moveToTarget(agent.id, *targetPosition, agentData -> moveVertical); 
  }
}

void onAiAmmo(Agent& agent, objid targetId, int amount){
  if (targetId == agent.id){
    AgentAttackState* attackState = anycast<AgentAttackState>(agent.agentData);
    modassert(attackState, "attackState invalid");
    if (attackState -> gunCore.has_value()){
      deliverAmmo("default", attackState -> gunCore.value().weaponCore -> weaponParams.name, amount);
    }
  }
}


void onAiHealthChange(Agent& agent, objid targetId, float remainingHealth){
  if (targetId == agent.id){
    AgentAttackState* attackState = anycast<AgentAttackState>(agent.agentData);
    modassert(attackState, "attackState invalid");
    attackState -> aggravated = true;
    float currHealthPercentage = remainingHealth / attackState -> initialHealth;
    if (currHealthPercentage < 0.2f){
      attackState -> scared = true;
    }
  }
}
