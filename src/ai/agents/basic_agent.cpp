#include "./basic_agent.h"

extern CustomApiBindings* gameapi;

Agent createBasicAgent(objid id){
	return Agent{
    .id = id,
    .type = AGENT_BASIC_AGENT,
  };
}



std::vector<Goal> getGoalsForBasicAgent(WorldInfo& worldInfo, Agent& agent){
  static int moveToTargetGoal = getSymbol("move-to-target");

  std::vector<Goal> goals = {};

  std::set<int> symbols = { getSymbol("target") };
  auto targetAttr = getSingleAttr(agent.id, "agent-target");
  if (targetAttr.has_value()){
    symbols.insert(getSymbol(targetAttr.value()));
  }
  auto targetPositions = getVec3StateByTag(worldInfo, symbols);

  if (targetPositions.size() > 0){
    auto targetPosition = targetPositions.at(0);
    goals.push_back(
      Goal {
        .goaltype = moveToTargetGoal,
        .goalData = targetPosition,
        .score = [&agent](std::any& targetPosition) -> int { 
          auto targetPos = anycast<glm::vec3>(targetPosition);
          modassert(targetPos, "target pos was null");
          auto distance = glm::distance(*targetPos, gameapi -> getGameObjectPos(agent.id, true));
          auto score = distance > 2 ? 10 : -1; 
          return score;
        }
      }
    );
  
  }
  return goals;
}

void doGoalBasicAgent(Goal& goal, Agent& agent){
  static int moveToTargetGoal = getSymbol("move-to-target");

  if (goal.goaltype == moveToTargetGoal){
    auto targetPosition = anycast<glm::vec3>(goal.goalData);
    modassert(targetPosition, "target pos was null");
    auto agentPos = gameapi -> getGameObjectPos(agent.id, true);
    auto towardTarget = gameapi -> orientationFromPos(agentPos, glm::vec3(targetPosition -> x, agentPos.y, targetPosition -> z));
    auto newPos = gameapi -> moveRelative(agentPos, towardTarget, 1 * gameapi -> timeElapsed());
    gameapi -> setGameObjectPosition(agent.id, newPos, true);  // probably should be applying impulse instead?
    gameapi -> setGameObjectRot(agent.id, towardTarget, true);
  }
}

void detectWorldInfoBasicAgent(WorldInfo& worldInfo, Agent& agent){
  std::string stateName = std::string("agent-pos-") + std::to_string(agent.id);
  updateVec3State(worldInfo, getSymbol(stateName), gameapi -> getGameObjectPos(agent.id, true));
}