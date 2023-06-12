#include "./basic_agent.h"

extern CustomApiBindings* gameapi;

Agent createBasicAgent(objid id){
	return Agent{
    .id = id,
    .type = AGENT_BASIC_AGENT,
  };
}

std::set<objid> checkVisibleTargets(WorldInfo& worldInfo, objid agentId){
  auto agentPosition = gameapi -> getGameObjectPos(agentId, true);
  auto hitobjectVal = gameapi -> contactTestShape(
    agentPosition, 
    orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f)), 
    glm::vec3(1.f, 1.f, 1.f)
  );

  std::set<objid> hitobjects;
  for (auto hitobject : hitobjectVal){
    hitobjects.insert(hitobject.id);
  }

  std::set<int> symbols = { getSymbol("target") };
  auto targetAttr = getSingleAttr(agentId, "agent-target");
  if (targetAttr.has_value()){
    symbols.insert(getSymbol(targetAttr.value()));
  }
  std::set<objid> targetIds = {};
  auto targetPosVecStates = getVec3StateRefByTag(worldInfo, symbols);
  for (auto targetPosVecState : targetPosVecStates){
    TargetData* targetData = anycast<TargetData>(targetPosVecState -> stateInfo.data);
    modassert(targetData, "target data was null");
    std::cout << "targetData: " << targetData -> id << std::endl;
    if (hitobjects.count(targetData -> id) > 0){
      targetIds.insert(targetData -> id);
    }
  }

  return targetIds;
}

// TODO - add contast test with provided shape 
// eg gameapi -> contactTest(glm::vec3 pos, glm::quat orientation, glm::vec3 scale, SHAPE)
void detectWorldInfoBasicAgent(WorldInfo& worldInfo, Agent& agent){
  auto visibleTargets = checkVisibleTargets(worldInfo, agent.id);


  //if (canSee && targetPositions.size() > 0){
    //updateVec3State(worldInfo, getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agentId)), targetPositions.at(0));
  //}


}

std::vector<Goal> getGoalsForBasicAgent(WorldInfo& worldInfo, Agent& agent){
  static int moveToTargetGoal = getSymbol("move-to-target");

  auto targetPosition = getVec3State(worldInfo, getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id)));
  std::vector<Goal> goals = {};
  if (targetPosition.has_value()){
    goals.push_back(
      Goal {
        .goaltype = moveToTargetGoal,
        .goalData = targetPosition.value(),
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

