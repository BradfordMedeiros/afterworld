#include "./basic_agent.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;


struct AgentAttackState {
  float lastAttackTime;
  float initialHealth;
  bool moveVertical;
  bool aggravated;
  bool scared;
  std::set<objid> visited;
};

std::any createBasicAgent(objid id){
  auto initialHealth = getSingleFloatAttr(id, "health").value();
  auto moveVerticalAttr = getSingleAttr(id, "move-vertical");  // probably shouldn't be in ai system, i think
	return AgentAttackState {
      .lastAttackTime = 0.f,
      .initialHealth = initialHealth,
      .moveVertical = moveVerticalAttr.has_value() && moveVerticalAttr.value() == "true",
      .aggravated = false,
      .scared = false,
      .visited = {},
  };
}

void detectWorldInfoBasicAgent(WorldInfo& worldInfo, Agent& agent){
  auto targetId = getAgentTargetId(worldInfo, agent);
  if (targetId.has_value()){
    return;
  }
  auto visibleTargets = checkVisibleTargets(worldInfo, agent.id);
  if (visibleTargets.size() > 0){
    std::cout << "basic agent set target: agent = " << std::to_string(agent.id) << ", target = " << std::to_string(visibleTargets.at(0).id) << std::endl;
    setAgentTargetId(worldInfo, agent, visibleTargets.at(0).id);
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

  auto agentPos = gameapi -> getGameObjectPos(agent.id, true, "[gamelogic] getGoalsForBasicAgent");

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

  std::cout << "basic agent target: " << print(targetId) << std::endl;

  if (attackState -> aggravated && targetId.has_value()){
    auto targetPosition = gameapi -> getGameObjectPos(targetId.value(), true, "[gamelogic] getGoalsForBasicAgent targetPosition");
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

  if (attackState -> scared && targetId.has_value()){
    auto targetPosition = gameapi -> getGameObjectPos(targetId.value(), true, "[gamelogic] getGoalsForBasicAgent targetPosition");
    glm::vec3 moveToPosition = targetPosition;
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
        .score = [&agent, &worldInfo, targetId](std::any&) -> int {
            if (targetId.has_value()){
              auto targetPosition = gameapi -> getGameObjectPos(targetId.value(), true, "[gamelogic] getGoalsForBasicAgent aggr targetPosition");
              auto distance = glm::distance(targetPosition, gameapi -> getGameObjectPos(agent.id, true, "[gamelogic] getGoalsForBasicAgent attackGoal"));
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
    auto pointOfInterestPositions = getPointsOfInterest(worldInfo);
    auto closestPointOfInterestIndex = closestPositionIndex(agentPos, pointOfInterestPositions, attackState -> visited);
    if (closestPointOfInterestIndex.has_value()){
      auto closestPointOfInterest = pointOfInterestPositions.at(closestPointOfInterestIndex.value());

      goals.push_back(
        Goal {
          .goaltype = wanderGoal,
          .goalData = closestPointOfInterest,
          .score = [&agent, &worldInfo](std::any&) -> int {
              return 100;
          }
        }
      );  
    }     
  }

  auto ammoPositions = getAmmoPositions(worldInfo);
  if (ammoPositions.size() > 0){
    goals.push_back(
      Goal {
        .goaltype = getAmmoGoal,
        .goalData = NULL,
        .score = [&agent, &worldInfo, attackState](std::any&) -> int {
            auto currentAmmo = 100;
            if (currentAmmo <= 0){
              return 150;
            }
            return 0;
        }
      }


    );    
  }
  return goals;
}


const bool useMovementSystem = true;
const bool useAiPathing = true;
void moveToTarget(objid agentId, glm::vec3 targetPosition, bool moveVertical, float speed = 1.f){
  auto finalTargetPosition = useAiPathing ? gameapi -> navPosition(agentId, targetPosition) : targetPosition;
  if (!useMovementSystem){  // simple behavior for debug
    auto agentPos = gameapi -> getGameObjectPos(agentId, true, "[gamelogic] moveToTarget basic agent");
    auto towardTarget = gameapi -> orientationFromPos(agentPos, finalTargetPosition);
    auto newPos = gameapi -> moveRelative(agentPos, towardTarget, speed * gameapi -> timeElapsed());
    gameapi -> setGameObjectRot(agentId, towardTarget, true, Hint { .hint = "ai - moveToTarget rot" }); // tempchecked
    gameapi -> setGameObjectPosition(agentId, newPos, true, Hint { .hint = "ai - moveToTarget pos" });  // tempchecked
    return;
  }

  aiInterface.move(agentId, targetPosition, speed);
}
void attackTarget(Agent& agent){
  AgentAttackState* attackState = anycast<AgentAttackState>(agent.agentData);
  modassert(attackState, "attackState invalid");

  float currentTime = gameapi -> timeSeconds(false);
  if (currentTime - attackState -> lastAttackTime > 1.f){
    modlog("ai basic", "attack");
    attackState -> lastAttackTime = currentTime;
    aiInterface.fireGun(agent.id);
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
  }else if (goal.goaltype == wanderGoal){
    auto targetPosition = anycast<EntityPosition>(goal.goalData);
    modassert(targetPosition, "target pos was null");
    moveToTarget(agent.id, targetPosition -> position, agentData -> moveVertical, 0.7f); 
    auto agentPos = gameapi -> getGameObjectPos(agent.id, true, "[gamelogic] doGoalBasicAgent wander");


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
    std::cout << "doing move to target goal: " << print(targetPosition) << std::endl;
    modassert(targetPosition, "target pos was null");
    moveToTarget(agent.id, *targetPosition, agentData -> moveVertical);
  }else if (goal.goaltype == attackTargetGoal){
    // not yet implemented
    attackTarget(agent);
  }else if (goal.goaltype == getAmmoGoal){
    auto ammoPositions = getAmmoPositions(worldInfo);
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

void onAiBasicAgentHealthChange(Agent& agent, objid targetId, float remainingHealth){
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


AiAgent basicAgent{
  .createAgent = createBasicAgent,
  .detect = detectWorldInfoBasicAgent,
  .getGoals = getGoalsForBasicAgent,
  .doGoal = doGoalBasicAgent,
  .onHealthChange = onAiBasicAgentHealthChange,
};