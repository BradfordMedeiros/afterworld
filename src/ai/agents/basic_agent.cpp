#include "./basic_agent.h"

extern CustomApiBindings* gameapi;

Agent createBasicAgent(objid id){
	return Agent{
    .id = id,
    .type = AGENT_MOVER,
  };
}

std::vector<Goal> getGoalsForBasicAgent(WorldInfo& worldInfo, Agent& agent){
  std::vector<Goal> goals = {};
  goals.push_back(
    Goal {
      .goaltype = getSymbol("idle"),
      .goalData = NULL,
      .score = [](std::any&) -> int { 
        return 0; 
      }
     }
  );

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
        .goaltype = getSymbol("move-to-fixed-target-high-value"),
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