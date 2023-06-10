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
  auto canSeeTarget = getBoolState(worldInfo, getSymbol(std::string("agent-can-see-") + std::to_string(agent.id)));
  auto canSee = canSeeTarget.has_value() && canSeeTarget.value();

  if (targetPositions.size() > 0 && canSee){
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

// TODO - add contast test with provided shape 
// eg gameapi -> contactTest(glm::vec3 pos, glm::quat orientation, glm::vec3 scale, SHAPE)
void detectWorldInfoBasicAgent(WorldInfo& worldInfo, Agent& agent){
  auto agentPosition = gameapi -> getGameObjectPos(agent.id, true);
  std::string stateName = std::string("agent-pos-") + std::to_string(agent.id);
  updateVec3State(worldInfo, getSymbol(stateName), agentPosition);

  auto hitobjects = gameapi -> contactTestShape(
    agentPosition, 
    orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f)), 
    glm::vec3(1.f, 1.f, 1.f)
  );
  
  //std::cout << "hit objects: [";
  //for (auto &hitobject : hitobjects){
  //  std::cout << hitobject.id << "(" + gameapi -> getGameObjNameForId(hitobject.id).value() + ") ";
  //  if (aboutEqual(hitobject.position, ))
  //}
//
//  //// this is hackey, since i'm comparing to make sure the position is just close to the target.  Should add a bucket field so I can get additional info about the state i care about here to resolve the id easily
//
//
//
  //std::cout << "]" << std::endl;

  updateBoolState(worldInfo, getSymbol(std::string("agent-can-see-") + std::to_string(agent.id)), hitobjects.size() > 1);

}