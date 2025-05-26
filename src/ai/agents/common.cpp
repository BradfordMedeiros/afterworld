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
