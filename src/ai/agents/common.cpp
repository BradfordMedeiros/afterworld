#include "./common.h"

extern CustomApiBindings* gameapi;

float MAX_DETECT_DISTANCE = 100.f;

std::vector<IdAndPosition> checkVisibleTargets(WorldInfo& worldInfo, objid agentId){
  auto agentPosition = gameapi -> getGameObjectPos(agentId, true, "[gamelogic] ai checkVisibleTargets");
  auto hitobjectVal = gameapi -> contactTestShape(   // probably parameterize on size / shape, visualize
    agentPosition, 
    orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f)), 
    glm::vec3(MAX_DETECT_DISTANCE, MAX_DETECT_DISTANCE, MAX_DETECT_DISTANCE)
  );

  // maybe consider doing a raycast after we have this list here

  std::set<objid> hitobjects;
  for (auto hitobject : hitobjectVal){
    hitobjects.insert(hitobject.id);
  }

  std::set<int> symbols = { getSymbol("target") };
  auto targetAttr = getSingleAttr(agentId, "agent-target");
  if (targetAttr.has_value()){
    symbols.insert(getSymbol(targetAttr.value()));
  }
  std::vector<IdAndPosition> idsAndPositions = {};
  auto targetPosVecStates = getStateByTag<EntityPosition>(worldInfo, symbols);
  for (auto &targetPosVecState : targetPosVecStates){
    if (hitobjects.count(targetPosVecState.id) > 0){
      idsAndPositions.push_back(IdAndPosition {
        .id = targetPosVecState.id,
        .position = targetPosVecState.position,
      });
    }
  }

  return idsAndPositions;
}

glm::vec3 getAgentPos(WorldInfo& worldInfo, Agent& agent){
  return gameapi -> getGameObjectPos(agent.id, true, "[gamelogic] getAgentPos");
}


std::optional<objid> getAgentTargetId(WorldInfo& worldInfo, objid agentId){
  auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agentId));
  return getState<int32_t>(worldInfo, symbol);
}
std::optional<glm::vec3> getAgentTargetPos(WorldInfo& worldInfo, objid agentId){
  auto targetId = getAgentTargetId(worldInfo, agentId);
  if (!targetId.has_value()){
    return std::nullopt;
  }
  return gameapi -> getGameObjectPos(targetId.value(), true, "[gamelogic] - getAgentTargetPos");
}
void setAgentTargetId(WorldInfo& worldInfo, objid agentId, objid targetId){
  auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agentId) /* bad basically a small leak */ ); 
  updateState(worldInfo, symbol, targetId, {}, STATE_ID, agentId);
}

std::vector<EntityPosition> getAmmoPositions(WorldInfo& worldInfo){
  static int ammoSymbol = getSymbol("ammo");
  auto ammoPositions = getStateByTag<EntityPosition>(worldInfo, { ammoSymbol });
  return ammoPositions;
}
void updateAmmoLocations(WorldInfo& worldInfo){
  static int ammoSymbol = getSymbol("ammo");
  auto targetIds = gameapi -> getObjectsByAttr("pickup-trigger", "ammo", std::nullopt);
  for (auto targetId : targetIds){
    std::string stateName = std::string("ammo-pos-") + std::to_string(targetId); // leak
    auto position = gameapi -> getGameObjectPos(targetId, true, "[gamelogic] ai updateAmmoLocations");
    updateState(worldInfo, getSymbol(stateName), EntityPosition { .id = targetId, .position = position }, { ammoSymbol }, STATE_ENTITY_POSITION, targetId);
  }
}

// based on goal-info:targets update vec3 position for each - target-pos-<objid> with team symbol from attr
void updateWorldStateTargets(WorldInfo& worldInfo){
  static int targetSymbol = getSymbol("target");
  auto targetIds = gameapi -> getObjectsByAttr("goal-info", "target", std::nullopt);
  for (auto targetId : targetIds){
    std::string stateName = std::string("target-pos-") + std::to_string(targetId);
    auto team = getSingleAttr(targetId, "team");
    std::set<int> symbols = { targetSymbol };
    if (team.has_value()){
      symbols.insert(getSymbol(team.value()));
    }
    auto position = gameapi -> getGameObjectPos(targetId, true, "[gamelogic] ai updateWorldStateTargets");
    updateState(worldInfo, getSymbol(stateName), EntityPosition { .id = targetId, .position = position }, symbols, STATE_ENTITY_POSITION, 0);
  }
  // agoal-info
}

std::vector<EntityPosition> getWorldStateTargets(WorldInfo& worldInfo){
  std::vector<EntityPosition> targets;
  static int targetSymbol = getSymbol("target");
  auto allTargets = getStateByTag<EntityPosition>(worldInfo, { targetSymbol });
  return allTargets;
}

void updatePointsOfInterest(WorldInfo& worldInfo){
  //static int ammoSymbol = getSymbol("ammo");
  auto targetIds = gameapi -> getObjectsByAttr("interest", std::nullopt, std::nullopt);
  for (auto targetId : targetIds){
    std::string stateName = std::string("interest-") + std::to_string(targetId); // leak
    auto attrValue = getSingleAttr(targetId, "interest").value();
    auto tags = getSymbol(std::string("interest-") + attrValue);
    auto position = gameapi -> getGameObjectPos(targetId, true, "[gamelogic] ai updatePointsOfInterest");
    updateState(worldInfo, getSymbol(stateName), EntityPosition { .id = targetId, .position = position }, { tags }, STATE_ENTITY_POSITION, targetId);
  }
}
std::vector<EntityPosition> getPointsOfInterest(WorldInfo& worldInfo){
  static int pointOfInterest = getSymbol("interest-generic");
  auto pointOfInterestPositions = getStateByTag<EntityPosition>(worldInfo, { pointOfInterest });
  return pointOfInterestPositions;
}
